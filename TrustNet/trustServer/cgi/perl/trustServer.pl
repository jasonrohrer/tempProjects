#!/usr/bin/perl -wT

#
# Modification History
#
# 2004-November-13  Jason Rohrer
# Created.
#
# 2005-January-3  Jason Rohrer
# Added support for getting trust relationships.
# Added support for getting trust sets (lists of trusted/distrusted users).
#
# 2005-January-6  Jason Rohrer
# Changed so a given IP/port cannot appear more than once on the host list.
# Fixed bugs in setTrustRelationship.  Fixed umask.
# Added start of support for root user.
#
# 2005-January-7  Jason Rohrer
# Changed so that all sets in update algorithm are single-membership.
#
# 2005-January-9  Jason Rohrer
# Changed to reject setTrustRelationship calls to trust one's self.
#
# 2005-January-10  Jason Rohrer
# Switched on strict mode and cleaned up many bugs.
#



# these user options can be set below
my $dataDirectory;
my $errorLogPath;
my $logPath;
my $signatureCheckerPath;
my $maxHostListSize;


BEGIN { 

    # BEGIN USER OPTIONS

    # path to the data directory
    # script must be able to create files in this directory
    $dataDirectory = "../../cgi-data/trustServerData";

    
    # path to the error log
    # script must be able to create/write to this file
    $errorLogPath = "/tmp/trustServer_errors.log";


    # path to the general log
    # script must be able to create/write to this file
    $logPath = "/tmp/trustServer.log";


    # path to the checkSignature application
    # script must be able to read/execute this file
    $signatureCheckerPath = "./checkSignature";
    
    # maximum number of hosts to keep in our cache
    $maxHostListSize = 200;

    # END USER OPTIONS

    
    use CGI::Carp qw( carpout );
    open( LOG, ">>$errorLogPath" ) or
        die( "Unable to open $errorLogPath: $!\n" );
    carpout( LOG );
}


# make taint checking happy
$ENV{ 'ENV' } = "";
$ENV{ 'PATH' } = "";



use CGI;
use CGI qw/escape unescape/;
use strict;


# allow group to write to our data files
umask( oct( "02" ) );


# make sure our host list file exists
if( not -e "$dataDirectory/hostList" ) {
    writeFile( "$dataDirectory/hostList", "" );
}

# make sure our ID file exists
if( not -e "$dataDirectory/nextFreeID" ) {
    writeFile( "$dataDirectory/nextFreeID", "0 1" );
}

# make sure our users folder exists
if( not -e "$dataDirectory/users" ) {
    makeDirectory( "$dataDirectory/users", oct( "0777" ) );
}


my $cgiQuery = CGI->new();


my $fullURL = $cgiQuery->self_url;
addToFile( $logPath, "Got request for:  $fullURL\n\n"  );


# all actions return plain text
print $cgiQuery->header( 'text/plain' );



my $action = $cgiQuery->param( "action" ) || '';


