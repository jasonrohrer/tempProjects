package collaboRate::util;

#
# modification History
#
# 2003-August-16   Jason Rohrer
# Created.



use collaboRate::common;



##
# Converts a numerical rating to a phrase.
#
# @param0 the numerical rating.
#
# @return the rating as a phrase.
#
# Example:
# my $rating = getRatingPhrase( "4" );
##
sub getRatingPhrase {
    my $number = $_[0];

    if( $number == 0 ) {
        return "bad link";
    }
    elsif( $number == 1 ) {
        return "terrible";
    }
    elsif( $number == 2 ) {
        return "poor";
    }
    elsif( $number == 3 ) {
        return "don't care";
    }
    elsif( $number == 4 ) {
        return "unrated";
    }
    elsif( $number == 5 ) {
        return "fair";
    }
    elsif( $number == 6 ) {
        return "good";
    }
    elsif( $number == 7 ) {
        return "excellent";
    }
    else {
        return "unknown";
    }
}



##
# Gets a user's rating of an item.
#
# @param0 the username.
# @param1 the item number.
#
# @return the rating as a string.
#
# Example:
# my $rating = getUserRating( "testUser", "10" );
##
sub getUserRating {
    my $username = $_[0];
    my $itemNumber = $_[1];
    
    my $count = getItemCount();
    if( $itemNumber < $count ) {
        
        # open for reading
        open( RATINGS, 
              "$dataDirectory/users/$username/ratings" );
        flock( RATINGS, 1 );
        # each item represented in 2 bytes
        seek( RATINGS, 2 * $itemNumber, 0 );
             
        my $rating;
        read( RATINGS, $rating, 1 ); 
        
        close( RATINGS );
        
        return $rating;
    }
    else {
        return 4;
    }
}



##
# Sets a user's rating of an item.
#
# @param0 the username.
# @param1 the item number.
# @param2 the rating.
#
# Example:
# setUserRating( "testUser", "10", "2" );
##
sub setUserRating {
    my $username = $_[0];
    my $itemNumber = $_[1];
    my $rating = $_[2];

    # bound rating
    if( $rating < 0 ) {
        $rating = 0;
    }
    if( $rating > 7 ) {
        $rating = 7;
    }

    my $count = getItemCount();

    if( $itemNumber < $count ) {
        # open for reading/writing, position at beginning
        open( RATINGS, 
              "+<$dataDirectory/users/$username/ratings" );
        flock( RATINGS, 2 );
        # each item represented in 2 bytes
        seek( RATINGS, 2 * $itemNumber, 0 );
                
        print RATINGS "$rating ";
                
        close( RATINGS );
    }
}



##
# Gets the number of items.
#
# @return the item count.
#
# Example:
# my $count = getItemCount();
##
sub getItemCount {
    my $count = readFileValue( "$dataDirectory/ratedItems_count" );
    return $count;
}



##
# Gets an item's URL.
#
# @param0 the item number.
#
# @return the URL.
#
# Example:
# my $url = getItemURL( "10" );
##
sub getItemURL {
    my $itemNumber = $_[0];

    my $count = getItemCount();
    if( $itemNumber < $count ) {
        my $currentIndex = 0;
        my $itemURL = "";
        
        open( ITEM_FILE, "$dataDirectory/ratedItems_urls" ) or die;
        flock( ITEM_FILE, 1 ) or die;
        
        while( <ITEM_FILE> ){
            chomp;
            if( $currentIndex == $itemNumber ) {
                $itemURL = $_;
            }
            $currentIndex ++;
        }
        
        close( ITEM_FILE );

        return $itemURL;
    }
    else {
        return "";
    }
}



##
# Gets an item's name.
#
# @param0 the item number.
#
# @return the name.
#
# Example:
# my $name = getItemName( "10" );
##
sub getItemName {
    my $itemNumber = $_[0];

    my $count = getItemCount();
    if( $itemNumber < $count ) {
        my $currentIndex = 0;
        my $itemName = "";
        
        open( ITEM_FILE, "$dataDirectory/ratedItems_names" ) or die;
        flock( ITEM_FILE, 1 ) or die;
        
        while( <ITEM_FILE> ){
            chomp;
            if( $currentIndex == $itemNumber ) {
                $itemName = $_;
            }
            $currentIndex ++;
        }
        
        close( ITEM_FILE );

        return $itemName;
    }
    else {
        return "";
    }
}


# end of package
1;
