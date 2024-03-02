<?php

$simPath = "/home/jcr13/checkout/PokerHandEvaluator/cpp/examples/myPLO4";



$action = plo_requestFilter( "action", "/[A-Z_]+/i", "" );

$hands = array();

for( $i=0; $i<9; $i++ ) {
    $hands[$i] = plo_requestFilter( "hand_$i",
                                    "/[AKQJT2345678910dhcs ?]+/i",
                                    "" );
    $hands[$i] = strtolower( $hands[$i] );

    $hands[$i] = preg_replace( '/a/', "A", $hands[$i] );
    $hands[$i] = preg_replace( '/k/', "K", $hands[$i] );
    $hands[$i] = preg_replace( '/q/', "Q", $hands[$i] );
    $hands[$i] = preg_replace( '/j/', "J", $hands[$i] );
    $hands[$i] = preg_replace( '/t/', "T", $hands[$i] );
    $hands[$i] = preg_replace( '/10/', "T", $hands[$i] );
    
    // normalize unknown hands to 4 ? symbols
    if( strstr( $hands[$i], "?" ) ) {
        $hands[$i] = "? ? ? ?";
        }
    }

$flops = array();

for( $i=0; $i<2; $i++ ) {
    $flops[$i] = plo_requestFilter( "flop_$i",
                                    "/[AKQJT2345678910dhcs ]+/i", "" );
    $flops[$i] = strtolower( $flops[$i] );
    
    $flops[$i] = preg_replace( '/a/', "A", $flops[$i] );
    $flops[$i] = preg_replace( '/k/', "K", $flops[$i] );
    $flops[$i] = preg_replace( '/q/', "Q", $flops[$i] );
    $flops[$i] = preg_replace( '/j/', "J", $flops[$i] );
    $flops[$i] = preg_replace( '/t/', "T", $flops[$i] );
    $flops[$i] = preg_replace( '/10/', "T", $flops[$i] );
    }


?>
<form action=index.php method=get>
<INPUT TYPE="hidden" NAME="action" VALUE="simulate">
<table>
	<tr>
	  <td>Top Flop:</td><td><INPUT TYPE="text" MAXLENGTH=40
								   SIZE=20 NAME="flop_0"
								   VALUE="<?php echo $flops[0];?>"></td>
	</tr>
	<tr>
	  <td>Bottom Flop:</td><td><INPUT TYPE="text" MAXLENGTH=40
								   SIZE=20 NAME="flop_1"
								   VALUE="<?php echo $flops[1];?>"></td>
	</tr>
<?php
          for( $i=0; $i<9; $i++ ) {
              $h =$i+1;
              ?>
    <tr>
	<tr>
	  <td>
		Hand <?php echo $h;?>:</td>
	  <td><INPUT TYPE="text" MAXLENGTH=40
				 SIZE=20 NAME="hand_<?php echo $i;?>"
				 VALUE="<?php echo $hands[$i];?>"></td>
	</tr>
              <?php
              }

?>
	</table>
  <INPUT TYPE="Submit" VALUE="Simulate"><br>
</FORM>

<?php

if( $action == "simulate" ) {
    $handFileString = "";

    for( $i=0; $i<9; $i++ ) {
        if( strlen( $hands[$i] ) > 3 ) {
            $handFileString = $handFileString . "hand " . $hands[$i] . "\n";
            }
        }
    for( $i=0; $i<2; $i++ ) {
        if( strlen( $flops[$i] ) > 3 ) {
            $handFileString = $handFileString . "flop " . $flops[$i] . "\n";
            }
        }
  
    $tmpFileName = tempnam( "/tmp", "handFormat" );
    chmod( $tmpFileName, 0666 );
    
    file_put_contents( $tmpFileName, $handFileString );

    //$output = shell_exec( "$simPath $tmpFileName" );
    $output = shell_exec( "$simPath $tmpFileName" );

    echo "<pre>\n$output\n</pre>";

    unlink( $tmpFileName );
    }




/**
 * Filters a $_REQUEST variable using a regex match.
 *
 * Returns "" (or specified default value) if there is no EXACT match.
 * Entire variable value must match regex, with no extra characters before
 * or after part matched by regex.
 */
function plo_requestFilter( $inRequestVariable, $inRegex, $inDefault = "" ) {
    if( ! isset( $_REQUEST[ $inRequestVariable ] ) ) {
        return $inDefault;
        }

    return plo_filter( $_REQUEST[ $inRequestVariable ], $inRegex, $inDefault );
    }


/**
 * Filters a value  using a regex match.
 *
 * Returns "" (or specified default value) if there is no EXACT match.
 * Entire value must match regex, with no extra characters before
 * or after part matched by regex.
 */
function plo_filter( $inValue, $inRegex, $inDefault = "" ) {
    
    $numMatches = preg_match( $inRegex,
                              $inValue, $matches );

    if( $numMatches != 1 ) {
        return $inDefault;
        }

    if( $matches[0] == $inValue ) {
        return $matches[0];
        }
    else {
        return $inDefault;
        }
    }





?>