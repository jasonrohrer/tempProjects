package collaboRate::htmlGenerator;

#
# Modification History
#
# 2003-August-17   Jason Rohrer
# Created.
# Modified from token_word.
#


use collaboRate::common;
use collaboRate::util;



##
# Generates a generic page header with no control bars.
#
# @param0 the title of the page.
##
sub generateHeader {
    ( my $title ) = @_;

    my $headerText = bypass_readFileValue( "$htmlDirectory/header.html" );

    $headerText =~ s/<!--#TITLE-->/$title/;
    
    print $headerText;
}



##
# Generates a generic page footer with no control bars.
##
sub generateFooter {

    my $footerText = bypass_readFileValue( "$htmlDirectory/footer.html" );

    print $footerText;
}




##
# Generates a full page header for a page with control bars.
#
# @param0 the title of the page.
##
sub generateFullHeader {
    ( my $title ) = @_;

    generateHeader( $title );

    #my $headerText = bypass_readFileValue( "$htmlDirectory/fullHeader.html" );
    
    #print $headerText;
}



##
# Generates a full page footer for a page with control bars.
#
# @param0 the currently logged-in user.
##
sub generateFullFooter {
    ( my $loggedInUser ) = @_;
    
    generateFooter();
    return;

    my $balance = collaboRate::userManager::getBalance( $loggedInUser );
    my $trialBalance = 
      collaboRate::userManager::getTrialBalance( $loggedInUser );

    
    
    my $footerText = bypass_readFileValue( "$htmlDirectory/fullFooter.html" );
    
    $footerText =~ s/<!--#USER-->/$loggedInUser/;
    $footerText =~ s/<!--#TOKEN_BALANCE-->/$balance/;
    $footerText =~ s/<!--#TRIAL_TOKEN_BALANCE-->/$trialBalance/;

    # generate most quoted list
    my @mostQuoted = collaboRate::documentManager::getMostQuotedDocuments();
    
    my @mostQuotedStrings = ();
    
    # push table header
    push( @mostQuotedStrings, "<TABLE BORDER=0>\n" );
    
    foreach $quoted ( @mostQuoted ) {
        ( my $doc, $count ) = split( /\|/, $quoted );
        my @docRegions = extractRegionComponents( $doc );
        my $docOwner = $docRegions[0];
        my $docID = $docRegions[1];

        my $docTitle = collaboRate::documentManager::getDocTitle( $docOwner,
                                                                $docID );
        if( length( $docTitle ) > 20 ) {
            $docTitle = substr( $docTitle, 0, 17 );
            $docTitle = "$docTitle...";
        }
        my $quotedString =
            "<TR><TD>$docOwner\'s</TD>".
            "<TD><A HREF=\"collaboRate.pl?action=showDocument".
            "&docOwner=$docOwner".
            "&docID=$docID\">".
            "$docTitle</A></TD>".
            "<TD>[$count]</TD></TR>\n";
        push( @mostQuotedStrings, $quotedString );
    }

    if( scalar( @mostQuoted ) == 0 ) {
        push( @mostQuotedStrings, "<TR><TD>none</TD></TR>" );
    }

    # push table footer
    push( @mostQuotedStrings, "</TABLE>\n" );
    
    my $mostQuotedList = join( "", @mostQuotedStrings );
    
    $footerText =~ s/<!--#MOST_QUOTED_DOCUMENT_LIST-->/$mostQuotedList/;


    # generate most recent list
    my @mostRecent = collaboRate::documentManager::getMostRecentDocuments();
    
    my @mostRecentStrings = ();
    
    # push table header
    push( @mostRecentStrings, "<TABLE BORDER=0>\n" );
    
    foreach $doc ( @mostRecent ) {
        my @docRegions = extractRegionComponents( $doc );
        my $docOwner = $docRegions[0];
        my $docID = $docRegions[1];

        my $docTitle = collaboRate::documentManager::getDocTitle( $docOwner,
                                                                $docID );
        if( length( $docTitle ) > 20 ) {
            $docTitle = substr( $docTitle, 0, 17 );
            $docTitle = "$docTitle...";
        }
        my $recentString =
            "<TR><TD>$docOwner\'s</TD>".
            "<TD><A HREF=\"collaboRate.pl?action=showDocument".
            "&docOwner=$docOwner".
            "&docID=$docID\">".
            "$docTitle</A></TD></TR>\n";
        push( @mostRecentStrings, $recentString );
    }

    if( scalar( @mostRecent ) == 0 ) {
        push( @mostRecentStrings, "<TR><TD>none</TD></TR>" );
    }

    # push table footer
    push( @mostRecentStrings, "</TABLE>\n" );
    
    my $mostRecentList = join( "", @mostRecentStrings );
    
    $footerText =~ s/<!--#MOST_RECENT_DOCUMENT_LIST-->/$mostRecentList/;





    print $footerText;
}



