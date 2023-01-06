#!/usr/bin/perl -wT

#
# Modification History
#
# 2004-April-15  Jason Rohrer
# Created.
#
# 2004-April-16  Jason Rohrer
# Added a Copyright column to the search results.
# Fixed copyright column header.
# Changed to get the script URL from the CGI interface by default.
# Added a thank-you for the host.
# Added ideas link and download link.
# Added link to VLC player.
#
# 2004-April-29  Jason Rohrer
# Fixed HTTP headers to work with Apple's more strict version 4.5 server.
# Thanks to nand (nandhp@myrealbox.com) for this fix.
#
# 2007-Frebruary-4  Jason Rohrer
# Turned decryption off, because latest version of store data is not encrypted.
#



# these user options can be set below
my $scriptURL;
my $tempFileName;
my $errorLogPath;


BEGIN { 

    # BEGIN USER OPTIONS


    # the URL for this script
    # leave blank to get the script's URL from the CGI interface
    # (should leave blank for most setups)
    $scriptURL = "";

    # Used to thank the people hosting this script
    # set to "" to skip printing the thank-you message
    $hostedByName = "Downhill Battle";
    $hostedByURL = "http://www.downhillbattle.org";
    $ideasName = "what this script could be used for";
    $ideasURL = "http://www.downhillbattle.org/itmsscript/index.html";
    $downloadURL = "http://hcsoftware.sourceforge.net/jason-rohrer/itms4all/";

    # script must have permissions to create this file
    $tempFileName = "/tmp/tempDownload";

    # set to include necessary paths for finding gzip
    # we cannot use the default PATH because taint checking forbids it
    $ENV{ 'PATH' } = "/bin:/usr/bin:/usr/local/bin";
    
    # path to the error log
    # script must be able to create/write to this file
    $errorLogPath = "/tmp/itms4all_errors.log";


    # END USER OPTIONS

    
    use CGI::Carp qw( carpout );
    open( LOG, ">>$errorLogPath" ) or
        die( "Unable to open $errorLogPath: $!\n" );
    carpout( LOG );
}


# make taint checking happy
$ENV{ 'ENV' } = "";




use lib '.';

use Crypt::CBC;
use Crypt::Rijndael;

use MD5;

use CGI;
use CGI qw/escape unescape/;





my $cgiQuery = CGI->new();

if( $scriptURL eq "" ) {    
    # not overrided by the user option
    
    # default to getting the URL from the query
    $scriptURL = $cgiQuery->url();
}



my $searchString = $cgiQuery->param( "searchString" ) || '';

