#!/usr/bin/perl -wT

#
# Modification History
#
# 2004-January-9   Jason Rohrer
# Created.
# Copied from DHB P2P fund.
# Made separate directories for public/private data.
# Fixed custom field.
# Fixed refund bug.
#
# 2004-February-26   Jason Rohrer
# Changed to format dollar amounts with two decimal places.
#
# 2004-March-7   Jason Rohrer
# Changed to match new IPN refund format.
#
# 2004-May-25   Jason Rohrer
# Fixed a bug and abstracted the allowed custom field as a setting.
# Fixed bug in dealing with new IPN refunds.
#
# 2004-December-29   Jason Rohrer
# Customized for Jon at p2pnet.
#
# 2004-December-30   Jason Rohrer
# Started working on generic script for public use.
# Added support for daily tallies and graphs.
#
# 2004-December-31   Jason Rohrer
# Fixed taint checking issue.
# Added code for a monthly reset.
#
# 2005-January-2   Jason Rohrer
# Added zero-day starting value for each month.
# Fixed umask for file permissions.
# Removed an unused variable.
#


# Script settings



# Basic settings

# This script must have write permissions to BOTH of its DataDirectories.
# It must be able to create files in these directories.
# On most web servers, this means the directory must be world-writable.
# (  chmod a+w donationData  )
# These paths are relative to the location of the script.
my $pubDataDirectory =  "../htdocs/donationData";
my $privDataDirectory = "../cgi-data/donationTracker";

# If this $privDataDirectory setting is changed, you must also change it below
# where the error LOG is opened

# location of the donation button
my $donationFormURL =   "http://changeMe.net";

# set to 0 to disable graphing
# Graphs are saved into the $pubDataDirectory as PNG files.
# See the graphing script donationGrapher.pl for details.
# The graphing script requires various GD modules.
# Disable this option if you do not have them installed.
my $enableGraphing  =   1;

# should overall sum and count files be reset (back to 0) at the beginning
# of each month?  1 to enable the reset, or 0 to disable the reset.
my $monthlyReset = 1;

# end of Basic settings





# Advanced settings
# Ignore these unless you know what you are doing.


# the custom field that we pass through paypal to ensure that we are
# getting IPN from the right donate button
my $allowCustomField =  "donation_tracker";


# where the log of incoming donations is stored
my $donationLogFile =   "$privDataDirectory/donationLog.txt";


# location of public data generated by this script
my $overallSumFile =    "$pubDataDirectory/overallSum.html";
my $overallCountFile = "$pubDataDirectory/donationCount.html";
my $donationDaySumDirectory = "$privDataDirectory/daySums";

# where non-fatal errors and other information is logged
my $logFile =           "$privDataDirectory/log.txt";



# IP of notify.paypal.com
# used as cheap security to make sure IPN is only coming from PayPal
my $paypalNotifyIP =    "216.113.188.202";



# setup a local error log
use CGI::Carp qw( carpout );
BEGIN {
    
    # location of the error log
    my $errorLogLocation = "../cgi-data/donationTracker/errors.log";

    use CGI::Carp qw( carpout );
    open( LOG, ">>$errorLogLocation" ) or
        die( "Unable to open $errorLogLocation: $!\n" );
    carpout( LOG );
}

# end of Advanced settings


# end of script settings




use strict;
use CGI;                # Object-Oriented CGI library



# setup stuff, make sure our needed files are initialized
if( not doesFileExist( $overallSumFile ) ) {
    writeFile( $overallSumFile, "0" );
}
if( not doesFileExist( $overallCountFile ) ) {
    writeFile( $overallCountFile, "0" );
}


# allow group to write to our data files
umask( oct( "02" ) );


# track daily sums (used by graph script)

# grab the local time to find our year, month, and day
( my $sec, my $min, my $hour, my $monthDay, my $monthNumber,
  my $year, my $wday, my $yday, my $isdst ) = localtime( time() );

# convert to actual year (localtime returns 2004 as 104)
$year += 1900;

# convert to actual month (localtime returns month in range 0-11)
$monthNumber += 1;

my $yearDirectory = "$donationDaySumDirectory/$year";
my $monthDirectory = "$yearDirectory/$monthNumber";

if( not -e $yearDirectory ) {
    makeDirectory( $yearDirectory, oct( "0777" ) );
}

if( not -e $monthDirectory ) {
    makeDirectory( $monthDirectory, oct( "0777" ) );
}


# running sum of total donation level reached on each day 
my $daySumFileName = "$monthDirectory/$monthDay.txt";

# include day zero (starting value) so that we always have at least 
# 2 data points
my $zeroDaySumFileName = "$monthDirectory/0.txt";


# running sum of total donation level reached on each month
# reset to 0 at the beginning of a month
my $monthSumFileName = "$monthDirectory/sum.txt"; 

# running count of donations for each month
# reset to 0 at the beginning of a month
my $monthCountFileName = "$monthDirectory/count.txt"; 


