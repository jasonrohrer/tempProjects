<?php


include_once( "common.php" );





$monthsPast = mu_requestFilter( "months_past", "/\d+/", 6);
$searchTerm = strtolower( mu_requestFilter( "search_term", "/(\w+\s*)*/", "" ) );


mu_checkPassword( "eventHosts" );

    // show form:

        // form for searching houses
?>
    <FORM ACTION="eventSearch.php" METHOD="get">
    [<a href="logoutMembers.php">Back to Homespun</a>]<br><br>
    Search for events containing the word or phrase
    <INPUT TYPE="text" MAXLENGTH=20 SIZE=10 NAME="search_term"
             VALUE="<?php echo $searchTerm;?>"> <br>
    hosted in the past 
    <INPUT TYPE="text" MAXLENGTH=5 SIZE=5 NAME="months_past"
             VALUE="<?php echo $monthsPast;?>"> months<br>
    <INPUT TYPE="Submit" VALUE="Search">
    </FORM>
<?php

if( $monthsPast != 0 && $searchTerm != "" ) {
        
    global $apiURL, $apiKey;
    
    
    $nextUrl ="$apiURL".
        "events.xml?group_urlname=homespun&".
        "status=past&".
        "time=-$monthsPast"."m,&fields=name,description,time,event_url&".
        "key=$apiKey";

    $totalEventCount = 0;
    $matchingEventCount = 0;

    // reverse their order to show most recent first
    $resultHTMLBlocks = array();

    date_default_timezone_set( 'America/Los_Angeles' );
    
    
    while( strlen( $nextUrl ) > 5 ) {
        
    
        $result = trim( file_get_contents( $nextUrl ) );
        
        
        $xml = simplexml_load_string( $result );

        if( $xml === FALSE ) {
            echo "Parsing Meetup data failed<br>";
            }


        
        
        foreach( $xml->items->item as $item ) {
            $totalEventCount ++;

            $date = date("Y F j (l) g:i A", $item->time / 1000);
            $name = $item->name;
            $description = $item->description;
            $eventURL = $item->event_url;
            
            if( stripos( $name, $searchTerm ) !== false ||
                stripos( $description, $searchTerm ) !== false ) {
                // match
                $matchingEventCount++;

                // bold matches
                $upperTerm = strtoupper( $searchTerm );
                $name =
                    str_ireplace ( $searchTerm , "<b>$upperTerm</b>", $name );

                $description =
                    str_ireplace ( $searchTerm , "<b>$upperTerm</b>",
                                   $description );
                array_unshift(
                    $resultHTMLBlocks,
                    "<font size=5><a href='$eventURL'>$name</a></font><br>".
                    "<table border=1 cellspacing=0 cellpadding=20><tr><td>".
                    "[$date]".
                    // descriptions seem to start with <p>
                    "$description</td></tr></table><br><br><br>" );
                }
            }


        // another page of results?
        $nextUrl = $xml->head->next;
        }

    $matchWord = "matches";
    if( $matchingEventCount == 1 ) {
        $matchWord = "match";
        }
    echo "Counted <b>$matchingEventCount</b> $matchWord in ".
    "<b>$totalEventCount</b> total events.<br><br>";
    
    foreach( $resultHTMLBlocks as $block ) {
        echo $block;
        }
    

    

    }



?>