if( $action eq "createAccount" ) {
    
    my $userName = $cgiQuery->param( "userName" ) || '';
    my $publicKey = $cgiQuery->param( "publicKey" ) || '';

    # untaint
    ( $userName ) = ( $userName =~ /(\w+)/ );

    # keep lock file open and locked while we are creating a new user
    open( NEW_USER_LOCK_FILE, ">$dataDirectory/newUser.lock" ) or die;
    flock( NEW_USER_LOCK_FILE, 2 ) or die;

    # make sure that this user does not already exist
    if( $userName eq "" or 
        -e "$dataDirectory/users/$userName" ) {

        print "NAME_EXISTS";
    }
    else {
        
        my $nextIDString = readFileValue( "$dataDirectory/nextFreeID" );

        my @nextIDParts = split( /\s+/, $nextIDString );

        # grab this ID as our new ID
        my $newIDPartA = $nextIDParts[0];
        my $newIDPartB = $nextIDParts[1];

        
        # untaint
        ( $newIDPartA ) = ( $newIDPartA =~ /(\d+)/ );
        ( $newIDPartB ) = ( $newIDPartB =~ /(\d+)/ );
        
        # increment the ID
        if( $nextIDParts[1] == 4294967296 ) {
            # low-order part is maxed out

            # carry into high-order part
            $nextIDParts[0] ++;
            
            $nextIDParts[1] = 0;
        }
        else {
            $nextIDParts[1] ++;
        }

        # save the next free ID
        my $nextPartA = $nextIDParts[0];
        my $nextPartB = $nextIDParts[1];
        writeFile( "$dataDirectory/nextFreeID", "$nextPartA $nextPartB" );
        
        my $idString = "$newIDPartA" . "_$newIDPartB";

        
        writeFile( "$dataDirectory/users/$idString.name", $userName );
        writeFile( "$dataDirectory/users/$idString.key", $publicKey );
        
        # users start out with the minimum trust score
        writeFile( "$dataDirectory/users/$idString.trustScore", "0" );
        
        writeFile( "$dataDirectory/users/$userName", "$idString" );

        # create trust lists for this user
        writeFile( "$dataDirectory/users/$idString.trustList", "" );
        writeFile( "$dataDirectory/users/$idString.distrustList", "" );
        
        

        # add the ID to our user list
        my $userListFile = "$dataDirectory/userList";
        addToFile( $userListFile, "$idString\n" );
        

        # print the ID that we read
        print $idString;
    }

    # unlock
    close( NEW_USER_LOCK_FILE );
}
elsif( $action eq "getTrustScore" ) {
    my $partA = $cgiQuery->param( "userIDPartA" );
    my $partB = $cgiQuery->param( "userIDPartB" );
    
    my $value = readUserFile( $partA, $partB, "trust" );
    
    if( $value eq "" ) {
        # user does not exist
        print "0";
    }
    else {

        # override trust to 1.0 for root user
        my $rootFileName = "$dataDirectory/rootUser";
        my $isRoot = 0;

        if( -e $rootFileName ) {
            my $rootUser = readFileValue( $rootFileName );

            my $thisID = "$partA" . "_" . "$partB";

            if( $rootUser =~ m/$thisID/ ) {
                $isRoot = 1;
                print "1";
            }
        }

        if( not $isRoot ) {
            print $value;
        }
    }
}
elsif( $action eq "getPublicKey" ) {
    my $partA = $cgiQuery->param( "userIDPartA" );
    my $partB = $cgiQuery->param( "userIDPartB" );
    
    my $value = readUserFile( $partA, $partB, "key" );
    
    if( $value eq "" ) {
        # user does not exist
        print "0";
    }
    else {
        print $value;
    }
}
elsif( $action eq "getUserName" ) {
    my $partA = $cgiQuery->param( "userIDPartA" );
    my $partB = $cgiQuery->param( "userIDPartB" );

    my $value = readUserFile( $partA, $partB, "name" );

    if( $value eq "" ) {
        # user does not exist
        print "-";
    }
    else {
        print $value;
    }
}