if( not -e $monthSumFileName ) {
    # first time script called during this month
    # create both whole-month sum file and zero-day starting sum file

    if( $monthlyReset ) {
        # starts at 0
        writeFile( $monthSumFileName, "0" );
        writeFile( $zeroDaySumFileName, "0" );
    }
    else {
        # starts at current overall sum
        my $sum = readTrimmedFileValue( $overallSumFile );
        writeFile( $monthSumFileName, "$sum" );
        writeFile( $zeroDaySumFileName, "$sum" );
        
    }
}

if( not -e $monthCountFileName ) {
    # first time script called during this month
    
    if( $monthlyReset ) {
        # starts at 0
        writeFile( $monthCountFileName, "0" );
    }
    else {
        # starts at current overall count
        my $count = readTrimmedFileValue( $overallCountFile );
        writeFile( $monthCountFileName, "$count" );
    }
}





# create object to extract the CGI query elements

my $cgiQuery = CGI->new();




# always at least send an HTTP OK header
print $cgiQuery->header( -type=>'text/html', -expires=>'now',
                         -Cache_control=>'no-cache' );

my $remoteAddress = $cgiQuery->remote_host();



my $action = $cgiQuery->param( "action" ) || '';

if( $remoteAddress eq $paypalNotifyIP ) {
    
    my $donorName;


    # was this paypal payment generated by us?
    my $customField = $cgiQuery->param( "custom" ) || '';
    if( $customField eq $allowCustomField ) {
        
        my $amount = $cgiQuery->param( "mc_gross" ) || '';
        
        my $currency = $cgiQuery->param( "mc_currency" ) || '';
        
        my $fee = $cgiQuery->param( "mc_fee" ) || '0';
        
        my $date = $cgiQuery->param( "payment_date" ) || '';
        
        my $transactionID = $cgiQuery->param( "txn_id" ) || '';


        # these are for our private log only, for tech support, etc.
        # this information should not be stored in a web-accessible
        # directory
        my $payerFirstName = $cgiQuery->param( "first_name" ) || '';
        my $payerLastName = $cgiQuery->param( "last_name" ) || '';
        my $payerEmail = $cgiQuery->param( "payer_email" ) || '';
            

        # only track US Dollars 
        # (can't add apples to oranges to get a final sum)
        if( $currency eq "USD" ) {

                my $status = $cgiQuery->param( "payment_status" ) || '';
                
                my $completed = $status eq "Completed";
                my $pending = $status eq "Pending";
                my $refunded = $status eq "Refunded";

                if( $completed or $pending or $refunded ) {
                    
                    # write all relevant payment info into our private log
                    addToFile( $donationLogFile,
                               "$transactionID  $date\n" . 
                               "From: $payerFirstName $payerLastName " .
                               "($payerEmail)\n" .
                               "Amount: \$$amount\n" .
                               "Fee: \$$fee\n" .
                               "Status: $status\n\n" );                    
                
                        
                    if( $refunded ) {
                        # subtract from total sum
                        
                        my $oldSum = 
                            readTrimmedFileValue( $monthSumFileName );

                        # both the refund amount and the
                        # fee on the refund are now reported as negative
                        # this changed as of February 13, 2004
                        my $newSum = $oldSum + $amount - $fee;
                        
                        # format to show 2 decimal places
                        my $newSumString = sprintf( "%.2f", $newSum );

                        writeFile( $monthSumFileName, $newSumString );
                        
                        
                        my $oldCount = readTrimmedFileValue( 
                                                       $monthCountFileName );
                        my $newCount = $oldCount - 1;
                        writeFile( $monthCountFileName, $newCount );
                              
                    }
                    
                    # This check no longer needed as of February 13, 2004
                    # since now only one IPN is sent for a refund.
                    #  
                    # ignore negative completed transactions, since
                    # they are reported for each refund (in addition to 
                    # the payment with Status: Refunded)
                    if( $completed and $amount > 0 ) {
                        # fee has not been subtracted yet
                        # (fee is not reported for Pending transactions)
                        
                        my $oldSum = 
                            readTrimmedFileValue( $monthSumFileName );
                        my $newSum = $oldSum + $amount - $fee;
                        
                        # format to show 2 decimal places
                        my $newSumString = sprintf( "%.2f", $newSum );
                        
                        writeFile( $monthSumFileName, $newSumString );
                        
                        my $oldCount = readTrimmedFileValue( 
                                                   $monthCountFileName );
                        my $newCount = $oldCount + 1;
                        writeFile( $monthCountFileName, $newCount );
                    }
                }
                else {
                    addToFile( $logFile, "Payment status unexpected\n" );
                    addToFile( $logFile, "status = $status\n" );
                }
            }
        else {
            addToFile( $logFile, "Currency not USD\n" );
            addToFile( $logFile, "currency = $currency\n" );
        }
    }
    else {
        addToFile( $logFile, "Custom field does not contain our tag\n" );
        addToFile( $logFile, "custom = $customField\n" );
    }
}
else {
    # else not from paypal, so it might be a user accessing the script
    # URL directly for some reason

    # make sure we're not getting IPN from another address
    my $customField = $cgiQuery->param( "custom" ) || '';
    if( $customField eq $allowCustomField ) {

        my $date = $cgiQuery->param( "payment_date" ) || '';
        my $transactionID = $cgiQuery->param( "txn_id" ) || '';
        my $amount = $cgiQuery->param( "mc_gross" ) || '';
        
        my $payerFirstName = $cgiQuery->param( "first_name" ) || '';
        my $payerLastName = $cgiQuery->param( "last_name" ) || '';
        my $payerEmail = $cgiQuery->param( "payer_email" ) || '';


        my $fee = $cgiQuery->param( "mc_fee" ) || '0';
        my $status = $cgiQuery->param( "payment_status" ) || '';

        # log it
        addToFile( $donationLogFile,
                   "WARNING:  got IPN from unexpected IP address\n" .
                   "IP address:  $remoteAddress\n" .
                   "$transactionID  $date\n" . 
                   "From: $payerFirstName $payerLastName " .
                   "($payerEmail)\n" .
                   "Amount: \$$amount\n" .
                   "Fee: \$$fee\n" .
                   "Status: $status\n\n" );
    }

    # Redirect to the donation form page
    print 
        "<META HTTP-EQUIV=\"Refresh\" CONTENT=\"0; URL=$donationFormURL\">\n";
}