##
# Generates the login form.
#
# @param0 a message to display in the form.
##
sub generateLoginForm {
    ( my $message ) = @_;
 
    generateHeader( "login" );

    my $formText = bypass_readFileValue( "$htmlDirectory/loginForm.html" );

    $formText =~ s/<!--#MESSAGE-->/$message/;
    
    print $formText;


    generateFooter();
}



##
# Generates a form for creating a new user.
#
# @param0 a message to display in the form.
##
sub generateCreateUserForm {
    ( my $message ) = @_;
 
    generateHeader( "create new user" );

    my $formText = 
        bypass_readFileValue( "$htmlDirectory/createUserForm.html" );

    $formText =~ s/<!--#MESSAGE-->/$message/;
    
    print $formText;


    generateFooter();
}



##
# Generates the feedback form.
#
# @param0 the currently logged-in user.
##
sub generateFeedbackForm {
    ( my $loggedInUser ) = @_;
 
    generateFullHeader( "feedback" );

    my $formText = bypass_readFileValue( "$htmlDirectory/feedbackForm.html" );
    
    print $formText;


    generateFullFooter( $loggedInUser );
}




##
# Generates a form for submitting and item.
#
# @param0 the currently logged-in user.
# @param1 a message to display.
##
sub generateSubmitItemForm {
    ( my $loggedInUser,
      my $message ) = @_;

    generateFullHeader( "submit item" );

    my $formText = 
        bypass_readFileValue( "$htmlDirectory/submitItemForm.html" );
    
    $formText =~ s/<!--#MESSAGE-->/$message/;
    
    print $formText;

    generateFullFooter( $loggedInUser );
}



##
# Generates a form for submitting an item.
#
# @param0 the currently logged-in user.
# @param1 a message to display.
##
sub generateItemSubmitted {
    ( my $loggedInUser,
      my $itemURL,
      my $itemName ) = @_;

    generateUserPage( $loggedInUser, "item <B>$itemName</B> submitted" );
}