elsif( $action eq "getUserID" ) {
    my $name = $cgiQuery->param( "userName" ) || '';

    # untaint 
    ( $name ) = ( $name =~ /(\w+)/ );
    
    my $fileName = "$dataDirectory/users/$name";


    if( not -e $fileName ) {
        # user does not exist
        print "-";
    }
    else {
        my $value = readFileValue( $fileName );

        if( $value eq "" ) {
            # user does not exist
            print "-";
        }
        else {
            print $value;
        }
    }
}
elsif( $action eq "setTrustRelationship" ) {
    my $partA = $cgiQuery->param( "userIDPartA" );
    my $partB = $cgiQuery->param( "userIDPartB" );

    my $trustedPartA = $cgiQuery->param( "trustedIDPartA" );
    my $trustedPartB = $cgiQuery->param( "trustedIDPartB" );

    # untaint
    ( $trustedPartA ) = ( $trustedPartA =~ /(\d+)/ );
    ( $trustedPartB ) = ( $trustedPartB =~ /(\d+)/ );
    
    
    if( $partA eq $trustedPartA and
        $partB eq $trustedPartB ) {
        
        # user trying to trust him/herself
        print "FAILED";
    }
    else {

        my $type = $cgiQuery->param( "type" ) || '';

        # untaint
        ( $type ) = ( $type =~ m/([a-z]+)/i );

        my $signatureHex = $cgiQuery->param( "signature" ) || '';
        
        # untaint
        ( $signatureHex ) = ( $signatureHex =~ m/([A-F0-9]+)/i );


        if( $type eq "trusted" or
            $type eq "neutral" or
            $type eq "distrusted" ) {

            # check the signature
            my $publicKey = readUserFile( $partA, $partB, "key" );
            
            # untaint
            ( $publicKey ) = ( $publicKey =~ m/([A-F0-9]+)/i );

            if( $publicKey ne "" and 
                $signatureHex ne "" ) {
                
                my $signedString = 
                    "$trustedPartA" . "_" . "$trustedPartB" . "_" . "$type";

                open( CHECK_SIG_PIPE, 
            "$signatureCheckerPath $publicKey $signatureHex $signedString |" );

                my $checkResult = <CHECK_SIG_PIPE>;
                chomp( $checkResult );
                
                close( CHECK_SIG_PIPE );
                
                if( $checkResult eq "OK" ) {
                    # signature correct
                    
                    my $oldTrustedList = readUserFile( $partA, $partB, 
                                                       "trustList" );
                    my $oldDistrustedList = readUserFile( $partA, $partB, 
                                                          "distrustList" );
                    my $trustedIDString = 
                        "$trustedPartA" . "_" . "$trustedPartB";

                    if( $type eq "trusted" ) {
                        $oldTrustedList = addString( $oldTrustedList, 
                                                     $trustedIDString, 0 );
                        $oldDistrustedList = removeString( $oldDistrustedList, 
                                                           $trustedIDString );
                    }
                    elsif( $type eq "distrusted" ) {
                        $oldTrustedList = removeString( $oldTrustedList, 
                                                        $trustedIDString );
                        $oldDistrustedList = addString( $oldDistrustedList, 
                                                        $trustedIDString, 0 );
                    }
                    elsif( $type eq "neutral" ) {
                        $oldTrustedList = removeString( $oldTrustedList, 
                                                        $trustedIDString );
                        $oldDistrustedList = removeString( $oldDistrustedList, 
                                                           $trustedIDString );
                    }
                    
                    # save the new lists
                    writeUserFile( $partA, $partB, 
                                   "trustList", $oldTrustedList );
                    writeUserFile( $partA, $partB, 
                                   "distrustList", $oldDistrustedList );

                    print "OK";
                }
                else {
                    print "FAILED";
                }
            }
            else {
                print "FAILED";
            }
        }
        else {
            print "FAILED";
        }
    }
}
elsif( $action eq "getTrustRelationship" ) {
    my $partA = $cgiQuery->param( "userIDPartA" );
    my $partB = $cgiQuery->param( "userIDPartB" );

    my $trustedPartA = $cgiQuery->param( "trustedIDPartA" );
    my $trustedPartB = $cgiQuery->param( "trustedIDPartB" );

    my $trustedList = readUserFile( $partA, $partB, 
                                       "trustList" );
    my $distrustedList = readUserFile( $partA, $partB, 
                                                      "distrustList" );
    my $trustedIDString = 
        "$trustedPartA" . "_" . "$trustedPartB";

    if( $trustedList =~ m/$trustedIDString/ ) {
        print "trusted";
    }
    elsif( $distrustedList =~ m/$trustedIDString/ ) {
        print "distrusted";
    }
    else {
        print "neutral";
    }
}
elsif( $action eq "listTrustSet" ) {
    my $partA = $cgiQuery->param( "userIDPartA" );
    my $partB = $cgiQuery->param( "userIDPartB" );

    my $type = $cgiQuery->param( "type" ) || '';

    if( $type eq "trusted" or
        $type eq "distrusted" ) {
        
        my $listName;

        if( $type eq "trusted" ) {
            $listName = "trustList";
        }
        else {
            $listName = "distrustList";
        }

        my $setList = readUserFile( $partA, $partB, $listName );
        
        if( countStrings( $setList ) > 0 ) {
            print $setList;   
        }
        else {
            print "NONE";
        }
    }
    else {
        # type is not recognized, so set defaults to empty
        print "NONE";
    }

}
elsif( $action eq "postAddress" ) {
    my $partA = $cgiQuery->param( "userIDPartA" );
    my $partB = $cgiQuery->param( "userIDPartB" );

    my $ipAddress = $cgiQuery->param( "ipAddress" ) || '';
    
    # untaint
    ( $ipAddress ) = ( $ipAddress =~ m/([\.0-9]+)/ );

    my $port = $cgiQuery->param( "port" );

    # untaint
    ( $port ) = ( $port =~ m/(\d+)/ );

   
    my $signatureHex = $cgiQuery->param( "signature" ) || '';
    
    # untaint
    ( $signatureHex ) = ( $signatureHex =~ m/([A-F0-9]+)/i );


    # check the signature
    my $publicKey = readUserFile( $partA, $partB, "key" );
    
    # untaint
    ( $publicKey ) = ( $publicKey =~ m/([A-F0-9]+)/i );

    if( $publicKey ne "" and 
        $signatureHex ne "" ) {
        
        my $signedString = 
            "$ipAddress" . "_" . "$port";

        open( CHECK_SIG_PIPE, 
         "$signatureCheckerPath $publicKey $signatureHex $signedString |" );

        my $checkResult = <CHECK_SIG_PIPE>;
        chomp( $checkResult );
            
        close( CHECK_SIG_PIPE );
            
        if( $checkResult eq "OK" ) {
            # signature correct
                
            my $oldHostList = readFileValue( "$dataDirectory/hostList" );


            my @lines = split( /\s+/, $oldHostList );
            
            # make sure the IP/port does not already exist on the list
            # for a different user ID
            my $ipAndPort = "$ipAddress" . "_" . "$port";

            my @cleanedLines = ();

            foreach my $line ( @lines ) {
                if( $line =~ m/$ipAndPort/ ) {
                    # skip this line
                }
                else {
                    # keep this line
                    push( @cleanedLines, $line );
                }
            }

            @lines = @cleanedLines;

            $oldHostList = join( "\n", @lines );


            my $hostString = "$partA" . "_" . "$partB" . "_" .
                "$ipAddress" . "_" . "$port";
            
            # add to the top of the list
            my $newHostList = addString( $oldHostList, $hostString, 1 );
            
            

            @lines = split( /\s+/, $newHostList );

            # make sure the list is not too long
            while( scalar( @lines ) > $maxHostListSize ) {
                # trim off the oldest entry
                pop( @lines );
            }
            
            $newHostList = join( "\n", @lines );
            
            # save the new lists
            writeFile( "$dataDirectory/hostList", $newHostList );
            
            print "OK";
        }
        else {
            print "FAILED";
        }
    }
}
elsif( $action eq "getHostList" ) {
    my $hostList = readFileValue( "$dataDirectory/hostList" );
    
    print $hostList;
}
elsif( $action eq "updateTrustScores" ) {
    
    # initialize:  set trust score of all users to 0
    
    my $userListFileName = "$dataDirectory/userList";
    
    if( -e $userListFileName ) {
        my $userListString = readFileValue( $userListFileName );
        
        my @userList = split( /\s+/, $userListString );
        
        foreach my $user ( @userList ) {
            # untaint
            ( $user ) = ( $user =~ /(\d+_\d+)/ );
            
            my $userTrustFileName = "$dataDirectory/users/$user.trustScore";

            writeFile( $userTrustFileName, "0" );
        }
    }


    # need a root user to run trust update alg.
    # do nothing if root does not exist

    my $rootFileName = "$dataDirectory/rootUser";    
    if( -e $rootFileName ) {

        my $rootUser = readFileValue( $rootFileName );

        # untaint
        ( $rootUser ) = ( $rootUser =~ /(\d+_\d+)/ );
                
        
        
        # part 1
        # walk the tree of trust arcs to increase the trust scores of users

        print "Update part 1:  Follow trust arcs to increase trust scores";

        # keep track of the ancestors of each node that we hit
        # so that we can avoid looping back
        my %ancestorLists = ();

        
        # start off with a set containing only the root user
        my @currentSet = ();
        push( @currentSet, $rootUser );

        # add this to the score of the users in the currentSet at each step
        my $currentScoreDelta = 1; 
        
        my $stepCount = 0;

        # keep going until current score delta gets very small (1/10000)
        # or our set is empty
        while( $currentScoreDelta > 0.0001 and
               scalar( @currentSet ) > 0 ) {
            
            my $setSize = scalar( @currentSet );

            # print report for this step
            print "\n\nStep $stepCount, set of size $setSize, " .
                "score increment of $currentScoreDelta\n";
            
            print "Users updated at this level:  ";
            
            # add the delta to the current set and compute the new set
            # this set is single-membership
            my @nextSet = ();

            foreach my $user ( @currentSet ) {
                
                # make sure this user has an ancestor list
                if( not exists( $ancestorLists{ $user } ) ) {
                    # add an empty list for this user
                    my @emptyArray = ();
                    $ancestorLists{ $user } = [ @emptyArray ];
                }
                
                my $trustScoreFile = "$dataDirectory/users/$user.trustScore"; 
                my $trustScore = readFileValue( $trustScoreFile );
                
                $trustScore += $currentScoreDelta;
                # cap at 1
                if( $trustScore > 1 ) {
                    $trustScore = 1;
                }
                writeFile( $trustScoreFile, "$trustScore" );

                # print report for this user
                print "[$user ~ $trustScore]  ";

                # follow this user's trust arcs and add trusted users
                # to the nextSet, skipping any ancestors that we encounter
                my $trustListFile = "$dataDirectory/users/$user.trustList";
                
                my $trustListString = readFileValue( $trustListFile );

                my @trustList = split( /\s+/, $trustListString );

                my @currentUserAncestorList = @{ $ancestorLists{ $user } };
                
                
                foreach my $trustedUser ( @trustList ) {
                    # untaint
                    ( $trustedUser ) = ( $trustedUser =~ /(\d+_\d+)/ );
                    
                    
                    # make sure this trusted user has not already been
                    # added to nextSet
                    my $alreadyAdded = doesStringExistInArray( $trustedUser,
                                                               @nextSet );

                    if( not $alreadyAdded ) {
                        # make sure this trusted user is not an ancestor
                        # of the current user
                    
                        my $isAncestor = 0;

                        foreach my $ancestor ( @currentUserAncestorList ) {
                            if( $trustedUser eq $ancestor ) {
                                $isAncestor = 1;
                            }
                        }

                        if( not $isAncestor ) {

                            # add to next set
                            push( @nextSet, $trustedUser );

                            # update this user's ancestor list

                            if( not 
                                exists( $ancestorLists{ $trustedUser } ) ) {
                                
                                # add an empty list for this user
                                $ancestorLists{ $trustedUser } = [ ];
                            }
                            my @trustedUserAncestorList = 
                                @{ $ancestorLists{ $trustedUser } };

                            # add this trusted user's parent to the array
                            push( @trustedUserAncestorList, $user );

                            # add all ancestors of this trusted user's parent
                            foreach my $ancestor ( @currentUserAncestorList ) {
                                push( @trustedUserAncestorList, $ancestor );
                            }

                            # re-insert the list into the hash
                            $ancestorLists{ $trustedUser } = 
                                [ @trustedUserAncestorList ];
                        }
                    }
                }
            }
            
            # use next set in next step
            @currentSet = @nextSet;

            # update the delta
            $currentScoreDelta = $currentScoreDelta / 2;

            $stepCount ++;
        }
        
        if( scalar( @currentSet ) == 0 ) {
            print "\n\nStopping because current set is empty\n";
        }
        else {
            print "\n\nStopping because score increment became small " .
                "($currentScoreDelta)\n";
        }
        


        # part 2
        # decrease scores based on distrust arcs
        print "\n\nUpdate part 2:  Use distrust arcs to decrease " . 
            "trust scores";

        # keep track of the ancestors of each node that we hit
        # so that we can avoid looping back
        %ancestorLists = ();

        
        # start off with a set containing only the root user
        @currentSet = ();
        push( @currentSet, $rootUser );

        # subtract this from the score of the users that are distrusted by
        # the users in the currentSet at each step
        $currentScoreDelta = 1; 
        
        $stepCount = 0;

        # keep going until current score delta gets very small (1/10000)
        # or our set is empty
        while( $currentScoreDelta > 0.0001 and
               scalar( @currentSet ) > 0 ) {
            
            my $setSize = scalar( @currentSet );

            # print report for this step
            print "\n\nStep $stepCount, set of size $setSize, " .
                "score increment of $currentScoreDelta\n";
            
            print "Users updated at this level:  ";
            
            # subtract the delta from users distrusted by to the current set 
            # and compute the new set
            # this set is single-membership (for efficiency)
            my @nextSet = ();

            # the list of users distrusted by users in the currentSet
            # this set is single-membership (for robustness against attack)
            my @currentDistrustedSet = ();

            foreach my $user ( @currentSet ) {
                
                # make sure this user has an ancestor list
                if( not exists( $ancestorLists{ $user } ) ) {
                    # add an empty list for this user
                    my @emptyArray = ();
                    $ancestorLists{ $user } = [ @emptyArray ];
                }
                

                # follow this user's trust arcs and add trusted users
                # to the nextSet, skipping any ancestors that we encounter
                my $trustListFile = "$dataDirectory/users/$user.trustList";
                
                my $trustListString = readFileValue( $trustListFile );

                my @trustList = split( /\s+/, $trustListString );

                my @currentUserAncestorList = @{ $ancestorLists{ $user } };
                
                
                foreach my $trustedUser ( @trustList ) {
                    # untaint
                    ( $trustedUser ) = ( $trustedUser =~ /(\d+_\d+)/ );
                    
                    # make sure this trusted user has not already been
                    # added to nextSet
                    my $alreadyAdded = doesStringExistInArray( $trustedUser,
                                                               @nextSet );
                    
                    if( not $alreadyAdded ) {
                        # make sure this trusted user is not an ancestor
                        # of the current user
                    
                        my $isAncestor = 0;

                        foreach my $ancestor ( @currentUserAncestorList ) {
                            if( $trustedUser eq $ancestor ) {
                                $isAncestor = 1;
                            }
                        }

                        if( not $isAncestor ) {

                            # add to next set
                            push( @nextSet, $trustedUser );

                            # update this user's ancestor list

                            if( not 
                                exists( $ancestorLists{ $trustedUser } ) ) {
                                
                                # add an empty list for this user
                                $ancestorLists{ $trustedUser } = [ ];
                            }
                            my @trustedUserAncestorList = 
                                @{ $ancestorLists{ $trustedUser } };

                            # add this trusted user's parent to the array
                            push( @trustedUserAncestorList, $user );

                            # add all ancestors of this trusted user's parent
                            foreach my $ancestor ( @currentUserAncestorList ) {
                                push( @trustedUserAncestorList, $ancestor );
                            }

                            # re-insert the list into the hash
                            $ancestorLists{ $trustedUser } = 
                                [ @trustedUserAncestorList ];
                        }
                    }
                }

                # follow this user's distrust arcs and penalize distrusted 
                # users, skipping any ancestors that we encounter
                my $distrustListFile = 
                    "$dataDirectory/users/$user.distrustList";
                
                my $distrustListString = readFileValue( $distrustListFile );

                my @distrustList = split( /\s+/, $distrustListString );
                
                foreach my $distrustedUser ( @distrustList ) {
                    # untaint
                    ( $distrustedUser ) = ( $distrustedUser =~ /(\d+_\d+)/ );
                    

                    # make sure this distrusted user has not already been
                    # added to currentDistrustedSet
                    my $alreadyAdded = 
                        doesStringExistInArray( $distrustedUser,
                                                @currentDistrustedSet );
                    
                    if( not $alreadyAdded ) {

                        # make sure this distrusted user is not an ancestor
                        # of the current user
                    
                        my $isAncestor = 0;

                        foreach my $ancestor ( @currentUserAncestorList ) {
                            if( $distrustedUser eq $ancestor ) {
                                $isAncestor = 1;
                            }
                        }

                        if( not $isAncestor ) {
                            
                            # add distrusted user to the set
                            push( @currentDistrustedSet, $distrustedUser );
                        }
                        
                    }
                    
                }

            }
            
            # penalize all users in the distrusted set
            foreach my $distrustedUser ( @currentDistrustedSet ) {
                my $trustScoreFile = 
                    "$dataDirectory/users/$distrustedUser.trustScore"; 
                my $trustScore = readFileValue( $trustScoreFile );
                
                $trustScore -= $currentScoreDelta;
                
                # bottom at 0
                if( $trustScore < 0 ) {
                    $trustScore = 0;
                }
                writeFile( $trustScoreFile, "$trustScore" );

                # print report for this user
                print "[$distrustedUser ~ $trustScore]  ";
            }


            # use next set in next step
            @currentSet = @nextSet;

            # update the delta
            $currentScoreDelta = $currentScoreDelta / 2;

            $stepCount ++;
        }
        
        if( scalar( @currentSet ) == 0 ) {
            print "\n\nStopping because current set is empty\n";
        }
        else {
            print "\n\nStopping because score increment became small " .
                "($currentScoreDelta)\n";
        }



    }
        
}