# sum so far for the month
my $monthSum = readTrimmedFileValue( $monthSumFileName );

# save it as our overall sum to be grabbed by the server-side include
writeFile( $overallSumFile, "$monthSum" );

# count so far for the month
my $monthCount = readTrimmedFileValue( $monthCountFileName );

# save it as our overall count to be grabbed by the server-side include
writeFile( $overallCountFile, "$monthCount" );


# update today's tally
writeFile( $daySumFileName, $monthSum );



if( $enableGraphing ) {
    # make taint checking happy
    $ENV{ 'PATH' } = "";

    `./donationGrapher.pl`;
}
 


##
# Reads file as a string.
#
# @param0 the name of the file.
#
# @return the file contents as a string.
#
# Example:
# my $value = readFileValue( "myFile.txt" );
##
sub readFileValue {
    my $fileName = $_[0];
    open( FILE, "$fileName" ) 
        or die( "Failed to open file $fileName: $!\n" );
    flock( FILE, 1 ) 
        or die( "Failed to lock file $fileName: $!\n" );

    my @lineList = <FILE>;

    my $value = join( "", @lineList );

    close FILE;
 
    return $value;
}



##
# Reads file as a string, trimming leading and trailing whitespace off.
#
# @param0 the name of the file.
#
# @return the trimmed file contents as a string.
#
# Example:
# my $value = readFileValue( "myFile.txt" );
##
sub readTrimmedFileValue {
    my $returnString = readFileValue( $_[0] );
    trimWhitespace( $returnString );

    return $returnString;
}



##
# Writes a string to a file.
#
# @param0 the name of the file.
# @param1 the string to print.
#
# Example:
# writeFile( "myFile.txt", "the new contents of this file" );
##
sub writeFile {
    my $fileName = $_[0];
    my $stringToPrint = $_[1];
    
    open( FILE, ">$fileName" ) 
        or die( "Failed to open file $fileName: $!\n" );
    flock( FILE, 2 ) 
        or die( "Failed to lock file $fileName: $!\n" );

    print FILE $stringToPrint;
        
    close FILE;
}



##
# Checks if a file exists in the filesystem.
#
# @param0 the name of the file.
#
# @return 1 if it exists, and 0 otherwise.
#
# Example:
# $exists = doesFileExist( "myFile.txt" );
##
sub doesFileExist {
    my $fileName = $_[0];
    if( -e $fileName ) {
        return 1;
    }
    else {
        return 0;
    }
}



##
# Trims any whitespace from the beginning and end of a string.
#
# @param0 the string to trim.
##
sub trimWhitespace {   
    
    # trim from front of string
    $_[0] =~ s/^\s+//;

    # trim from end of string
    $_[0] =~ s/\s+$//;
}



##
# Appends a string to a file.
#
# @param0 the name of the file.
# @param1 the string to append.
#
# Example:
# addToFile( "myFile.txt", "the new contents of this file" );
##
sub addToFile {
    my $fileName = $_[0];
    my $stringToPrint = $_[1];
        
    open( FILE, ">>$fileName" ) 
        or die( "Failed to open file $fileName: $!\n" );
    flock( FILE, 2 ) 
        or die( "Failed to lock file $fileName: $!\n" );
        
    print FILE $stringToPrint;
        
    close FILE;
}



##
# Makes a directory file.
#
# @param0 the name of the directory.
# @param1 the octal permission mask.
#
# Example:
# makeDirectory( "myDir", oct( "0777" ) );
##
sub makeDirectory {
    my $fileName = $_[0];
    my $permissionMask = $_[1];
    
    mkdir( $fileName, $permissionMask );
}

