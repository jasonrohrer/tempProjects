#!/usr/bin/perl -wT

#!/usr/local/bin/perl -wT






#
# Modification History
#
# 2003-August-16   Jason Rohrer
# Created.
# Copied from token_word.
#



# setup a local error log
use CGI::Carp qw(carpout);
BEGIN {
    use CGI::Carp qw(carpout);
    open(LOG, ">>./collaboRate_errors.log") or
        die("Unable to open collaboRate_errors.log: $!\n");
    carpout(LOG);
}



use lib '.';

use strict;
use CGI;                # Object-Oriented
use MD5;

use collaboRate::common;
use collaboRate::util;
use collaboRate::htmlGenerator;


# allow group to write to our data files
umask( oct( "02" ) );

# make sure data directories exist
setupDataDirectory();




my $cgiQuery = CGI->new();

# always set the Pragma: no-cache directive
# this feature seems to be undocumented...
$cgiQuery->cache( 1 );


my $action = $cgiQuery->param( "action" ) || '';



# get the cookie, if it exists
my $userCookie = $cgiQuery->cookie( "loggedInUser" ) ;
my $userSessionIDCookie = $cgiQuery->cookie( "sessionID" ) ;


my $loggedInUser;
my $sessionID;
my $showQuotes;


if( $userCookie ) {
    $loggedInUser = $userCookie;
    # untaint
    ( $loggedInUser ) = ( $loggedInUser =~ /(\w+)/ );
}
else {
    $loggedInUser = '';
}

if( $userSessionIDCookie ) {
    $sessionID = $userSessionIDCookie;
    # untaint
    ( $sessionID ) = ( $sessionID =~ /(\w+)/ );
}
else {
    $sessionID = '';
}