##
# Gets whether a string exists in an array.
#
# @param0 the string to check for.
# @param1 the array.
#
# @return 1 if the string exists, or 0 otherwise.
##
sub doesStringExistInArray {
    ( my $checkString, my @array ) = @_;

    foreach my $string ( @array ) {
        if( $string eq $checkString ) {
            return 1;
        }
    }

    return 0;
}



##
# Gets whether a string exists in a whitespace-delimited stringlist.
#
# @param0 the string list.
# @param1 the string to check for.
#
# @return 1 if the string exists, or 0 otherwise.
##
sub doesStringExist {
    my $mainString = $_[0];
    my $checkString = $_[1];
    
    my @existingStrings = split( /\s+/, $mainString );
    
    my $exists = 0;

    foreach my $string ( @existingStrings ) {
        if( $string eq $checkString ) {
            return 1;
        }
    }

    return 0;
}



##
# Counts strings in a whitespace-delimited stringlist.
#
# @param0 the string list.
#
# @return the string count.
##
sub countStrings {
    my $mainString = $_[0];

    my @existingStrings = split( /\s+/, $mainString );
    
    return scalar( @existingStrings );
}



##
# Adds a string to a whitespace-delimited stringlist if it does not already 
# exist.
#
# @param0 the string list.
# @param1 the string to add.
# @param2 1 to add the string to the beginning, or 0 to add it to the end
#   of the string list.
#
# @return the modified string.
##
sub addString {
    my $mainString = $_[0];
    my $newString = $_[1];
    my $addToBeginning = $_[2];


    my @existingStrings = split( /\s+/, $mainString );

    my $exists = 0;

    foreach my $string ( @existingStrings ) {
        if( $string eq $newString ) {
            $exists = 1;
        }
    }

    if( not $exists and not $addToBeginning ) {
        push( @existingStrings, $newString );
    }
    elsif( not $exists and $addToBeginning ) {
        unshift( @existingStrings, $newString );
    }

    return join( "\n", @existingStrings );
}



