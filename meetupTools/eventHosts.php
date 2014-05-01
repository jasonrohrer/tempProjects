<?php


include_once( "common.php" );





$monthsPast = mu_requestFilter( "months_past", "/\d+/", 0 );


mu_checkPassword( "eventHosts" );

    // show form:

        // form for searching houses
?>
    <FORM ACTION="eventHosts.php" METHOD="get">
    [<a href="mainMenu.php">Menu</a>]<br><br>
    Search for events hosted in the past 
    <INPUT TYPE="text" MAXLENGTH=5 SIZE=5 NAME="months_past"
             VALUE="<?php echo $monthsPast;?>"> months<br>
    <INPUT TYPE="Submit" VALUE="Search">
    </FORM>
<?php

if( $monthsPast != 0 ) {
        
    global $apiURL, $apiKey;


    $url ="$apiURL".
        "members.xml?group_urlname=homespun&key=$apiKey";
    
    $result = trim( file_get_contents( $url ) );


    $xml = simplexml_load_string( $result );

    if( $xml === FALSE ) {
        echo "Parsing Meetup data failed<br>";
        }

    $memberList = array();
    
    foreach( $xml->items->item as $item ) {

        $member = array();

        $member["name"] = (string)( $item->name );
        $member["host_count"] = 0;

        $id = (string)( $item->id );

        $memberList["$id"] = $member;
        }

    

    
    
    $url ="$apiURL".
        "events.xml?group_urlname=homespun&".
        "status=past&".
        "time=-$monthsPast"."m,&fields=event_hosts&key=$apiKey";
    
    $result = trim( file_get_contents( $url ) );


    $xml = simplexml_load_string( $result );

    if( $xml === FALSE ) {
        echo "Parsing Meetup data failed<br>";
        }


    foreach( $xml->items->item as $item ) {
        if( (bool)$item->event_hosts->event_hosts_item ) {
            
            foreach( $item->event_hosts->event_hosts_item as $host ) {
                
                $id = $host->member_id;
                
                //echo "$id\n";
                
                $oldHostCount = $memberList["$id"]["host_count"];
                
                $memberList["$id"]["host_count"] = $oldHostCount + 1;
                //echo $memberList["$id"]["name"] . "<br>";
                //echo "   ".$memberList["$id"]["host_count"] . "<br>";
                }
            }
        }
    
    
    
    //echo "Result = $result<br><br>";
    

    //$memberList = mu_parseXMLFields( $result, "member_name" );


    //$coun

    //var_dump( $memberList );

    //die();


    $sortedMemberList = array();


    $doneSorting = false;
    while( ! $doneSorting ) {

        $largestCount = -1;
        $largestCountID = "";

        foreach( $memberList as $id => $member ) {

            if( $member["host_count"] > $largestCount ) {
                $largestCount = $member["host_count"];
                $largestCountID = $id;
                }
            }
        
        if( $largestCount == -1 ) {
            $doneSorting = true;
            }
        else {
            $sortedMemberList[] = $memberList["$largestCountID"];
            unset( $memberList["$largestCountID"] );
            }
        }
    
            
    
    
    foreach( $sortedMemberList as $member ) {

        $name = $member["name"];
        $count = $member["host_count"];

        $eventWord = "events";

        if( $count == 1 ) {
            $eventWord = "event";
            }
        
        
        echo "$name hosted <b>$count</b> $eventWord<br>";
        }
    }



?>