<style>
.metascore_font {
   color: #fff;
   font-family: Arial,Helvetica,sans-serif;
   font-style: normal;
   font-weight: bold;
   font-size: 25px;
   }
.title_font{
   color: #000;
   font-family: Arial,Helvetica,sans-serif;
   font-style: normal;
   font-weight: bold;
   font-size: 18px;
   text-decoration: none;	 
   }
.title_font:hover{
   color: #FC3;
    }

.row_table {
    border-bottom-width: 1px;
    border-bottom-style: solid;
	border-bottom-color: #ccc; 
}    
</style>

<?php


class Game {
    public $url;
    public $title;
    public $platform;
    public $metaScore;
    public $dev;
    }

/*
$headersToUse = "";


foreach (getallheaders() as $name => $value) {

      if( $name != "Host" && $name != "Accept-Encoding" ) {

          $headersToUse = $headersToUse . "$name: $value\r\n";
          }
      echo "$name: $value<br>";
    }
*/

$opts = array(
    'http'=>array(
        'method'=>"GET",
        'header'=>"Connection: keep-alive\r\n".
        "Cache-Control: max-age=0\r\n".
        "Upgrade-Insecure-Requests: 1\r\n".
        "User-Agent: Mozilla/5.0 (Windows NT 6.1; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/56.0.2924.87 Safari/537.36\r\n".
        "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8\r\n".
        "Accept-Language: en-GB,en-US;q=0.8,en;q=0.6\r\n"
        )
    );


$context = stream_context_create( $opts );

$mainURL = "http://www.metacritic.com/browse/games/score/metascore/90day/filtered";


$list = file_get_contents( $mainURL, false, $context );


$lines = array();


foreach( preg_split("/((\r?\n)|(\r\n?))/", $list ) as $line ){

    $lines[] = $line;
    }


// up to 10 pages
for( $p=1; $p<10; $p++ ) {
    
    $listB = file_get_contents( $mainURL . "&page=$p", false, $context );


    foreach( preg_split("/((\r?\n)|(\r\n?))/", $listB ) as $line ){

        $lines[] = $line;
        }
    
    }

$games = array();
$numPrinted = 0;
$numToSkip = 0;
$numSkipped = 0;

$maxToPrint = 9999;

?>

<center>
<table border=0><tr><td>
<?php

for( $i=0; $i<count( $lines ) && $numPrinted < $maxToPrint; $i++ ) {
    if( preg_match( "/product_item product_title/", $lines[$i] ) ) {
        preg_match( "/href=\"([^\"]+)\"/", $lines[$i+1], $matches );
        
        $gameURL = $matches[1];

        $gameURL = "http://www.metacritic.com" . $gameURL . "/critic-reviews";

        if( $numToSkip > $numSkipped ) {
            $numSkipped ++;
            }
        else {
            
            $game = printReportForGameURL( $gameURL );
            
            if( $game != FALSE && $game->title != "" ) {
                $games[] = $game;
                $numPrinted ++;
                }
            }
        }
    }


function sortGamesByDev( $a, $b ) {
    if( $a->dev == $b->dev ){
        return 0;
        }

    return ( $a->dev < $b->dev ) ? 1 : -1;
    }


usort( $games, 'sortGamesByDev' );


echo "<h2>Games from the <a href=$mainURL>past 90 days</a><br>Sorted by Metascore Standard Deviation:</h2>";

echo "<table border=0>";

$gameIndex = 1;

foreach( $games as $game ) {
    echo "<tr><td class=row_table>";
    printGameLine( $gameIndex, $game->url, $game->title, $game->platform,
                   $game->metaScore, $game->dev );
    echo "</td></tr>";
    $gameIndex++;
    }
echo "</table><br>";


function printGameLine( $index, $inURL, $inTitle, $inPlatform,
                        $inMetaScore, $inDev ) {
    echo "<table border=0 cellpadding=10><tr>";

    $color = "#f00";

    if( $inMetaScore > 39 ) {
        $color = "#fc3";
        }
    if( $inMetaScore > 60 ) {
        $color = "#6c3";
        }
    echo "<td align=left width=20>$index</td>".
        "<td align=center><table border=0 cellpadding=10>".
        "<tr><td bgcolor=$color align=center class=metascore_font width=30>".
        "$inMetaScore</td></tr></table></td>";
    
    echo "<td width=250><a class=title_font href=$inURL>$inTitle</a> ($inPlatform)</td>";

    
    echo "<td>(&sigma;:$inDev)</td>";

    echo "</tr></table>";
    }

echo "<br><br>";
$date = date('m/d/Y h:i:s a T', time());

echo "Data fetched $date.<br><br>";
echo "Code by <a href=http://hcsoftware.sf.net/jason-rohrer>Jason Rohrer</a>".
" (Source is <a href=https://sourceforge.net/p/hcsoftware/tempProjects/ci/default/tree/metascoreStdDev/>here</a>).";
echo "<br><br><br><br>";

?>
</td></tr></table>
</center>
<?php

function printReportForGameURL( $inURL ) {
    global $context;
    
    $list = file_get_contents( $inURL, false, $context );

    if( $list === FALSE ) {
        if( preg_match( "/429/", $http_response_header[0] ) ) {
            // too many requests
            // try sleeping and retrying
            sleep( 10 );
            
            $list = file_get_contents( $inURL, false, $context );
            }       
        }
    

    $lines = array();

    $title = "";
    $platform = "";
    
    foreach( preg_split("/((\r?\n)|(\r\n?))/", $list ) as $line ){

        $lines[] = $line;

        if( preg_match( "/og:title/", $line ) ) {
            preg_match( "/content=\"([^\"]+)\"/", $line, $matches );
        
            $title = $matches[1];
            }
        }

    
    if( $title == "" ) {
        return FALSE;
        }
    
    $metaScore;
    
    $scores = array();
    $scoreSum = 0;
    
    for( $i=0; $i<count( $lines ); $i++ ) {
        if( preg_match( "/ratingValue/", $lines[$i] ) ) {
            preg_match( "/>([0-9]+)</", $lines[$i], $matches );
            $metaScore = $matches[1];
            }
        
        if( preg_match( "/metascore_w medium.+indiv/", $lines[$i] ) ) {
            $didMatch = preg_match( "/>([0-9]+)</", $lines[$i], $matches );

            if( $didMatch ) {
                $s = $matches[1];
                //echo "score = $s<br>";
                $scores[] = $s;
                $scoreSum += $s;
                }
            }

        if( preg_match( "/span class=\"platform\"/", $lines[$i] ) ) {
            preg_match( "/([0-9a-zA-z]+.*[0-9a-zA-z]+)/",
                        $lines[$i+2], $matches );
            $platform = $matches[1];
            }
            
        }
    
    $meanScore = $scoreSum / count( $scores );

    $sumSquares = 0;

    foreach( $scores as $s ) {

        $sumSquares += pow( ( $s - $meanScore ), 2 );
        }
    $stdDev = sqrt( $sumSquares / count( $scores ) );
    $devText = round( $stdDev, 2 );


    //printGameLine( $inURL, $title, $platform, $metaScore, $devText );
    
    //echo "<br>";

    
    $returnObj = new Game();
    $returnObj->url = $inURL;
    $returnObj->title = $title;
    $returnObj->platform = $platform;
    $returnObj->metaScore = $metaScore;
    $returnObj->dev = $devText;
    
    flush();    

    return $returnObj;
    }

?>