##
# Removes a string from a whitespace-delimited stringlist if it exists.
#
# @param0 the string list.
# @param1 the string to remove.
#
# @return the modified string.
##
sub removeString {
    my $mainString = $_[0];
    my $removeString = $_[1];
    
    my @existingStrings = split( /\s+/, $mainString );
    my @newStrings = ();


    foreach my $string ( @existingStrings ) {
        if( $string ne $removeString ) {
            push( @newStrings, $string );
        }
    }


    return join( "\n", @newStrings );
}



##
# Reads a value from a user file.
#
# @param0 part A of the user ID.  Will be taint checked.
# @param1 part B of the user ID.  Will be taint checked.
# @param2 the file extension.  Will be taint checked.
#
# @return the file value, or "" if reading fails.
##
sub readUserFile {
    my $partA = $_[0];
    my $partB = $_[1];
    my $extension = $_[2];
    

    # untaint
    ( $partA ) = ( $partA =~ /(\d+)/ );
    ( $partB ) = ( $partB =~ /(\d+)/ );
    ( $extension ) = ( $extension =~ /(\w+)/ );
    
    if( $partA eq "" or $partB eq "" ) {
        return "";
    }
    else {
        my $fileName = 
            "$dataDirectory/users/$partA" . "_$partB.$extension";

        if( not -e $fileName ) {
            return "";
        }
        else {
            return readFileValue( $fileName );
        }
    }
}



