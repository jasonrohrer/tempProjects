#!/usr/bin/perl -wT

#
# Modification History
#
# 2004-December-30   Jason Rohrer
# Created.
#
# 2004-December-31   Jason Rohrer
# Improved layout of thumbnail.
#
# 2005-January-2   Jason Rohrer
# Added zero-day starting value for each month.
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

# Daily expenses in dollars
# Used to show a red expense line that "competes" with the donation line.
# Set to 0 to disable the red expense line.
my $dailyExpenses = 27;

# end of Basic settings



# Advanced settings
# Ignore these unless you know what you are doing.


# location of public data generated and consumed by this script
my $overallSumFile =    "$pubDataDirectory/overallSum.html";
my $donationCountFile = "$pubDataDirectory/donationCount.html";
my $donationDaySumDirectory = "$privDataDirectory/daySums";
my $graphImageFile =    "$pubDataDirectory/donationGraph.png";
my $graphThumbnailFile =    "$pubDataDirectory/donationGraph_thumb.png";

# where non-fatal errors and other information is logged
my $logFile =           "$privDataDirectory/grapher_log.txt";


# setup a local error log
use CGI::Carp qw( carpout );
BEGIN {
    
    # location of the error log
    my $errorLogLocation = "../cgi-data/donationTracker/grapher_errors.log";

    #use CGI::Carp qw( carpout );
    #open( LOG, ">>$errorLogLocation" ) or
    #    die( "Unable to open $errorLogLocation: $!\n" );
    #carpout( LOG );
}

# end of Advanced settings


# end of script settings


use lib '.';

use strict;
use GD::Graph::lines;
#use CGI;

#my $cgiQuery = CGI->new();

# always at least send an HTTP OK header
#print $cgiQuery->header( -type=>'text/plain', -expires=>'now',
#                         -Cache_control=>'no-cache' );


#print "test\n";




( my $sec, my $min, my $hour, my $monthDay, my $monthNumber,
  my $year, my $wday, my $yday, my $isdst ) = localtime( time() );

# convert to actual year 
$year += 1900;

# convert to actual month
$monthNumber += 1;


my @days = ();
my @donationSum = ();
my @expenseSum = ();

my $lastSum = 0;

for( my $day=0; $day <= $monthDay; $day++ ) {
    push( @days, $day );
    
    my $daySumFileName = 
        "$donationDaySumDirectory/$year/$monthNumber/$day.txt";
    if( -e $daySumFileName ) {
        my $daySum = readFileValue( $daySumFileName );
        
        $lastSum = $daySum;
    }
    push( @donationSum, $lastSum );
    if( $dailyExpenses > 0 ) {
        push( @expenseSum, $day * $dailyExpenses );
    }
}

my @monthNames = ( "January", "February", "March", "April", "May", "June",
                   "July", "August", "September", "October", "November",
                   "December" );

my $month = $monthNames[ $monthNumber - 1 ];


my @data  = ( \@days, \@donationSum );

if( $dailyExpenses > 0 ) {
    push( @data, \@expenseSum );
}



# plot main graph
my $graph = new GD::Graph::lines( 400, 300 );

$graph->set( 
    title           => "Donation Total",
    x_label         => "Days in $month",
    y_label         => "Dollars",
    x_label_skip    => 5,
    x_tick_offset   => 5,
    x_label_position=> 0.5,
    line_width      => 2,
    legend_placement=> "BL",
    bgclr           => "white",
    fgclr           => "black",
    legendclr       => "black",
    transparent     => 0 );

$graph->set_text_clr( "black" );

$graph->set_title_font( "FreeSans.ttf", 10 );
$graph->set_legend_font( "FreeSans.ttf", 8 );
$graph->set_x_label_font( "FreeSans.ttf", 9 );
$graph->set_x_axis_font( "FreeSans.ttf", 8 );
$graph->set_y_label_font( "FreeSans.ttf", 9 );
$graph->set_y_axis_font( "FreeSans.ttf", 8 );

my @legendLabels = ( "Donations" );
if( $dailyExpenses > 0 ) {
    push( @legendLabels, "Expenses (\$$dailyExpenses per day)" );
}
$graph->set_legend( @legendLabels );
$graph->set( dclrs => [ "#00AA00" , "#FF0000" ] );

my $gd_image = $graph->plot( \@data );

open( FILE, ">$graphImageFile" ) 
    or die( "Failed to open file $graphImageFile: $!\n" );

binmode FILE;
print FILE $gd_image->png;

close( FILE );


# plot thumbnail
my $thumbGraph = new GD::Graph::lines( 150, 100 );

$thumbGraph->set( 
    x_plot_values   => 0,
    y_plot_values   => 1,
    x_label         => "Day",
    y_label         => "Donations",
    bgclr           => "white",
    fgclr           => "black",
    legendclr       => "black",
    x_label_position=> 0.5,
    line_width      => 2,
    transparent     => 0 );

$thumbGraph->set_text_clr( "black" );

$thumbGraph->set_x_label_font( "FreeSans.ttf", 6 );
$thumbGraph->set_y_label_font( "FreeSans.ttf", 6 );
$thumbGraph->set_y_axis_font( "FreeSans.ttf", 6 );

$thumbGraph->set( dclrs => [ "#00AA00", "#FF0000" ] );

$gd_image = $thumbGraph->plot( \@data );

open( FILE, ">$graphThumbnailFile" ) 
    or die( "Failed to open file $graphThumbnailFile: $!\n" );

binmode FILE;
print FILE $gd_image->png;

close( FILE );


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
    open( FILE, "$fileName" ) or die;
    flock( FILE, 1 ) or die;

    # read the entire file, set the <> separator to nothing
    local $/;

    my $value = <FILE>;
    close FILE;

    return $value;
}
