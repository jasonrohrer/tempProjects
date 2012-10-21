<?php


// enable verbose error reporting to detect uninitialized variables
error_reporting( E_ALL );


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

/*
foreach( $mapping as $letter ) {
    echo "checking $letter:  ";

    if( $mapping[ $mapping[$letter] ] == $letter ) {
        echo "okay<br>";
        }
    else {
        echo "failed<br>";
        }
    }
*/


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

$message = "Test this";





$message = "";
if( isset( $_REQUEST[ "message" ] ) ) {
    $message = $_REQUEST[ "message" ];
    }


$encoded = encodeMessage( $message );

?>

<center>

<table width=75% border=1>
<tr>

<?php

$words = preg_split( "/\s+/", $encoded );

$rowLetterCount = 0;

$blankRowContents = array();


foreach( $words as $word ) {

    $rowLetterCount += strlen( $word ) + 1;

    if( $rowLetterCount > $rowSize ) {
        // word too long for this line

        $rowLetterCount -= strlen( $word ) + 1;

        while( $rowLetterCount < $rowSize ) {
            // pad it
            echo "<td></td>";
            $rowLetterCount ++;
            }
        echo "</tr>\n";

        echo "<tr>";
        foreach( $blankRowContents as $blank ) {
            echo "<td align=center>$blank</td>";
            }
        echo "</tr>";
        
        $rowLetterCount = 0;
        $blankRowContents = "";
        echo "<tr>";
        }
    
    $wordArray = str_split( $word );

    foreach( $wordArray as $letter ) {
        echo "<td align=center>$letter</td>";
        $blankRowContents[] = "_";
        }
    echo "<td> </td>";
    $blankRowContents[] = " ";
    }

?>

</tr>

<?php
echo "<tr>";
foreach( $blankRowContents as $blank ) {
    echo "<td align=center>$blank</td>";
    }
echo "</tr>";
?>
</table>
</center>

<?php


/*
$doubleEncoded = encodeMessage( $encoded );


echo "Message:  $message<br> encoded: $encoded<br> back: $doubleEncoded";
*/