##
# Writes a value to a user file.
#
# @param0 part A of the user ID.  Will be taint checked.
# @param1 part B of the user ID.  Will be taint checked.
# @param2 the file extension.  Will be taint checked.
# @param3 the value to write.
##
sub writeUserFile {
    my $partA = $_[0];
    my $partB = $_[1];
    my $extension = $_[2];
    my $value = $_[3];

    # untaint
    ( $partA ) = ( $partA =~ /(\d+)/ );
    ( $partB ) = ( $partB =~ /(\d+)/ );
    ( $extension ) = ( $extension =~ /(\w+)/ );
    
    if( $partA eq "" or $partB eq "" ) {
        return;
    }
    else {
        my $fileName = 
            "$dataDirectory/users/$partA" . "_$partB.$extension";
        writeFile( $fileName, $value );
    }
}



##
# Reads file as a string.
#
# @param0 the name of the file.
#
# @return the file contents as a string.
#
# Example:
# my $value = readFileValue( "myFile.txt" );
##
sub readFileValue {
    my $fileName = $_[0];
    open( FILE, "$fileName" ) or 
        die "Failed to open file $fileName for reading";
    
    flock( FILE, 1 ) or die "Failed to lock file $fileName for reading";

    # read the entire file, set the <> separator to nothing
    local $/;

    my $value = <FILE>;
    close FILE;

    return $value;
}



