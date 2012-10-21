<?php


// enable verbose error reporting to detect uninitialized variables
error_reporting( E_ALL );


// ensure that magic quotes are OFF
if( get_magic_quotes_gpc() ) {
    // force magic quotes to be removed
    $_GET     = array_map( 'em_stripslashes_deep', $_GET );
    $_POST    = array_map( 'em_stripslashes_deep', $_POST );
    $_REQUEST = array_map( 'em_stripslashes_deep', $_REQUEST );
    $_COOKIE  = array_map( 'em_stripslashes_deep', $_COOKIE );
    }


/**
 * Recursively applies the stripslashes function to arrays of arrays.
 * This effectively disables magic_quote escaping behavior. 
 *
 * @inValue the value or array to stripslashes from.
 *
 * @return the value or array with slashes removed.
 */
function em_stripslashes_deep( $inValue ) {
    return
        ( is_array( $inValue )
          ? array_map( 'em_stripslashes_deep', $inValue )
          : stripslashes( $inValue ) );
    }



$rowSize = 20;


$mapping = array( "a" => "i",
                  "b" => "c",
                  "c" => "b",
                  "d" => "l",
                  "e" => "y",
                  "f" => "u",
                  "g" => "n",
                  "h" => "j",
                  "i" => "a",
                  "j" => "h",
                  "k" => "r",
                  "l" => "d",
                  "m" => "x",
                  "n" => "g",
                  "o" => "q",
                  "p" => "s",
                  "q" => "o",
                  "r" => "k",
                  "s" => "p",
                  "t" => "z",
                  "u" => "f",
                  "v" => "w",
                  "w" => "v",
                  "x" => "m",
                  "y" => "e",
                  "z" => "t" );


echo "<center><table border=1 cellpadding=10><tr>";
$i = 0;
foreach( $mapping as $letterA => $letterB ) {
    echo "<td align=center><table border=1>".
        "<tr><td align=center><font size=5>$letterA</font></td></tr>".
        "<tr><td align=center>:</td></tr>".
        "<tr><td align=center><font size=5>$letterB</font></td></tr>".
        "</table></td>";
    $i++;
    if( $i == 13 ) {
        echo "</tr><tr><td colspan=13></td></tr></tr>";
        }
    }
echo "</tr></table></center><hr>";



function encodeMessage( $inMessage ) {
    global $mapping;

    $encodedMessage = "";
    

    $inMessage = strtolower( $inMessage );
    
    $messageArray = str_split( $inMessage );
    
    foreach( $messageArray as $letter ) {
        $encodedChar = $letter;

        if( array_key_exists( $letter, $mapping ) ) {    
            $encodedChar = $mapping[ $letter ];
            }
        
        $encodedMessage = $encodedMessage . $encodedChar;
        }
    return $encodedMessage;
    }





$message = "";
if( isset( $_REQUEST[ "message" ] ) ) {
    $message = $_REQUEST[ "message" ];
    }


$encoded = encodeMessage( $message );

?>

<center>
<font size=6>

<?php



$messages = preg_split( "/-+/", $encoded );

$numMessages = count( $messages );

$i = 0;
foreach( $messages as $nextMessage ) {
    echo "<br>$nextMessage<br><br>\n";
    if( $i < $numMessages - 1 ) {
        echo "<hr>\n";
        }
    $i++;
    }

?>
</font>
</center>


<?php


/*
$doubleEncoded = encodeMessage( $encoded );


echo "Message:  $message<br> encoded: $encoded<br> back: $doubleEncoded";
*/