if( $action eq "getDataTarball" ) {
    my $password = $cgiQuery->param( "password" ) || '';
    
    my $truePassword = bypass_readFileValue( "$dataDirectory/admin.pass" );

    if( $password eq $truePassword ) {
    
        print $cgiQuery->header( -type=>'application-x/gzip', -expires=>'now',
                                 -Cache_control=>'no-cache' );
        my $oldPath = $ENV{ "PATH" };
        $ENV{ "PATH" } = "";

        open( TARBALL_READER, 
  "cd $dataDirectory/..; /bin/tar cf - $dataDirectoryName | /bin/gzip -f |" );
        while( <TARBALL_READER> ) {
            print "$_";
        }
        close( TARBALL_READER );

        $ENV{ "PATH" } = $oldPath;
    }
    else {
        print $cgiQuery->header( -type=>'text/html', -expires=>'now',
                                 -Cache_control=>'no-cache' );
        print "access denied";
    }
}
elsif( $action eq "makeDataTarball" ) {
    my $password = $cgiQuery->param( "password" ) || '';
    
    my $truePassword = bypass_readFileValue( "$dataDirectory/admin.pass" );

    if( $password eq $truePassword ) {
    
        print $cgiQuery->header( -type=>'text/html', -expires=>'now',
                                 -Cache_control=>'no-cache' );
        my $oldPath = $ENV{ "PATH" };
        $ENV{ "PATH" } = "";

        my $outcome =
            `cd $dataDirectory/..; /bin/tar cf - $dataDirectoryName | /bin/gzip -f > $dataDirectoryName.tar.gz`;

        print "Outcome = $outcome <BR>(blank indicates no error)";

        $ENV{ "PATH" } = $oldPath;
    }
    else {
        print $cgiQuery->header( -type=>'text/html', -expires=>'now',
                                 -Cache_control=>'no-cache' );
        print "access denied";
    }
}
elsif( $action eq "refreshFromDataTarball" ) {
    my $password = $cgiQuery->param( "password" ) || '';
    
    my $truePassword = bypass_readFileValue( "$dataDirectory/admin.pass" );

    if( $password eq $truePassword ) {
    
        print $cgiQuery->header( -type=>'text/html', -expires=>'now',
                                 -Cache_control=>'no-cache' );
        my $oldPath = $ENV{ "PATH" };
        $ENV{ "PATH" } = "";

        my $outcome =
            `cd $dataDirectory/..; /bin/rm -r $dataDirectoryName; /bin/cat ./$dataDirectoryName.tar.gz | /bin/gzip -d - | /bin/tar xf -`;

        print "Outcome = $outcome <BR>(blank indicates no error)";

        $ENV{ "PATH" } = $oldPath;
    }
    else {
        print $cgiQuery->header( -type=>'text/html', -expires=>'now',
                                 -Cache_control=>'no-cache' );
        print "access denied";
    }
}
elsif( $action eq "updateDatabaseFromDataTarball" ) {
    my $password = $cgiQuery->param( "password" ) || '';
    
    my $truePassword = bypass_readFileValue( "$dataDirectory/admin.pass" );

    if( $password eq $truePassword ) {
    
        print $cgiQuery->header( -type=>'text/html', -expires=>'now',
                                 -Cache_control=>'no-cache' );

        updateDatabaseFromDataTarball();
    }
    else {
        print $cgiQuery->header( -type=>'text/html', -expires=>'now',
                                 -Cache_control=>'no-cache' );
        print "access denied";
    }
}
elsif( $action eq "createUserForm" ) {
    print $cgiQuery->header( -type=>'text/html', -expires=>'now',
                             -Cache_control=>'no-cache' );
    collaboRate::htmlGenerator::generateCreateUserForm( "" );
}
elsif( $action eq "createUser" ) {
    my $user = $cgiQuery->param( "user" ) || '';
    my $password = $cgiQuery->param( "password" ) || '';

    #untaint
    ( $user ) = ( $user =~ /(\w+)/ );
    ( $password ) = ( $password =~ /(\w+)/ );
    
    
    if( $user eq '' ) {
        print $cgiQuery->header( -type=>'text/html', -expires=>'now',
                                 -Cache_control=>'no-cache' );
        collaboRate::htmlGenerator::generateCreateUserForm( 
                        "invalid username" );
    }
    elsif( length( $password ) < 4 ) {
        print $cgiQuery->header( -type=>'text/html', -expires=>'now',
                             -Cache_control=>'no-cache' );
        collaboRate::htmlGenerator::generateCreateUserForm( 
                        "password must be at least 4 characters long" );
    }
    else {
        my $userDir = "$dataDirectory/users/$user";

        if( doesFileExist( $userDir ) ) {
            print $cgiQuery->header( -type=>'text/html', -expires=>'now',
                             -Cache_control=>'no-cache' );
            collaboRate::htmlGenerator::generateCreateUserForm( 
                                                 "username already exists" );
        }
        else {
            makeDirectory( "$dataDirectory/users/$user", oct( "0777" ) );
            
            writeFile( "$dataDirectory/users/$user/password", "$password" );
            
            open( ITEM_FILE, "$dataDirectory/ratedItems_urls" ) or die;
            flock( ITEM_FILE, 1 ) or die;
            open( USER_FILE, ">$dataDirectory/users/$user/ratings" ) or die;
            flock( USER_FILE, 2 ) or die;

            while( <ITEM_FILE> ){
                chomp;
                # user hasn't rated this item yet
                print USER_FILE "4 ";
            }

            close( ITEM_FILE );
            close( USER_FILE );

            writeFile( "$dataDirectory/users/$user/itemsWaitingForRating",
                       "" );
            writeFile( "$dataDirectory/users/$user/similarUsers",
                       "" );
            
            addToFile( "$dataDirectory/userList", "$user\n" );

            # the user has been created... 
            # set cookie and show the main page (same as code
            # for user login)

            my $userCookie = $cgiQuery->cookie( -name=>"loggedInUser",
                                                -value=>"$user",
                                                -expires=>'+1h' );
        
            # take the MD5 hash of the username, password, 
            # and current system time 
            my $md5 = new MD5;
            $md5->add( $user, $password, time() ); 
            my $newSessionID = $md5->hexdigest();
            
            my $sessionIDCookie = $cgiQuery->cookie( -name=>"sessionID",
                                                     -value=>"$newSessionID",
                                                     -expires=>'+1h' );
            print $cgiQuery->header( -type=>'text/html',
                                     -expires=>'now',
                                     -Cache_control=>'no-cache',
                                     -cookie=>[ $userCookie, 
                                                $sessionIDCookie ] );
            $loggedInUser = $user;

            # save the new session ID
            writeFile( "$dataDirectory/users/$user/sessionID",
                       $newSessionID );

            showMainPage();
        }
    }
}
elsif( $action eq "loginForm" ) {
    print $cgiQuery->header( -type=>'text/html', -expires=>'now',
                             -Cache_control=>'no-cache' );
    collaboRate::htmlGenerator::generateLoginForm( "" );
}
elsif( $action eq "login" ) {
    my $user = $cgiQuery->param( "user" ) || '';
    my $password = $cgiQuery->param( "password" ) || '';

    #untaint
    ( $user ) = ( $user =~ /(\w+)/ );
    ( $password ) = ( $password =~ /(\w+)/ );
    
    
    my $correct = 0;
    
    if( doesFileExist( "$dataDirectory/users/$user/password" ) ) {
        my $realPassword = 
            readFileValue( "$dataDirectory/users/$user/password" );
        if( $realPassword eq $password ) {
            $correct = 1;
        }
    }

    if( not $correct ) {
        print $cgiQuery->header( -type=>'text/html', -expires=>'now',
                                 -Cache_control=>'no-cache' );
        collaboRate::htmlGenerator::generateLoginForm( "login failed" );
    }
    else {
        my $userCookie = $cgiQuery->cookie( -name=>"loggedInUser",
                                            -value=>"$user",
                                            -expires=>'+1h' );
        
        # take the MD5 hash of the username, password, 
        # and current system time 
        my $md5 = new MD5;
        $md5->add( $user, $password, time() ); 
        my $newSessionID = $md5->hexdigest();

        my $sessionIDCookie = $cgiQuery->cookie( -name=>"sessionID",
                                                 -value=>"$newSessionID",
                                                 -expires=>'+1h' );
        
        print $cgiQuery->header( -type=>'text/html',
                                 -expires=>'now',
                                 -Cache_control=>'no-cache',
                                 -cookie=>[ $userCookie, $sessionIDCookie ] );
        $loggedInUser = $user;

        # save the new session ID
        writeFile( "$dataDirectory/users/$user/sessionID",
                   $newSessionID );

        showMainPage();
    }
}
elsif( $action eq "logout" ) {
    my $userCookie = $cgiQuery->cookie( -name=>"loggedInUser",
                                        -value=>"" );
    my $sessionIDCookie = $cgiQuery->cookie( -name=>"sessionID",
                                             -value=>"" );

    print $cgiQuery->header( -type=>'text/html',
                             -expires=>'now',
                             -Cache_control=>'no-cache',
                             -cookie=>[ $userCookie, $sessionIDCookie ] );
    
    # leave the old sessionID file in place    

    if( $loggedInUser ne '' ) {
        collaboRate::htmlGenerator::generateLoginForm( 
                                      "$loggedInUser has logged out\n" );
    }
    else {
        collaboRate::htmlGenerator::generateLoginForm( "" );
    }
}
else {

    if( $loggedInUser eq '' ) {
        
        print $cgiQuery->header( -type=>'text/html', -expires=>'now',
                                 -Cache_control=>'no-cache' );

        collaboRate::htmlGenerator::generateLoginForm( "" );
    }
    elsif( doesFileExist( "$dataDirectory/users/$loggedInUser/sessionID" ) and
           $sessionID ne 
           readFileValue( "$dataDirectory/users/$loggedInUser/sessionID" ) ) {
        
        # bad session ID returned in cookie
        print $cgiQuery->header( -type=>'text/html', -expires=>'now',
                                 -Cache_control=>'no-cache' );

        collaboRate::htmlGenerator::generateLoginForm( "" );
    }
    elsif( 
        not doesFileExist( "$dataDirectory/users/$loggedInUser/sessionID" ) ) {
        # session ID file does not exist
        print $cgiQuery->header( -type=>'text/html', -expires=>'now',
                                 -Cache_control=>'no-cache' );

        collaboRate::htmlGenerator::generateLoginForm( "" );
    }
    else {
        # session ID returned in cookie is correct

        # send back a new cookie to keep the user logged in longer
        my $userCookie = $cgiQuery->cookie( -name=>"loggedInUser",
                                        -value=>"$loggedInUser",
                                        -expires=>'+1h' );
        my $sessionIDCookie = $cgiQuery->cookie( -name=>"sessionID",
                                                 -value=>"$sessionID",
                                                 -expires=>'+1h' );
        
        print $cgiQuery->header( -type=>'text/html',
                                 -expires=>'now',
                                 -Cache_control=>'no-cache',
                                 -cookie=>[ $userCookie, $sessionIDCookie ] );


        if( $action eq "test" ) {
            print "test for user $loggedInUser\n";
        }
        elsif( $action eq "feedbackForm" ) {            
            collaboRate::htmlGenerator::generateFeedbackForm( $loggedInUser );
        }
        elsif( $action eq "feedback" ) { 
            my $message = $cgiQuery->param( "message" ) || '';

            bypass_addToFile( "$dataDirectory/feedback",
                              "$loggedInUser :\n$message\n\n" );
            
            showMainPage();
        }
        elsif( $action eq "submitItemForm" ) {            
           collaboRate::htmlGenerator::generateSubmitItemForm( $loggedInUser,
                                                               "" );
        }
        elsif( $action eq "submitItem" ) {
            
            open( SUBMIT_LOCK, ">$dataDirectory/submit_lock" );
            flock( SUBMIT_LOCK, 2 );

            my $itemURL = $cgiQuery->param( "itemURL" ) || '';
            my $itemName = $cgiQuery->param( "itemName" ) || '';
           
            if( $itemURL ne "" and $itemName ne "" ) {
                my $count = readFileValue( "$dataDirectory/ratedItems_count" );
                $count += 1;
                writeFile( "$dataDirectory/ratedItems_count", $count );
 
                addToFile( "$dataDirectory/ratedItems_urls", "$itemURL\n" );
                addToFile( "$dataDirectory/ratedItems_names", "$itemName\n" );
                
                # no one has rated this yet
                open( USER_LIST, "$dataDirectory/userList" );
                flock( USER_LIST, 1 );

                while( <USER_LIST> ) {
                    chomp;
                    my $user = $_;
                    
                    #untaint
                    ( $user ) = ( $user =~ /(\w+)/ );

                    # unrated
                    addToFile( "$dataDirectory/users/$user/ratings", "4 " );
                }
                close( USER_LIST );

              collaboRate::htmlGenerator::generateItemSubmitted( 
                                                                 $loggedInUser,
                                                                 $itemURL,
                                                                 $itemName );
            }
            else {
                # missing values
              collaboRate::htmlGenerator::generateSubmitItemForm( 
                                                 $loggedInUser,
                                                 "required data missing" ); 
            }
            
            close( SUBMIT_LOCK );
        }
        elsif( $action eq "rateItem" ) {
            my $itemNumber = $cgiQuery->param( "itemNumber" ) || '';
            
            my $itemRating = $cgiQuery->param( "rating" ) || '';


            #untaint
            ( $itemNumber ) = ( $itemNumber =~ /(\d+)/ );
            ( $itemRating ) = ( $itemRating =~ /(\d+)/ );
            

            my $count = readFileValue( "$dataDirectory/ratedItems_count" );
            if( $itemNumber < $count ) {
                collaboRate::util::setUserRating( $loggedInUser, 
                                                  $itemNumber, $itemRating );
                  
                  # remove from waiting list
                  my @newWaitingList = ();

                  my $waitingFileName =
                    "$dataDirectory/users/$loggedInUser/itemsWaitingForRating";
                    

                  open( WAITING_LIST, "$waitingFileName" );
                  flock( WAITING_LIST, 2 );
                  while( <WAITING_LIST> ) {
                      chomp;
                      if( not ( $itemNumber eq $_ ) ) {
                          push( @newWaitingList, "$_\n" );
                      }
                  }
                  close( WAITING_LIST );

                  my $newListText = join( "", @newWaitingList );

                  writeFile( $waitingFileName, $newListText );

                  my $name = collaboRate::util::getItemName( $itemNumber );
                collaboRate::htmlGenerator::generateUserPage( 
                      $loggedInUser, "rating recorded for <B>$name</B>" );
            }
            else {
                collaboRate::htmlGenerator::generateErrorPage( 
                                                 $loggedInUser,
                                                 "item does not exist" );
            }
        }
        elsif( $action eq "downloadItem" ) {
            my $itemNumber = $cgiQuery->param( "itemNumber" ) || '';
            
            #untaint
            ( $itemNumber ) = ( $itemNumber =~ /(\d+)/ );
                        
            my $count = readFileValue( "$dataDirectory/ratedItems_count" );
            if( $itemNumber < $count ) {
                my $currentIndex = 0;
                my $itemURL = collaboRate::util::getItemURL( $itemNumber );

                # generate an instand refresh
                print "<META HTTP-EQUIV=\"Refresh\" ";
                print "CONTENT=\"0; URL=$itemURL\">";
                
                my $currentRating = 
                  collaboRate::util::getUserRating( $loggedInUser, 
                                                    $itemNumber );
                if( $currentRating == 4 ) {
                    my $waitingFileName =
                    "$dataDirectory/users/$loggedInUser/itemsWaitingForRating";
                    
                    # unrated
                    my $alreadyListed = 0;
                    
                    open( WAITING_LIST, $waitingFileName );
                    flock( WAITING_LIST, 1 );
                    while( <WAITING_LIST> ) {
                        chomp;
                        if( $itemNumber eq $_ ) {
                            $alreadyListed = 1;
                        }
                    }
                    close( WAITING_LIST );

                    if( not $alreadyListed ) {
                        addToFile( $waitingFileName, "$itemNumber\n" );
                    }
                }
            }
            else {
                collaboRate::htmlGenerator::generateErrorPage( 
                                                 $loggedInUser,
                                                 "item does not exist" );
            }
        }
        elsif( $action eq "listUserFavorites" ) {
            my $user = $cgiQuery->param( "user" ) || '';

            # untaint
            ( $user ) = ( $user =~ /(\w+)/ );

            if( doesFileExist( "$dataDirectory/users/$user" ) ) {
                my $userRatings = 
                    readFileValue( "$dataDirectory/users/$user/ratings" );
                
                my @ratingsList = split( / /, $userRatings );

                # sort indices by comparing their corresponding ratings
                # sort in decending order
                my @sortedIndices = 
                    sort { $ratingsList[ $b ] <=> $ratingsList[ $a ] } 
                    0 .. $#ratingsList;

                my $numItems = 10;
                if( $numItems > $#ratingsList + 1) {
                    $numItems = $#ratingsList + 1;
                }
                
                
                # trim top item count so we only have "positive" ratings
                while( $ratingsList[ $sortedIndices[ $numItems - 1 ] ] < 5
                       and $numItems > 0 ) {
                    $numItems --;
                }
                
                my @topItems = ();
                if( $numItems > 0 ) {
                    @topItems = 
                        @sortedIndices[ 0 .. ($numItems -1) ];
                }

                collaboRate::htmlGenerator::generateUserFavoritesPage( 
                                                 $loggedInUser,
                                                 $user,
                                                 @topItems );                
            }
            else {
                collaboRate::htmlGenerator::generateErrorPage( 
                                                 $loggedInUser,
                                                 "user does not exist" );
            }
        } 
        else {
            # show main page
            showMainPage();
        }
    }
}




sub showMainPage {       
  collaboRate::htmlGenerator::generateUserPage( $loggedInUser, "" );
}



sub setupDataDirectory {
    if( not -e "$dataDirectory" ) {
        
        bypass_makeDirectory( "$dataDirectory", oct( "0777" ) );
        
        makeDirectory( "$dataDirectory/users", oct( "0777" ) );

        writeFile( "$dataDirectory/userList", "" );

        writeFile( "$dataDirectory/ratedItems_urls", "" );
        writeFile( "$dataDirectory/ratedItems_names", "" );
        writeFile( "$dataDirectory/ratedItems_count", "0" );
        
        writeFile( "$dataDirectory/submit_lock", "" );

        bypass_writeFile( "$dataDirectory/admin.pass", "changeme" );
    }
}



