#!/usr/bin/perl -wT

#
# Modification History
#
# 2004-October-22  Jason Rohrer
# Created.
#
# 2004-October-24  Jason Rohrer
# Added support for ping commands.
# Added support for alternate proxies.
# Fixed relative URL behavior.
# Added auto-hopping.
#



# these user options can be set below
my $scriptURL;
my $dataDirectory;
my $errorLogPath;


BEGIN { 

    # BEGIN USER OPTIONS


    # the URL for this script
    # leave blank to get the script's URL from the CGI interface
    # (should leave blank for most setups)
    $scriptURL = "";


    # path to the data directory
    # script must be able to create files in this directory
    $dataDirectory = "../../cgi-data/freeWebData";


    # set to include necessary paths for finding gzip
    # we cannot use the default PATH because taint checking forbids it
    #$ENV{ 'PATH' } = "/bin:/usr/bin:/usr/local/bin";

    
    # path to the error log
    # script must be able to create/write to this file
    $errorLogPath = "/tmp/freeWeb_errors.log";


    # END USER OPTIONS

    
    use CGI::Carp qw( carpout );
    open( LOG, ">>$errorLogPath" ) or
        die( "Unable to open $errorLogPath: $!\n" );
    carpout( LOG );
}


# make taint checking happy
$ENV{ 'ENV' } = "";




use lib '.';

##use Crypt::CBC;
#use Crypt::Rijndael;

#use MD5;

use CGI;
use CGI qw/escape unescape/;





my $cgiQuery = CGI->new();

if( $scriptURL eq "" ) {    
    # not overrided by the user option
    
    # default to getting the URL from the query
    $scriptURL = $cgiQuery->url();
}

# we pack all of our parameters and the proxied URL after our script
# name in the path
$extraPath = $cgiQuery->url(-path_info=>1,-query=>1);

my $barStatus = "showBar";

if( $extraPath =~ m/hideBar/ ) {
    $barStatus = "hideBar";
}


my $autoHopStatus = "autoHopOff";

if( $extraPath =~ m/autoHopOn/ ) {
    $autoHopStatus = "autoHopOn";
}


# our URL comes after ~~~
( my $packedParams, my $packedURL ) = split( "~~~", $extraPath );



my $Accept  = $cgiQuery->http( 'Accept' ) || "*/*";
my $UserAgent  = $cgiQuery->user_agent() || "";


my $urlToFetch = $cgiQuery->param( "url" ) || '';
my $debug = $cgiQuery->param( "debug" ) || '';
my $autoHopParam = $cgiQuery->param( "autoHop" ) || '';
my $ping = $cgiQuery->param( "ping" ) || '';
my $addProxy = $cgiQuery->param( "addProxy" ) || '';

# empty list of good proxies
# filled later by subroutines if we need it
my @goodProxies = ();



if( $extraPath =~ m/debug/ ) {
    $debug = 1;
}

if( $autoHopParam == 1 ) {
    $autoHopStatus = "autoHopOn";
}


if( $urlToFetch ne "" ) {
    # create a packed URL from the unpacked
    $packedURL = $urlToFetch;
    # use s&&& instead of s/// so we don't have to escape "/"
    $packedURL =~ s&http://&http/&;
}
else {
    # unpack the URL
    $urlToFetch = $packedURL;
    # use s&&& instead of s/// so we don't have to escape "/"
    $urlToFetch =~ s&http/&http://&;
}


if( not -e "$dataDirectory/otherProxies.txt" ) {
    writeFile( "$dataDirectory/otherProxies.txt", "" );
}