##
# Generates a user's main page.
#
# @param0 the currently logged-in user.
# @param1 a message for the user.
##
sub generateUserPage {
    ( my $loggedInUser,
      my $message ) = @_;

    generateFullHeader( "main page for $loggedInUser" );


    my $pageText = 
        bypass_readFileValue( "$htmlDirectory/userPage.html" );
    
    $pageText =~ s/<!--#USER-->/$loggedInUser/;
    $pageText =~ s/<!--#MESSAGE-->/$message/;
    



    # build the new items list
    my @newItemsList = ();

    push( @newItemsList, "<TABLE BORDER=0>\n" );

    my $maxListSize = 10;
    my $itemCount = collaboRate::util::getItemCount();
    
    my $itemIndex = $itemCount - 1;
    my $listSize = 0;

    my @freshItems = ();

    while( $listSize <= $maxListSize && $itemIndex >= 0 ) {
        if( collaboRate::util::getUserRating( $loggedInUser, $itemIndex ) 
            == 4 ) {
            # unrated, count it as new
            push( @freshItems, $itemIndex );
            $listSize++;
        }
        $itemIndex--;
    }

    my $currentItem;
    for( $currentItem=0; 
         $currentItem<$listSize; $currentItem++ ) {
        
        my $itemNumber = $freshItems[ $currentItem ];

        my $name = 
          collaboRate::util::getItemName( $itemNumber);
        
        push( @newItemsList, 
              "<TR><TD><A HREF=\"collaboRate.pl?".
              "action=downloadItem&itemNumber=$itemNumber\">$name</A>".
              "</TD></TR>\n" );
    }

    if( $listSize == 0 ) {
        push( @newItemsList, "<TR><TD>none</TD></TR>\n" );
    }

    push( @newItemsList, "</TABLE>\n" );

    my $newItemsListText = join( "", @newItemsList );

    
    $pageText =~ s/<!--#NEW_ITEM_LIST-->/$newItemsListText/;






    # build the waiting items list
    my @waitingItemsList = ();

    push( @waitingItemsList, "<TABLE BORDER=0>\n" );

    open( WAITING_FILE, 
          "$dataDirectory/users/$loggedInUser/itemsWaitingForRating" );
    flock( WAITING_FILE, 1 );

    my $someWaiting = 0;
    while( <WAITING_FILE> ) {
        $someWaiting = 1;
        chomp;
        my $itemNumber = $_;
        my $name = collaboRate::util::getItemName( $itemNumber );

        push( @waitingItemsList, 
              "<TR><TD><A HREF=\"collaboRate.pl?".
              "action=downloadItem&itemNumber=$itemNumber\">$name</A>".
              "---</TD>".
              "<TD>".
              "[<A HREF=\"collaboRate.pl?action=rateItem&".
              "itemNumber=$itemNumber&rating=0\">bad link</A>]".
              "-".
              "[<A HREF=\"collaboRate.pl?action=rateItem&".
              "itemNumber=$itemNumber&rating=1\">terrible</A>]".
              "-".
              "[<A HREF=\"collaboRate.pl?action=rateItem&".
              "itemNumber=$itemNumber&rating=2\">poor</A>]".
              "-".
              "[<A HREF=\"collaboRate.pl?action=rateItem&".
              "itemNumber=$itemNumber&rating=3\">don't care</A>]".
              "-".
              "[<A HREF=\"collaboRate.pl?action=rateItem&".
              "itemNumber=$itemNumber&rating=5\">fair</A>]".
              "-".
              "[<A HREF=\"collaboRate.pl?action=rateItem&".
              "itemNumber=$itemNumber&rating=6\">good</A>]".
              "-".
              "[<A HREF=\"collaboRate.pl?action=rateItem&".
              "itemNumber=$itemNumber&rating=7\">excellent</A>]".
              "</TR>\n" ); 
    }
    close( WAITING_FILE );
    
    if( not $someWaiting ) {
        push( @waitingItemsList, "<TR><TD>none</TD></TR>\n" );
    }

    push( @waitingItemsList, "</TABLE>\n" );

    my $waitingItemsListText = join( "", @waitingItemsList );

    
    $pageText =~ s/<!--#WAITING_ITEM_LIST-->/$waitingItemsListText/;





    # build the similar users list
    my @similarUsersList = ();

    push( @similarUsersList, "<TABLE BORDER=0>\n" );

    my @ourRatings = 
        split( / /, 
               readFileValue( "$dataDirectory/users/$loggedInUser/ratings" ) );
    

    open( USER_LIST, 
          "$dataDirectory/userList" );
    flock( USER_LIST, 1 );

    my @users = ();
    my @distances = ();

    my $someFound = 0;
    while( <USER_LIST> ) {
        $someFound = 1;
        chomp;
        my $otherUser = $_;

        if( not $otherUser eq $loggedInUser ) {

            my @theirRatings = 
                split( / /, 
                 readFileValue( "$dataDirectory/users/$otherUser/ratings" ) );
        
            my $distance = 0;

            my $ratingIndex;
            for( $ratingIndex=0; $ratingIndex <= $#ourRatings; 
                 $ratingIndex++ ) {
            
                $distance += ( $theirRatings[ $ratingIndex ] -
                               $ourRatings[ $ratingIndex ] ) ** 2;
            }
            $distance = sqrt( $distance );

            push( @users, $otherUser );
            push( @distances, $distance );
        }
    }
    close( USER_LIST );
   
    
    # sort user indices by distance, shortest distance first
    my @sortedIndices = 
        sort { $distances[ $a ] <=> $distances[ $b ] } 
        0 .. $#users;
    
    my @sortedDistances = @distances[ @sortedIndices ];
    
    my @sortedUsers = @users[ @sortedIndices ];

    my $numUsers = 10;
    if( $numUsers > scalar( @sortedUsers ) ) {
        $numUsers = scalar( @sortedUsers );
    }
    
    my $userIndex;

    for( $userIndex=0; $userIndex<$numUsers; $userIndex++ ) {
        my $user = $sortedUsers[ $userIndex ];
        my $distance = $sortedDistances[ $userIndex ];

        my $distString = sprintf( "%.2f", $distance );

        push( @similarUsersList, 
              "<TR><TD><A HREF=\"collaboRate.pl?".
              "action=listUserFavorites&user=$user\">$user</A></TD>".
              "<TD>[$distString]</TD>".
              "</TR>\n" ); 
    }

    
    if( not $someFound ) {
        push( @similarUsersList, "<TR><TD>none</TD></TR>\n" );
    }

    push( @similarUsersList, "</TABLE>\n" );

    my $similarUsersListText = join( "", @similarUsersList );

    
    $pageText =~ s/<!--#SIMILAR_USER_LIST-->/$similarUsersListText/;




    

    print $pageText;


    generateFullFooter( $loggedInUser );
}