##
# Writes a string to a file in the filesystem.
#
# @param0 the name of the file.
# @param1 the string to print.
#
# Example:
# writeFile( "myFile.txt", "the new contents of this file" );
##
sub writeFile {
    my $fileName = $_[0];
    my $stringToPrint = $_[1];
    
    open( FILE, ">$fileName" ) or 
        die "Failed to open file $fileName for writing";
    flock( FILE, 2 ) or die "Failed to open lock $fileName for writing";

    print FILE $stringToPrint;
        
    close FILE;
}



##
# Appends a string to a file in the filesystem.
#
# @param0 the name of the file.
# @param1 the string to append.
#
# Example:
# addToFile( "myFile.txt", "the new contents of this file" );
##
sub addToFile {
    my $fileName = $_[0];
    my $stringToPrint = $_[1];
        
    open( FILE, ">>$fileName" ) or 
        die "Failed to open file $fileName for adding";
    flock( FILE, 2 ) or die "Failed to lock file $fileName for adding";
        
    print FILE $stringToPrint;
        
    close FILE;
}



##
# Makes a directory file.
#
# @param0 the name of the directory.
# @param1 the octal permission mask.
#
# Example:
# makeDirectory( "myDir", oct( "0777" ) );
##
sub makeDirectory {
    my $fileName = $_[0];
    my $permissionMask = $_[1];
    
    mkdir( $fileName, $permissionMask );
}