if( $ping ne "" ) {
    print $cgiQuery->header( -type=>"text/plain" );
    print "ALIVE";
}
elsif( $addProxy ne "" ) {
    print $cgiQuery->header( -type=>"text/plain" );
    
    # make sure the proxy is alive
    my $response = fetchURLSimple( "$addProxy?ping=1" );
    
    if( $response eq "ALIVE" ) {
        
        
        my $proxyFile = "$dataDirectory/otherProxies.txt";
        my @existingProxies = split( /\s+/, readFileValue( $proxyFile ) );

        my $exists = 0;

        foreach my $proxy ( @existingProxies ) {
            if( $debug == 1 ) {
                print "Comparing \"$addProxy\" to \"$proxy\"\n";
            }
            if( $proxy eq $addProxy ) {
                $exists = 1;
            }
        }

        if( $exists != 1 ) {
            addToFile( $proxyFile,
                       "$addProxy\n" );

            # add ourself to this proxy
            fetchURLSimple( "$addProxy?addProxy=$scriptURL" );
            
            print "OK";
        }
        else {
            print "EXISTS";
        }
    }
    else {
        print "FAILED";
    } 
}
elsif( $urlToFetch ne "" ) {

    # untaint the url

    # allowed URL characters include alpha-numeric plus marks, plus some
    # reserved characters
    # mark        = "-" | "_" | "." | "!" | "~" | "*" | "'" | "(" | ")"
    # reserved    = ";" | "/" | "?" | ":" | "@" | "&" | "=" | "+" | "$" | ","
    # reserved we allow = "/" | "?" | ":" | "@" | "&" | "=" | "+" | "$" | ","
    ( $urlToFetch ) = 
        ( $urlToFetch =~ 
          m/(http:\/\/[A-Za-z0-9\-_\.!~\*\'\(\)\/\?:@&=\+\$,]+)/i );

    proxyPage( $urlToFetch );
}
else {
    # just show our toolbar
    # print the HTTP header
    print $cgiQuery->header( -type=>"text/html" );

    printToolbar( "http://" );
}



##
# Proxifies the HTML fetched from a URL, creating HTML in which all URLs
# point back at the proxy, and prints the result.
#
# @param0 the URL to proxy.
##
sub proxyPage {

    my $url = $_[0];


    ( my $response, my $mimeType ) = 
        fetchURL( $url, $Accept, $UserAgent );


    # print the HTTP header
    print $cgiQuery->header( -type=>"$mimeType" );



    # only search and replace in text files
    if( $mimeType =~ m/text/ ) {
        
        my $proxyFile = "$dataDirectory/otherProxies.txt";
        my @existingProxies = split( /\s+/, readFileValue( $proxyFile ) );
        
        # ping each one
        my $numProxies = scalar( @existingProxies ); 
        for( my $i=0; $i<$numProxies; $i++ ) {
            
            my $proxy = $existingProxies[$i];
            if( $debug == 1 ) {
                print "Pinging proxy $proxy\n";
            }
            # make sure the proxy is alive
            my $response = fetchURLSimple( "$proxy?ping=1" );
    
            if( $response eq "ALIVE" ) {
                push( @goodProxies, $proxy );
            }
        }
        my $goodList = join( "\n", @goodProxies );
        writeFile( $proxyFile, "$goodList\n" );
        

        
        my @urlPrefixes = (); 

        # add our proxy to the list of possible URL prefixes
        push( @urlPrefixes, "$scriptURL/$barStatus/$autoHopStatus~~~" );

        if( $autoHopStatus eq "autoHopOn" ) {
            # if auto-hop is on, add other proxies too
            $numProxies = scalar( @goodProxies );
            for( my $i=0; $i<$numProxies; $i++ ) {
                my $proxy = $goodProxies[$i];
                push( @urlPrefixes, "$proxy/$barStatus/$autoHopStatus~~~" );
            }
        }

        # extract the root URLs

        # short root is for relative URLs that start with "/"
        # this is the http://server_name part of the URL
        # use m&& instead of m// so we don't have to escape "/"
        ( my $shortRootURL ) = ( $urlToFetch =~ m&(http://[^/]+)&i ); 

        # long root is for relative URLs that do not start with "/"
        # this is the http://server_name/dir/subdir/subsubdir part of the URL
        ( my $longRootURL ) = ( $urlToFetch =~ m&(http://.+)/&i ); 
        
        if( $debug == 1 ) {
            print "Short root = $shortRootURL   long = $longRootURL\n";
        }

        if( $longRootURL eq "" ) {
            # no final "/" in the URL
            # this means no subdirs, so the short and long root are the same
            $longRootURL = $shortRootURL;
        }
        
        


        $response = proxyURLs( $response, "SRC", $shortRootURL, 
                               $longRootURL, @urlPrefixes );
        $response = proxyURLs( $response, "ACTION", $shortRootURL, 
                               $longRootURL, @urlPrefixes );
        $response = proxyURLs( $response, "HREF", $shortRootURL, 
                               $longRootURL, @urlPrefixes );
    }


    if( $debug == 1 ) {
        print "\n\n\n[PROCESSED_RESPONSE]=";
        print $response;
    }

    if( $mimeType =~ m/text\/html/ ) {
        # print our header table
    
        if( $barStatus ne "hideBar" ) {
            printToolbar( $urlToFetch );
        }   
        else {
            print
              "<TABLE BORDER=1><TR>".
              "<TD BGCOLOR=#FFFFFF>".
              "<A HREF=\"$scriptURL/showBar/$autoHopStatus~~~$packedURL\">" .
              "show</A></TD>".
              "</TR></TABLE>".
              "</FORM>";
        }
    }

    print $response;
}



##
# Prints the toolbar HTML.
#
# @param0 the current URL.
##
sub printToolbar {
    my $currentURL = $_[0];
    
    # FIXME  need to force font colors so that page fonts do not override ours
    print
        "<FORM ACTION=\"$scriptURL/showBar/\">".
        "<TABLE WIDTH=100% BORDER=1><TR>".
        "<TD COLSPAN=3 BGCOLOR=#FFFFFF>".
        "<FONT COLOR=#000000>Current proxy: $scriptURL</FONT></TD></TR>" .
        "<TR>" .
        "<TD BGCOLOR=#FFFFFF ALIGN=LEFT>".
        "<input type=text name=url size=45 maxlength=2048 value=\"$currentURL\"> ".
        "<input type=submit name=btnG VALUE=\"Go\"></TD>".
        "<TD BGCOLOR=#FFFFFF ALIGN=RIGHT>";
    if( $autoHopStatus eq "autoHopOn" ) {
        print 
            "[<A HREF=\"$scriptURL/$barStatus/autoHopOff~~~$packedURL\">" .
            "disable auto-hop</A>]";
    }
    else {
        print 
            "[<A HREF=\"$scriptURL/$barStatus/autoHopOn~~~$packedURL\">" .
            "enable auto-hop</A>]";
    }
    print 
        " [<A HREF=\"$scriptURL/hideBar/$autoHopStatus~~~$packedURL\">" .
        "hide</A>]</TD>".
        "</TR>\n";

    if( $currentURL ne "http://" ) {
        # show 5 alternate proxies
        my $numProxies = scalar( @goodProxies ); 
        if( $numProxies > 0 ) {
            my $numToShow = 5;
            if( $numToShow > $numProxies) {
                $numToShow = $numProxies;
            }
            print "<TR><TD BGCOLOR=#FFFFFF COLSPAN=3 ALIGN=CENTER>\n";
            print "Alternates: ";
            for( my $i=0; $i<$numToShow; $i++ ) {
                my $proxyChoice = int( rand( $numProxies ) );
                my $proxy = $goodProxies[ $proxyChoice ];
                
                my $shortProxyURL = $proxy;

                # remove http:// from start of short URL
                # use s&&& instead of s/// so we don't have to escape "/"
                $shortProxyURL =~ s&http://&&;
                my $maxLength = 30; 
                if( length( $shortProxyURL ) > 20 ) {
                    # truncate, leaving room for "..."
                    $shortProxyURL = substr( $shortProxyURL, 0, 
                                             $maxLength - 3 );
                    $shortProxyURL = "$shortProxyURL...";
                }

                print "[<A HREF=\"$proxy/$barStatus/$autoHopStatus" .
                      "~~~$currentURL\">".
                      "$shortProxyURL</A>] ";
            }
            print "\n</TD></TR>\n";
        }
    }

    print "</TABLE>".
        "</FORM></FONT>";
}




##
# Fetch the contents of a URL with simple parameters and return value.
#
# @param0 the URL to fetch.
#
# @return the fetched data response.
##
sub fetchURLSimple {
    my $url = $_[0];
    my $Accept = "*/*";
    my $UserAgent = "";

    ( my $response, my $type ) = fetchURL( $url, $Accept, $UserAgent );

    return $response;
}



use IO::Socket::INET;


##
# Fetch the contents of a URL.
#
# Based on code written by nand (nandhp@myrealbox.com) for itms4all.
#
# @param0 the URL to fetch.
# @param1 the HTTP Accept header value.
# @param2 the HTTP User-Agent header value.
#
# @return list containing ( response, mimeType ).
##
sub fetchURL {
    my $url = $_[0];
    my $Accept = $_[1];
    my $UserAgent = $_[2];
    
    # strip off http://
    $url =~ s/http:\/\///;


    # server and port are everything before the first slash
    ( my $serverAndPort ) = ( $url =~ m/([^\/]+)/i ); 

    # server is everything up to an optional :port
    ( my $server ) = ( $serverAndPort =~ m/([^:]+)/i );

    # get the server-relative URL
    $url =~ s/$serverAndPort//;
    
    if( length( $url ) == 0 ) {
        # at least a /
        $url = "/";
    }

    # make sure it has a port number... default to 80
    if( not $serverAndPort =~ m/:[0-9]+/ ) {
        $serverAndPort = $serverAndPort . ":80";
    }

    my $sock = IO::Socket::INET->new( $serverAndPort ) 
               or return "";

    binmode( $sock );
    
    my $request =
      "GET $url HTTP/1.0\r\n".
      "Host: $server\r\n".
      "User-Agent: $UserAgent\r\n".
      "Accept: $Accept\r\n".
      "Connection: close\r\n\r\n";
    
    print $sock $request;

    if( $debug == 1 ) {
        print $cgiQuery->header( -type=>"text/plain" );
        print $request;
    }

    my $response = "";
    my $buffer;
    
    # read buffers of response until connection is closed
    while( read( $sock, $buffer, 2560 ) ) {
        $response = $response . $buffer;
    }

    if( $debug == 1 ) {
        print "\n\n\n[RAW_RESPONSE]=";
        print $response;
    }

    ( my $type ) = ( $response =~ m/Content-Type:\s*(.+)\s*\r\n/ ); 

    # strip off the headers
    # headers should end with \r\n\r\n
    my @parts = split( "\r\n\r\n", $response );
    
    # remove the first part, which is the header block
    shift( @parts );

    $response = join( "\r\n\r\n", @parts );


    if( $debug == 1 ) {
        print "\n\n\n[EXTRACTED_type]=";
        print $type;
        print "\n\n\n[EXTRACTED_RESPONSE]=";
        print $response;
        print "\n";
    }

    return ( $response, $type );
}



##
# Replaces a URL with a proxied URL.
#
# @param0 the HTML text of the page.
# @param1 the URL tag to search for (example:  "SRC" or "HREF").
# @param2 the short root URL to use when fixing relative URLs 
#    (example: "http://www.google.com" ).
# @param3 the long root URL to use when fixing relative URLs 
#    (example: "http://www.google.com/dir/subdir/subsubdir" ).
# @param4 a list of possible proxy URL prefixes 
#    (example: "http://test.com/cgi-bin/myScript?url=" ).
#
# @return the HTML text with replacements.
##
sub proxyURLs {
    (my $html,
     my $tag,
     my $shortRootURL,
     my $longRootURL,
     my @proxyURLPrefixes ) = @_;
    
    
    # fix relative URLs to make them absolute
    
    # first, add quotes, if missing, to all URL tags
    # search and replace all unquoted URLs
    # (this step makes finding relative URLs easier in the next step)
    $html =~ 
        s/$tag\s*=\s*([^\"\s>]*)([\s>])/$tag=\"$1\"$2/gi;


    # relative URLS do not contain  : or  "
    # by ignoring urls that contain :, we ignore those that contain "http://"

    # if they start with a /, we should use our short root
    $html =~ 
        s/$tag\s*=\s*\"(\/[^:\"]*)\"/$tag=\"$shortRootURL$1\"/gi;

    # if they do not start with a /, we should use our long root
    $html =~ 
        s/$tag\s*=\s*\"([^\/:\"][^:\"]*)\"/$tag=\"$longRootURL\/$1\"/gi;

    # finally, pack all URLs
    # use s&&& instead of s/// so we don't have to escape "/"
    $html =~ 
        s&$tag\s*=\s*\"\http://([^\"]*)\"&$tag=\"http/$1\"&gi;

    # replace all TAG = URLs with proxy URLs
    # for each match, pick a random prefix
    my $prefixIndex = 0;
    my $numPrefixes = scalar( @proxyURLPrefixes );
    my $currentPrefix;
    # URLs do not contain  > "   or whitespace
    $html =~ 
        s/$tag\s*=\s*\"*([^\">\s]*)\"*/
            $currentPrefix = $proxyURLPrefixes[ $prefixIndex ];
            $prefixIndex++;
            if( $prefixIndex >= $numPrefixes ) {
                $prefixIndex = 0;
            };
            "$tag=\"$currentPrefix$1\""/egi;

    return $html;
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
    open( FILE, "$fileName" ) or die;
    flock( FILE, 1 ) or die;

    # read the entire file, set the <> separator to nothing
    local $/;

    my $value = <FILE>;
    close FILE;

    return $value;
}



##
# Writes a string to a file in the filesystem.
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
    
    open( FILE, ">$fileName" ) or die;
    flock( FILE, 2 ) or die;

    print FILE $stringToPrint;
        
    close FILE;
}



##
# Appends a string to a file in the filesystem.
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
        
    open( FILE, ">>$fileName" ) or die;
    flock( FILE, 2 ) or die;
        
    print FILE $stringToPrint;
        
    close FILE;
}