##
# Generates a list of a user's favorite items.
#
# @param0 the currently logged-in user.
# @param1 the user to list.
# @param2 a list of the user's top items.
##
sub generateUserFavoritesPage {
    ( my $loggedInUser,
      my $user,
      my @topItems ) = @_; 
    
    
    generateFullHeader( "favorite items for $user" );


    my $pageText = 
        bypass_readFileValue( "$htmlDirectory/userFavorites.html" );

    
    $pageText =~ s/<!--#USER-->/$user/;


    my $someFound = 0;

    # build the top items list
    my @topItemsList = ();

    push( @topItemsList, "<TABLE BORDER=0>\n" );

    foreach $item ( @topItems ) {
        $someFound = 1;

        my $name = 
          collaboRate::util::getItemName( $item );
        
        my $rating = 
          collaboRate::util::getUserRating( $user, $item );
        
        my $ratingPhrase =
          collaboRate::util::getRatingPhrase( $rating );

        push( @topItemsList, 
              "<TR><TD><A HREF=\"collaboRate.pl?".
              "action=downloadItem&itemNumber=$item\">$name</A></TD>".
              "<TD>[$ratingPhrase]</TD>".
              "</TR>\n" );
    }

    if( not $someFound ) {
        push( @topItemsList, "<TR><TD>none</TD></TR>\n" );
    }

    push( @topItemsList, "</TABLE>\n" );

    my $topItemsListText = join( "", @topItemsList );

    
    $pageText =~ s/<!--#TOP_ITEM_LIST-->/$topItemsListText/;
    

    print $pageText;

    generateFullFooter( $loggedInUser );
}



##
# Generates an error page.
#
# @param0 the currently logged-in user.
# @param1 the error message to display 
##
sub generateErrorPage {
    ( my $loggedInUser, my $message ) = @_;
    
    generateFullHeader( "error" );

    print "<CENTER>$message</CENTER>";

    generateFullFooter( $loggedInUser );
}



# end of package
1;
