#!/usr/bin/perl -wT

#
# Modification History
#
# 2004-October-27   Jason Rohrer
# Fixed some bugs.  Added proper MIME typing.
#


my $wgetPath = "/usr/bin/wget";
my $sendmailPath = "/usr/sbin/sendmail";

my $logFile = "/tmp/webByEmail.log";
my $errorLogFile = "/tmp/webByEmail_errors.log";


# then add the path to this script (e.g  /home/jcr13/webByEmail.pl) to
# your /etc/aliases file in the following way:

# name:  "| /home/jcr13/webByEmail.pl"

# where "name" is the name@myserver.com that you want people to email to
# request web pages  (a good name might be webget, which would result
# in an email address of webget@myserver.com).

# What is /etc/aliases?
# It maps various email names to user names or to command actions to run.
# In our case, we set an aliase that pipes the message to the webByEmail.pl
# script (the "|" is the "pipe").

# Next, make sure that sendmail has permission to run the webByEmail script.

# Finally, make sure that the script is an "allowed" program for sendmail
# to run.  The list of allowed programs is kept in the directory /etc/smrsh
# as a collection of softlinks.  You need to add a softlink to this
# directory for your script, like this:

# ln -s /home/jcr13/webByEmail.pl /etc/smrsh/webByEmail.pl

# This is how the "allowed" list is maintained on linux, though it may
# work differently on other platforms.


# redirect error messages to our error log
open STDERR, ">>$errorLogFile" ||
    die "Failed to redirect STDERR to our error log\n";

# redirect output messages to our log
open STDOUT, ">>$logFile" ||
    die "Failed to redirect STDOUT to our log\n";



# make taint checking happy
$ENV{ 'ENV' } = "";
$ENV{ 'PATH' } = "";


my $url = "";
my $emailAddress = "";




# read lines from stdin and deal with them
# look for URL and email address
while( <STDIN> ) {
    my $line = $_;

    if( $line =~ m/From: .*/i ) {
        ( $emailAddress ) = ( $line =~ m/From: (.*)/i );
    } 
    elsif( $line =~ m/GET http\S*/i ) {
        ( $url ) = ( $line =~ m/GET (http\S*)/i );
    }
}

#print "URL = $url\n";
#print "addr = $emailAddress\n";


my $page = `$wgetPath $url -q -O -`;


# open pipe to sendmail, and tell it to get the recipient list from
# the message headers
open( SENDMAIL_PIPE, "| $sendmailPath -t" ) ||
    die "Unable to invoke sendmail\n";

# sendmail reads up to a line with a single "."
print SENDMAIL_PIPE "To: $emailAddress\n";
print SENDMAIL_PIPE "Subject: webByEmail: $url\n";
print SENDMAIL_PIPE "MIME-Version: 1.0\n";
print SENDMAIL_PIPE "Content-Type: text/html\n\n";
print SENDMAIL_PIPE "$page\n.\n";

close SENDMAIL_PIPE;


print "Sending $url to $emailAddress\n";
