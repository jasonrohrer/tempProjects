#!/usr/bin/perl -wT

#
# Modification History
#
# 2005-January-2   Jason Rohrer
# Created.
#
# 2005-January-9   Jason Rohrer
# Changed to re-gen graph images if they do not exist.
#
# 2005-May-12   Jason Rohrer
# Changed to serve images directly instead of redirecting.
#



# Script settings


# Basic settings


# These paths are relative to the location of the script.

# Where the graph image files are stored
my $pubDataDirectory =  "../htdocs/donationData";
my $pubDataURL =  "http://changeMe.net/donationData";


# Where this script will keep its private data (such as error logs).
# It must be able to create files in this directory.
# On most web servers, this means the directory must be world-writable.
# (  chmod a+w donationData  )
my $privDataDirectory = "../cgi-data/donationTracker";

# If this $privDataDirectory setting is changed, you must also change it below
# where the error LOG is opened


# end of Basic settings





# Advanced settings
# Ignore these unless you know what you are doing.


# location of graph image files
my $graphImageFile =     "$pubDataDirectory/donationGraph.png";
my $graphThumbnailFile = "$pubDataDirectory/donationGraph_thumb.png";

my $graphImageFileURL =     "$pubDataURL/donationGraph.png";
my $graphThumbnailFileURL = "$pubDataURL/donationGraph_thumb.png";



# setup a local error log
use CGI::Carp qw( carpout );
BEGIN {
    
    # location of the error log
    my $errorLogLocation = 
        "../cgi-data/donationTracker/graphGrabber_errors.log";

    use CGI::Carp qw( carpout );
    open( LOG, ">>$errorLogLocation" ) or
        die( "Unable to open $errorLogLocation: $!\n" );
    carpout( LOG );
}

# end of Advanced settings


# end of script settings




use strict;
use CGI;                # Object-Oriented CGI library


# allow group to write to our data files
umask( oct( "02" ) );



# create object to extract the CGI query elements

my $cgiQuery = CGI->new();


my $imageToGet = $cgiQuery->param( "image" ) || '';


# either "thumb" or "full"

my $imageFile;
my $imageURL;

if( $imageToGet eq "thumb" ) {
    $imageFile = $graphThumbnailFile;
    $imageURL = $graphThumbnailFileURL;
}
else {
    $imageFile = $graphImageFile;
    $imageURL = $graphImageFileURL;
}


if( not -e $imageFile ) {
    # graph does not exist


    # call our grapher script to updat the graph images

    # make taint checking happy
    $ENV{ 'PATH' } = "";

	`./donationGrapher.pl`;
}
else {


    # grab the modification time of the image file
    ( my $dev, my $ino, my $mode, my $nlink, my $uid, 
      my $gid, my $rdev, my $size, my $atime, my $modTime, 
      my $ctime, my $blksize, my $blocks) = stat( $imageFile );
 
    # convert the mod time to a year / yearDayNumber form
    ( my $sec, my $min, my $hour, my $monthDay, my $monthNumber,
      my $modYear, my $wday, my $modYearDay, my $isdst ) = 
          localtime( $modTime );

    # grab our current year and year day number
    ( $sec, $min, $hour, $monthDay, $monthNumber,
      my $currentYear, $wday, my $currentYearDay, $isdst ) = 
          localtime( time() );


    if( ( $currentYear > $modYear ) or
        ( $currentYearDay > $modYearDay ) ) {

        # graph was not updated today


        # call our grapher script to updat the graph images
        
        # make taint checking happy
        $ENV{ 'PATH' } = "";

        `./donationGrapher.pl`;
    }
}



# Redirect to the chosen image URL
# print $cgiQuery->redirect( "$imageURL" );

# Redirection is a problem if the image is not located in the servable
# part of the web server's file system.  Instead, serve the image directly

print $cgiQuery->header( -type=>'image/png' );

open( IMAGE_FILE, "$imageFile" );

my @lineList = <IMAGE_FILE>;
print @lineList;

close( IMAGE_FILE );