# untaint the search string (only alphanumeric, quotes, and spaces)
( $searchString ) = ( $searchString =~ m/([A-Za-z0-9\" ]*)/ );



# whether we have a search query or not

# print the HTTP header
print $cgiQuery->header( -type=>'text/html' );

# print the start of our HTML doc
print "<HTML><HEAD><TITLE>iTMS-4-ALL</TITLE></HEAD>\n";
print "<BODY BGCOLOR=#FFFFFF TEXT=#000000 LINK=#0000FF VLINK=#FF0000>\n";


# print our search form
print "<TABLE BORDER=0 CELLPADDING=0><TR>\n";
print "<TD VALIGN=MIDDLE><H1>iTMS-4-ALL</H1></TD>\n";
print "<TD VALIGN=MIDDLE>".
      "(you just gotta hate proprietary internet services that ".
      "<I>try</I> to block non-approved platforms)";

if( $hostedByName ne "" ) {
    print "\n<BR>Hosted by <A HREF=\"$hostedByURL\">$hostedByName</A><BR>\n";
    print "[<A HREF=\"$ideasURL\">$ideasName</A>] ".
          "[<A HREF=\"$downloadURL\">download script</A>]\n";
}

print "</TD></TR></TABLE>\n";

print "<CENTER><FORM ACTION=\"$scriptURL\">\n";

print "<INPUT TYPE=\"text\" MAXLENGTH=256 ".
      "SIZE=60 NAME=\"searchString\" VALUE=\"$searchString\">\n";
    
print "<INPUT TYPE=submit VALUE=\"search\" ".
      "NAME=\"buttonSearch\">\n";

print "</FORM></CENTER>\n";



# if we have a query
if( $searchString ne "" ) {

    # to create the itunes AES key
    # in c:

    # MD5_Init(&ctx);
    # MD5_Update(&ctx,"Accept-Language",15);
    # MD5_Update(&ctx,"user-agent",10);
    # MD5_Update(&ctx,"max-age",7);
    # MD5_Final(key,&ctx);

    # to create the itunes AES key
    # in perl:

    # my $md5 = new MD5;
    # $md5->add( "Accept-Language" ); 
    # $md5->add( "user-agent" ); 
    # $md5->add( "max-age" ); 
    # my $passwordHash = $md5->hexdigest();

    # Since the key is static, we can just hard-code it here
    my $iTunesKeyHex = "8a9dad399fb014c131be611820d78895";

    # convert the hex key to binary
    my $iTunesKey = pack "H*", $iTunesKeyHex;


    # URL-encode the search string
    $searchStringSafe = escape( $searchString );



    # fetch the encrypted results from apple
    my $searchResultWithHTTPHeaders = 
        getEncryptedResultsWithHTTPHeaders( $searchStringSafe );
    
    # wget no longer works, since it also sends its own 
    # User-Agent: Wget/1.5.3
    # header, causing Apples new (v4.5) server to reject the request
    # have wget print the search results, with HTTP headers, to std out
    # my $searchResultWithHTTPHeaders = `wget http://phobos.apple.com/WebObjects/MZSearch.woa/wa/com.apple.jingle.search.DirectAction/search?term=$searchStringSafe -s --header="User-Agent: iTunes/4.0 (Macintosh; U; PPC Mac OS X 10.2)" --header="Accept-Encoding: gzip, x-aes-cbc" -O - -q`;
    

    #open( TEMP_FILE, ">/tmp/results" );
    #print TEMP_FILE $searchResultWithHTTPHeaders;
    #close( TEMP_FILE );

    # look for the initialization vector header
    ( my $ivHex ) = 
        ( $searchResultWithHTTPHeaders =~ /x-apple-crypto-iv: (\w+)/ );


    # get the content length
    ( my $contentLength ) = 
        ( $searchResultWithHTTPHeaders =~ /Content-Length: (\d+)/ );
    
    # skip the headers to get to the encrypted search results (the content)
    my $encryptedSearchResults = 
        substr( $searchResultWithHTTPHeaders,
                length( $searchResultWithHTTPHeaders ) - $contentLength );


    # convert the hex IV to binary
    my $iv = pack( "H*", $ivHex );
    
    # create the AES CBC decryption object using the iTunes key and the
    # initialization vector
    my $cipher = Crypt::CBC->new( {'key'             => $iTunesKey,
                                   'cipher'          => 'Rijndael',
                                   'iv'              => $iv,
                                   'regenerate_key'  => 0,   # default true
                                   'padding'         => 'standard',
                                   'prepend_iv'      => 0
                                   } );
    
    #my $decryptedSearchResultsGZIP = 
    #    $cipher->decrypt( $encryptedSearchResults );
    
    # seems like Apple is no longer encrypting
    $decryptedSearchResultsGZIP = $encryptedSearchResults;
    

    # take the md5 of the search string to come up with a uniqe temp file name
    my $md5 = new MD5;
    $md5->add( $searchString ); 
    my $searchStringHash = $md5->hexdigest();

    $tempFileName = "$tempFileName.$searchStringHash";


    
    # pipe the result into gzip to unzip it
    # have gzip send the result into our temp file
    open( EXTRACT_GZIP_PIPE, "| gzip -dcf > $tempFileName" ) 
        or die "Cannot start gzip pipe process: $!\n";
    
    print EXTRACT_GZIP_PIPE $decryptedSearchResultsGZIP;
    
    close( EXTRACT_GZIP_PIPE );
    

    # read the search results out of our temp file
    my $searchResultsXML = readFileValue( $tempFileName );
    
    # delete our temp file
    unlink( $tempFileName );
    

    print "<H1>Album matches:</H1>\n";
    print "<CENTER><TABLE BORDER=1 CELLPADDING=10 WIDTH=75%><TR>\n";
    
    # format all jpg images in a table
    # draggingName indicates the album name
    # url is the image URL
    $_ = $searchResultsXML;
    while( 
      m/draggingName=\"([^\"]+)\"[^>]*>\s*<[^>]*url=\"([^\"]+\.jpg)\"/gis ) {

        my $albumName = $1;
        my $safeAlbum = escape( $1 );
        print "<TD ALIGN=CENTER>".
            "<A HREF=\"$scriptURL?searchString=$safeAlbum\">".
            "<IMG BORDER=0 SRC=\"$2\"><BR>".
            "$albumName</A></TD>\n";
    }
    print "</TR></TABLE></CENTER><BR>\n";



    # extract the track list
    # the /s modifier forces a match across multiple lines
    ( my $trackListXML ) = 
        ( $searchResultsXML =~ m/(<TrackList>.*<\/TrackList>)/gis );
    
    # extract the track array from the track list
    ( my $trackArrayXML ) = 
        ( $trackListXML =~ m/(<array>.*<\/array>)/gis );
    
    # process each dict entry (each track)
    # print a table
    print "<H1>Track matches:</H1>\n";
    print "<CENTER><TABLE BORDER=1 CELLPADDING=10 WIDTH=75%>\n";
    print "<TR><TD><B>Artist</B></TD><TD><B>Album</B></TD>".
          "<TD><B>Song</B></TD>".
          # copyright field is gone
          #"<TD NOWRAP><B>Copyright<BR>(record label)</B></TD>".
          "<TD><B>Price</B></TD>".
          "<TD><B>Preview</B><BR>".
          "[try <A HREF=\"http://www.videolan.org/vlc/\">VLC</A>]</TD></TR>\n";

    $_ = $trackArrayXML;
    # use the non-greedy any-string match .*? to avoid matching across
    # multiple dictionary items
    while( m/<dict>(.*?)<\/dict>/gis ) {
        my $trackXML = $1;
        
        my $artistName = extractDictionaryKey( $trackXML, "artistName" ); 

        # skip extra <dict> entries at top
        if( defined( $artistName ) ) {

            my $albumName = extractDictionaryKey( $trackXML, "playlistName" ); 
            my $previewURL = extractDictionaryKey( $trackXML, "previewURL" ); 
            my $songName = extractDictionaryKey( $trackXML, "songName" ); 

            # copyright field is gone
            # my $recordLabel = extractDictionaryKey( $trackXML, "copyright" );
            
            # replace weird (unicode?) copyright &#8471; with a standard &copy;
            # $recordLabel =~ s/&\#8471;/&copy;/;
            
            my $priceInMilliDollars = 
                extractDictionaryKey( $trackXML, "price" );
            
            my $priceInDollars = $priceInMilliDollars / 1000;
            
            my $priceString = sprintf( "%.2f", $priceInDollars );
            
            my $safeArtist = escape( $artistName );
            my $safeAlbum = escape( $albumName );
            my $safeSong = escape( $songName );

        
            print 
                "<TR><TD><A HREF=\"$scriptURL?searchString=$safeArtist\">".
                    "$artistName</A></TD>".
                     "<TD><A HREF=\"$scriptURL?searchString=$safeAlbum\">".
                         "$albumName</A></TD>".
                     "<TD><A HREF=\"$scriptURL?searchString=$safeSong\">".
                         "$songName</A></TD>".
                     # copyright field is gone
                     #"<TD>$recordLabel</TD>".
                     "<TD NOWRAP>\$$priceString</TD>".
                     "<TD NOWRAP><A HREF=\"$previewURL\">play preview</A>".
                     "</TD></TR>\n";
        }
    }
    
    print "</TABLE></CENTER>\n";
}


# print the end of our HTML doc
print "</BODY></HTML>\n";



use IO::Socket::INET;

##
# Fetches encrypted search results from the iTunes music store, complete
# with HTTP headers.
#
# Written by nand (nandhp@myrealbox.com) in response to Apples new v4.5
# iTunes server.
#
# @param0 the URL-safe encoded search string.
##
sub getEncryptedResultsWithHTTPHeaders {
    my $searchStringSafe = $_[0];
    
    my $sock = IO::Socket::INET->new( "phobos.apple.com:80") 
               or return "";
    
    binmode( $sock );
    print $sock 
      "GET /WebObjects/MZSearch.woa/wa/".
          "com.apple.jingle.search.DirectAction/search?term=".
          "$searchStringSafe HTTP/1.1\r\n".
      "Accept-Language: en-us, en;q=0.50\r\n".
      "User-Agent: iTunes/4.0 (Macintosh; U; PPC Mac OS X 10.2)\r\n".
      "Cookie: countryVerified=1\r\n".
      "Accept-Encoding: gzip, x-aes-cbc\r\n".
      "Host: phobos.apple.com\r\n".
      "Connection: close\r\n\r\n";
    
    my $response = "";
    my $buffer;
    
    # read buffers of response until connection is closed
    while( read( $sock, $buffer, 2560 ) ) {
        $response = $response . $buffer;
    }
    #print $response;
    return $response;
}



##
# Extracts a key value from dictionary XML.
#
# @param0 the dictionary XML.
# @param1 the key name to extract.
#
# @return the key value.
##
sub extractDictionaryKey {
    my $dictXML = $_[0];
    my $keyName = $_[1];
    
    # values can be wrapped with many different tags, 
    # like <string> or <integer>
    # use <\w+> to match all of these possible value tags
    ( my $keyValue ) = 
        ( $dictXML =~ 
          m/<key>$keyName<\/key>\s*<\w+>([^<]*)<\/\w+>/i );
    
    return $keyValue;
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
