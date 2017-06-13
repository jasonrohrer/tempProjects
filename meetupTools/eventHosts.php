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
        echo "Parsing Meetup data failed ( url = $url result = $result )<br>";      }

    $eventList = array();
    
    $memberList = array();
    
    foreach( $xml->items->item as $item ) {

        $member = array();

        $member["name"] = (string)( $item->name );
        $member["host_count"] = 0;
        $member["hosted_rsvp_count"] = 0;

        // key event id, value rsvp_count
        $member[ "hosted_events" ] = array();
        
        $id = (string)( $item->id );

        $member[ "id" ] = $id;

        $memberList["$id"] = $member;
        }

    
    $allURLList = array();
    
    
    $nextUrl ="$apiURL".
        "events.xml?group_urlname=homespun&".
        "status=past&".
        "time=-$monthsPast"."m,&fields=event_hosts&key=$apiKey";

    $totalEventCount = 0;
    $hostedEventCount = 0;
    $totalRSVPCount = 0;
    
    while( strlen( $nextUrl ) > 5 ) {
        
        $allURLList[] = $nextUrl;
        
        $result = trim( file_get_contents( $nextUrl ) );
        
        
        $xml = simplexml_load_string( $result );

        if( $xml === FALSE ) {
            echo "Parsing Meetup data failed ( url = $nextUrl result = $result )<br>";
            }


        foreach( $xml->items->item as $item ) {
            $totalEventCount ++;
            
            $rsvpCount = $item->yes_rsvp_count;

            $name = $item->name;
            $date = gmdate("Y-m-d", $item->time / 1000);

            $eventID = $item->id;
            
            $eventListing = " attended <a href=$item->event_url>$item->name</a> on $date";
            
            
            if( (bool)$item->event_hosts->event_hosts_item ) {

                $hostedEventCount++;                
                
                foreach( $item->event_hosts->event_hosts_item as $host ) {

                    // don't count the host(s) in the rsvp count
                    if( $rsvpCount > 0 ) {
                        $rsvpCount -= 1;
                        }
                    
                    
                    $id = $host->member_id;
                
                    //echo "$id\n";
                
                    $oldHostCount = $memberList["$id"]["host_count"];
                    $oldHostedRSVPCount =
                        $memberList["$id"]["hosted_rsvp_count"];
                                        
                    $memberList["$id"]["host_count"] = $oldHostCount + 1;
                    $memberList["$id"]["hosted_rsvp_count"] =
                        $oldHostedRSVPCount + $rsvpCount;

                    $memberList["$id"]["hosted_events"]["$item->event_url"] =
                        $rsvpCount;

                    $hostName = $memberList["$id"]["name"];
                    $eventListing = $eventListing .
                        " hosted by <a href=https://www.meetup.com/Homespun/members/$id>$hostName</a>";
                    
                    //echo $memberList["$id"]["name"] . "<br>";
                    //echo "   ".$memberList["$id"]["host_count"] . "<br>";
                    }
                }
            else if( $rsvpCount > 0 ) {

                // manually find the host for this event
                // (the first to RSVP for it)
                $rsvpURL ="$apiURL".
                    "rsvps.xml?event_id=$eventID".
                    "&key=$apiKey";
                
                $rsvpResult = trim( file_get_contents( $rsvpURL ) );
        
        
                $rsvpXML = simplexml_load_string( $rsvpResult );


                if( $rsvpXML === FALSE ) {
                    echo "Parsing Meetup data failed (url = $rsvpURL result = $rsvpResult )<br>";
                    }

                // way in future
                $earliestTime = 9992188300000;
                $earliestMemberID = -1;
                
                foreach( $rsvpXML->items->item as $rsvp ) {
                    
                    if( (float)$rsvp->created < (float)$earliestTime ) {
                        $earliestTime = $rsvp->created;
                        $earliestMemberID = $rsvp->member->member_id;
                        }
                    }

                if( $earliestMemberID != -1 ) {

                    $hostedEventCount++;                

                    // don't count the host(s) in the rsvp count
                    $rsvpCount -= 1;

                    
                    $id = $earliestMemberID;
                
                    //echo "$id\n";
                
                    $oldHostCount = $memberList["$id"]["host_count"];
                    $oldHostedRSVPCount =
                        $memberList["$id"]["hosted_rsvp_count"];
                                        
                    $memberList["$id"]["host_count"] = $oldHostCount + 1;
                    $memberList["$id"]["hosted_rsvp_count"] =
                        $oldHostedRSVPCount + $rsvpCount;

                    $memberList["$id"]["hosted_events"]["$item->event_url"] =
                        $rsvpCount;

                    $hostName = $memberList["$id"]["name"];
                    $eventListing = $eventListing .
                        " ( <b>no official host</b>, earliest RSVP by <a href=https://www.meetup.com/Homespun/members/$id>$hostName</a> )</b>";
                    }
                else {
                    $eventListing = $eventListing . " (<b>no host listed</b>)";
                    }
                }

            $eventListing = $rsvpCount . $eventListing;
            
            $eventList[] = $eventListing;
            

            $totalRSVPCount += $rsvpCount;
            }


        // another page of results?
        $nextUrl = $xml->head->next;
        }
    


    echo "Counted <b>$totalEventCount</b> events " .
         "attended by <b>$totalRSVPCount</b> members.<br>";
    echo "(<b>$hostedEventCount</b> of them had hosts assigned)<br><br>";

           
    
    
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

            if( $member["hosted_rsvp_count"] > $largestCount ) {
                $largestCount = $member["hosted_rsvp_count"];
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
    
            
    
    echo "<table border=1 cellpadding=10>";

    $color = "#FFFFFF";
    $colorAlt = "#EEEEEE";
    

    $ind = 1;
    
    foreach( $sortedMemberList as $member ) {

        $name = $member["name"];
        $count = $member["host_count"];
        $rsvp_count = $member["hosted_rsvp_count"];
        $id = $member["id"];

        arsort( $member["hosted_events"] );
        
        $maxURL = "";
        $max = 0;

        $urlList = array_keys( $member["hosted_events"] );
        $countList = array_values( $member["hosted_events"] );
            
        if( count( $urlList ) > 0 ) {
            $maxURL = $urlList[0];
            $max = $countList[0];
            }
            
        $eventWord = "events";

        if( $count == 1 ) {
            $eventWord = "event";
            }

        $memberWord = "members";

        if( $rsvp_count == 1 ) {
            $memberWord = "member";
            }

        $maxString = "";

        if( $count > 0 && $rsvp_count > 0 ) {

            $maxMemberWord = "members";

            if( $max == 1 ) {
                $maxMemberWord = "member";
                }
            
            $maxString = "<b>$max</b> $maxMemberWord at ".
                "<a href=$maxURL>largest event</a>";

            $numEvents = count( $urlList );
            
            if( $numEvents > 1 ) {

                for( $i = 1; $i<5 && $i < $numEvents; $i++ ) {

                    $url = $urlList[$i];
                    $c = $countList[$i];
                    
                    $maxString = $maxString . " [<a href=$url>$c</a>]";
                    }
                }
            }

        echo "<tr bgcolor=$color><td>$ind</td><td><a href=https://www.meetup.com/Homespun/members/$id>$name</a></td><td>hosted <b>$count</b> $eventWord</td><td>attended by <b>$rsvp_count</b> $memberWord</td><td>$maxString</td></tr>\n";

        $temp = $color;
        $color = $colorAlt;
        $colorAlt = $temp;

        $ind ++;
        }

    echo "</table>";

    echo "<br><br><br><br>";

    echo "Raw event dump:<br><br>\n";
    
    foreach( $eventList as $event ) {
        echo "$event<br><br>\n";
        }

    echo "<br><br><br><br>";

    echo "Fetched events from these URLs:<br><br>\n";

    foreach( $allURLList as $url ) {
        echo "<a href=$url>$url</a><br><br>\n";
        }

    }




?>