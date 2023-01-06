/*
 * Modification History
 *
 * 2004-November-13   Jason Rohrer
 * Created.
 *
 * 2004-December-7   Jason Rohrer
 * Fixed memory leaks.
 *
 * 2005-January-6   Jason Rohrer
 * Added commands for dealing with trust.
 * Added return values for functions that set server trust relationships.
 *
 * 2005-January-9   Jason Rohrer
 * Added commands for setting trust levels.
 */



#include "Primrose/api/primroseAPI.h"
#include "minorGems/system/Thread.h"
#include "minorGems/system/MutexLock.h"
#include "minorGems/system/Time.h"
#include "minorGems/util/stringUtils.h"
#include "minorGems/util/SettingsManager.h"

#include <stdio.h>
#include <string.h>


MutexLock *resultLock;

double searchResultFilterLevel = 0.0;

int netSearchCount = 0;
char *currentSearchID = NULL;

class ResultWrapper {
    public:
        struct UserID *mID;
        char mFirewallStatus;
        char *mIPAddress;
        unsigned short mTCPPort;
        unsigned short mUDPPort;
        char *mFileName;
        char *mHash;
        unsigned long mSize;

        // all params are copied internally
        ResultWrapper( struct UserID *inID,
                       char inFirewallStatus,
                       char *inIPAddress,
                       unsigned short inTCPPort,
                       unsigned short inUDPPort,
                       char *inFileName,
                       char *inHash,
                       unsigned long inSize )
            : mID( ::copy( inID ) ),
              mFirewallStatus( inFirewallStatus ),
              mIPAddress( stringDuplicate( inIPAddress ) ),
              mTCPPort( inTCPPort ),
              mUDPPort( inUDPPort ),
              mFileName( stringDuplicate( inFileName ) ),
              mHash( stringDuplicate( inHash ) ),
              mSize( inSize ) {
            
            }
        
        
        ~ResultWrapper() {
            delete mID;
            delete [] mIPAddress;
            delete [] mFileName;
            delete [] mHash;
            }

        ResultWrapper *copy() {
            return new ResultWrapper( mID, mFirewallStatus, mIPAddress,
                                      mTCPPort, mUDPPort, mFileName,
                                      mHash, mSize );
            }
    };


SimpleVector<ResultWrapper*> *results = NULL;

void searchHandler( char *inSearchID, struct UserID *inUserID,
                    char inFirewallStatus,
                    char *inIPAddress,
                    unsigned short inTCPPort,
                    unsigned short inUDPPort,
                    unsigned long inNumResults,
                    char **inFileNames, char **inHashes,
                    unsigned long *inSizes,
                    void *inExtraArgument ) {

    resultLock->lock();
    
    if( currentSearchID != NULL ) {

        if( strcmp( inSearchID, currentSearchID ) == 0 ) {
            char *name = primroseGetUserName( inUserID );

            double trustScore = primroseGetUserTrustScore( inUserID );

            if( trustScore >= searchResultFilterLevel ) {
                if( name != NULL ) {
                    printf( "\n\nResults from %s [%lu_%lu] "
                            "(trust score = %.2f):\n",
                            name, inUserID->partA, inUserID->partB,
                            trustScore );
                    delete [] name;
                    }
            
                for( unsigned long i=0; i<inNumResults; i++ ) {
                    printf( "  #%d\n"
                            "    file: %s\n"
                            "    size: %lu bytes\n"
                            "    hash: %s\n",
                            results->size(), inFileNames[i], inSizes[i],
                            inHashes[i] );
                    
                    results->push_back(
                        new ResultWrapper(
                            inUserID, inFirewallStatus, inIPAddress, inTCPPort,
                            inUDPPort, inFileNames[i], inHashes[i],
                            inSizes[i] ) );
                    }
                }

            }
        }
    
    resultLock->unlock();
    }


unsigned long downloadStartSeconds;
unsigned long downloadStartMilliseconds;

char downloadProgressHandler( int inResultCode,
                              unsigned long inTotalBytesReceived,
                              void *inExtraArgument ) {
    ResultWrapper *wrapper = (ResultWrapper *)inExtraArgument;

    if( inResultCode == PRIMROSE_DOWNLOAD_FAILED ) {
        printf( "Error while downloading file %s\n",
                wrapper->mFileName );

        // done with this result
        delete wrapper;
        }
    else {
        printf( "." );
        fflush( stdout );

        if( inTotalBytesReceived == wrapper->mSize ) {
            printf( "Finished downloading file %s\n",
                    wrapper->mFileName );

            // done with this result
            delete wrapper;

            unsigned long deltaMilliseconds =
                Time::getMillisecondsSince( downloadStartSeconds,
                                            downloadStartMilliseconds );

            double bytesPerSecond =
                (double)inTotalBytesReceived
                / ( (double)deltaMilliseconds / 1000.0 );
            double kiBytesPerSecond = bytesPerSecond / 1024;

            printf( "Speed:  %.2f KiB/second\n", kiBytesPerSecond ); 
            }
        }
    return true;
    }



char *readUserLine( char *inPrompt ) {
    printf( inPrompt );
    fflush( stdout );
    
    SimpleVector<char> *readChars = new SimpleVector<char>();


    
    int readInt = getc( stdin );

    // first, skip starting newlines
    while( readInt == '\n' || readInt == '\r' ) {
        readInt = getc( stdin );
        }
    
    while( readInt != '\n' && readInt != '\r' && readInt != EOF ) {
        readChars->push_back( (char)readInt );

        readInt = getc( stdin );
        }

    char *readLine = readChars->getElementString();

    delete readChars;

    return readLine;
    }


int readUserInt( char *inPrompt ) {

    int readInt;
    int numRead = 0;
    
    while( numRead != 1 ) {
        printf( inPrompt );
        fflush( stdout );
        
        numRead = scanf( "%d", &readInt );

        if( numRead != 1 ) {
            // the next character is not an int, so we failed to scanf it
            // skip the line
            char *line = readUserLine( "" );
            delete [] line;
            printf( "\n" );
            }
        }
    return readInt;
    }



double readUserDouble( char *inPrompt ) {

    double readDouble;
    int numRead = 0;
    
    while( numRead != 1 ) {
        printf( inPrompt );
        fflush( stdout );
        
        numRead = scanf( "%lf", &readDouble );

        if( numRead != 1 ) {
            // the next character is not an double, so we failed to scanf it
            // skip the line
            char *line = readUserLine( "" );
            delete [] line;
            printf( "\n" );
            }
        }
    return readDouble;
    }



char *readUserString( char *inPrompt ) {

    char *buffer = new char[100];
    int numRead = 0;
    
    while( numRead != 1 ) {
        printf( inPrompt );
        fflush( stdout );
        
        numRead = scanf( "%99s", buffer );

        if( numRead != 1 ) {
            // the next character is not part of a string, so we failed to
            // scan it---skip the line
            char *line = readUserLine( "" );
            delete [] line;
            printf( "\n" );
            }
        }

    char *trimmedString = stringDuplicate( buffer );
    delete [] buffer;

    return trimmedString;
    }



// asks the user to enter an ID or name, and then returns the ID (mapping
// the name to an ID if necessary)
//
// Returns NULL if the user cancels
struct UserID *readUserID() {
    struct UserID *readID = NULL;

    char canceled = false;
    while( readID == NULL && ! canceled ) {
        char *idOrName = readUserString(
            "Enter user ID or user name (enter c to cancel): " );

        if( strcmp( idOrName, "c" ) == 0 ) {
            canceled = true;
            }
        else {
            
            // try parsing as ID first
            unsigned long partA, partB;
    
            int numRead = sscanf( idOrName, "%lu_%lu", &partA, &partB );

            if( numRead == 2 ) {
                readID = makeUserID( partA, partB );
                }
            else {
                // try treating as a user name
                readID = primroseGetUserID( idOrName );
                }

            if( readID == NULL ) {
                printf( "%s is not a valid ID or name\n", idOrName );
                }
            }
        delete [] idOrName;

        }

    return readID;
    }



int main() {
    resultLock = new MutexLock();
    
    char *readBuffer = new char[ 500 ];
    
    if( !primroseIsFirewallStatusSet() ) {
        printf( "Are you behind a firewall that might\n"
                "block inbound connections to Primrose? [y/n] " );
        fflush( stdout );

        scanf( "%499s", readBuffer );

        if( strstr( readBuffer, "y" ) != NULL ||
            strstr( readBuffer, "Y" ) != NULL ) {

            primroseSetFirewallStatus( true );
            }
        else {
            primroseSetFirewallStatus( false );
            }
        }

    if( !primroseDoesLocalKeyExist() ) {
        printf( "Enter random keystrokes to generate encryption keys: " );
        fflush( stdout );

        scanf( "%499s", readBuffer );

        printf( "Generating RSA encryption key pair... " );
        fflush( stdout );

        primroseCreateLocalKeyPair( readBuffer );

        printf( "Done.\n" );
        }

    if( !primroseDoesAccountExist() ) {
        printf( "Create an account on the trust server:\n" );

        char *userName = new char[ 500 ];
         
        printf( "Username: " );
        fflush( stdout );

        scanf( "%499s", userName );

        printf( "Connecting to server...\n" );
        
        int result = primroseCreateAccount( userName );


        while( result == 3 ) {
            printf( "Username \"%s\" is already taken.\n", userName );

            printf( "Username: " );
            fflush( stdout );
            
            scanf( "%499s", userName );
                        
            printf( "Connecting to server...\n" );

            result = primroseCreateAccount( userName );
            }
            
        delete [] userName;


        switch( result ) {
            case 1:
                printf( "Account created\n" );
                break;
            case 2:
                printf( "Connection failed\n" );
                break;
            case 4:
                printf(
                    "Missing required information for account creation\n" );
                break;
            default:
                printf( "Bad result code: %d\n", result );
                break;
            }
        }
        

    if( !primroseDoesLocalKeyExist() ) {

        printf( "Failed to generate keys.  Cannot start Primrose.\n" );
        return 1;
        }

    if( !primroseDoesAccountExist() ) {

        printf( "Failed to create an account.  Cannot start Primrose.\n" );
        return 1;
        }

    // else start primrose
    
    primroseStart();

    char found;
    float readValue = SettingsManager::getFloatSetting( "searchFilterLevel",
                                                        &found );
    if( found ) {
        searchResultFilterLevel = (double)readValue;
        }
    
    
    int handlerID = primroseAddSearchResultsHandler( searchHandler, NULL );


    printf( "Enter \"help\" to see a list of commands.\n" );
    
    printf( "> " );
    fflush( stdout );
    scanf( "%499s", readBuffer );
    
    while( strcmp( readBuffer, "quit" ) != 0 ) {

        if ( strstr( readBuffer, "help" ) != NULL ) {
            printf(
                "Commands:\n"
                "    help       --print this help message\n\n"
                "    search     --search the network...\n\n"
                "    get        --download a search result...\n\n"
                "    trust      --add user to trusted list...\n\n"
                "    distrust   --add user to the distrusted list...\n\n"
                "    neutralize --give user neutral status (neither\n"
                "                 trusted nor distrusted)...\n\n"
                "    list       --display the trusted and distrusted sets\n\n"
                "    level      --set minimum trust level of remote users\n"
                "                 that will be allowed to search and\n"
                "                 download from us...\n\n"
                "    filter     --set minimum trust level to filter search\n"
                "                 results with...\n\n"
                "    quit       --quit the program\n\n" );
           
            }
        else if( strstr( readBuffer, "search" ) != NULL ) {
            char *searchString =
                readUserLine( "Enter search string: " );
                
            resultLock->lock();
            if( currentSearchID != NULL ) {
                delete [] currentSearchID;
                currentSearchID = NULL;

                for( int i=0; i<results->size(); i++ ) {
                    delete *( results->getElement( i ) );
                    }
                delete results;
                results = NULL;
                }
            results = new SimpleVector<ResultWrapper*>();
            
            currentSearchID = autoSprintf( "%d", netSearchCount );
            netSearchCount++;

            double trustLevel = searchResultFilterLevel;
            
            resultLock->unlock();
            
            primroseSearch( currentSearchID, searchString,
                            100,                     
                            20,  // 20 seconds
                            trustLevel );
            
            delete [] searchString;
            }
        else if( strstr( readBuffer, "get" ) != NULL ) {
            resultLock->lock();
                
            int resultNumber =
                readUserInt( "Enter file number: " );

            int numResults = results->size();
            
            if( resultNumber >= 0 &&
                resultNumber < numResults ) {

                ResultWrapper *wrapper =
                    *( results->getElement( resultNumber ) );

                // find other copies of this file in our current result list
                SimpleVector<struct UserID *> *copyIDs =
                    new SimpleVector<struct UserID *>();
                SimpleVector<char *> *copyIPAddresses =
                    new SimpleVector<char *>();
                SimpleVector<unsigned short> *copyTCPPorts =
                    new SimpleVector<unsigned short>();
                SimpleVector<unsigned short> *copyUDPPorts =
                    new SimpleVector<unsigned short>();
                SimpleVector<char> *copyFirewallStatuses =
                    new SimpleVector<char>();

                printf( "Starting download from the following users:\n    " );
                
                for( int i=0; i<numResults; i++ ) {
                    ResultWrapper *currentWrapper =
                        *( results->getElement( i ) );

                    if( strcmp( wrapper->mHash,
                                currentWrapper->mHash ) == 0 ) {
                        // hashes match
                        copyIDs->push_back( currentWrapper->mID );
                        copyIPAddresses->push_back(
                            currentWrapper->mIPAddress );
                        copyTCPPorts->push_back( currentWrapper->mTCPPort );
                        copyUDPPorts->push_back( currentWrapper->mUDPPort );
                        copyFirewallStatuses->push_back(
                            currentWrapper->mFirewallStatus );

                        char *name =
                            primroseGetUserName( currentWrapper->mID );

                        if( name != NULL ) {
                            printf( "%s   ", name );
                            delete [] name;
                            }
                        
                        }
                    }

                int numCopies = copyIDs->size();

                struct UserID **copyIDArray = copyIDs->getElementArray();
                char **copyIPAddressArray = copyIPAddresses->getElementArray();
                unsigned short *copyTCPPortArray =
                    copyTCPPorts->getElementArray();
                unsigned short *copyUDPPortArray =
                    copyUDPPorts->getElementArray();
                char *copyFirewallStatusArray =
                    copyFirewallStatuses->getElementArray();

                delete copyIDs;
                delete copyIPAddresses;
                delete copyTCPPorts;
                delete copyUDPPorts;
                delete copyFirewallStatuses;
                                        

                Time::getCurrentTime( &downloadStartSeconds,
                                      &downloadStartMilliseconds );
                
                // wrapper is extra arg to progress handler
                primroseGetFile( numCopies,
                                 copyIDArray,
                                 copyIPAddressArray, 
                                 copyTCPPortArray,
                                 copyUDPPortArray,
                                 copyFirewallStatusArray,
                                 wrapper->mFileName,
                                 wrapper->mHash,
                                 wrapper->mSize,
                                 downloadProgressHandler,
                                 (void *)( wrapper->copy() ) );

                delete [] copyIDArray;
                delete [] copyIPAddressArray;
                delete [] copyTCPPortArray;
                delete [] copyUDPPortArray;
                delete [] copyFirewallStatusArray;
                }
            else {
                printf( "Invalid file number.\n" );
                }
            resultLock->unlock();
            }
        else if( strstr( readBuffer, "distrust" ) != NULL ) {
            struct UserID *readID = readUserID();

            if( readID != NULL ) {

                char result = primroseDistrustUser( readID );

                if( result ) {
                    char *name = primroseGetUserName( readID );
                    
                    printf( "Distrusting %s [%lu_%lu]\n",
                            name, readID->partA, readID->partB );
                    delete [] name;
                    }
                else {
                    printf( "Failed to establish distrust relationship\n" );
                    }

                delete readID;
                }
            }
        else if( strstr( readBuffer, "trust" ) != NULL ) {
            struct UserID *readID = readUserID();

            if( readID != NULL ) {

                char result = primroseTrustUser( readID );
                
                
                if( result ) {
                    char *name = primroseGetUserName( readID );
                    
                    printf( "Trusting %s [%lu_%lu]\n",
                            name, readID->partA, readID->partB );
                    delete [] name;
                    }
                else {
                    printf( "Failed to establish trust relationship\n" );
                    }
                delete readID;
                }
            }
        else if( strstr( readBuffer, "neutralize" ) != NULL ) {
            struct UserID *readID = readUserID();

            if( readID != NULL ) {

                char result = primroseNeutralTrustUser( readID );

                if( result ) {
                    char *name = primroseGetUserName( readID );
                    
                    printf( "Setting %s [%lu_%lu] to neutral status\n",
                            name, readID->partA, readID->partB );
                    delete [] name;
                    }
                else {
                    printf( "Failed to clear trust relationship\n" );
                    }
                
                delete readID;
                }
            }
        else if( strstr( readBuffer, "list" ) != NULL ) {
            int numTrusted;
            struct UserID **trustedList =
                primroseGetTrustedUsers( &numTrusted );

            printf( "Trusted:\n" );

            if( trustedList != NULL ) {
                
                for( int i=0; i<numTrusted; i++ ) {
                    char *name = primroseGetUserName( trustedList[i] );

                    if( name != NULL ) {
                        printf( "   %s [%lu_%lu]\n", name,
                                trustedList[i]->partA,
                                trustedList[i]->partB ); 
                        delete [] name;
                        }
                    else {
                        printf( "   %lu_%lu\n",
                                trustedList[i]->partA,
                                trustedList[i]->partB ); 
                        }

                    delete trustedList[i];
                    }
                delete [] trustedList;

                if( numTrusted == 0 ) {
                    printf( "   (none)\n" );
                    }
                }
            else {
                printf( "   (failed)\n" );
                }

            
            int numDistrusted;
            struct UserID **distrustedList =
                primroseGetDistrustedUsers( &numDistrusted );

            printf( "Distrusted:\n" );

            if( distrustedList != NULL ) {

                for( int i=0; i<numDistrusted; i++ ) {
                    char *name = primroseGetUserName( distrustedList[i] );

                    if( name != NULL ) {
                        printf( "   %s [%lu_%lu]\n", name,
                                distrustedList[i]->partA,
                                distrustedList[i]->partB ); 
                        delete [] name;
                        }
                    else {
                        printf( "   %lu_%lu\n",
                                distrustedList[i]->partA,
                                distrustedList[i]->partB ); 
                        }
                    
                    delete distrustedList[i];
                    }
                delete [] distrustedList;

                if( numDistrusted == 0 ) {
                    printf( "   (none)\n" );
                    }
                }
            else {
                printf( "   (failed)\n" );
                }
            
            }
        else if( strstr( readBuffer, "level" ) != NULL ) {
            double level = readUserDouble(
                "Enter minimum trust score in range [0.0, 1.0]\n"
                "of remote users that can search and download from us: " );

            if( level < 0 || level > 1 ) {
                printf( "Trust level must be in the range [0.0, 1.0]\n\n" );
                }
            else {
                primroseSetUploadTrustLevel( level );
                primroseSetLocalSearchTrustLevel( level );
                printf( "Users with trust scores less than %.2f will be\n"
                        "blocked from searching and downloading.\n\n",
                       level );
                }
            }
        else if( strstr( readBuffer, "filter" ) != NULL ) {
            double level = readUserDouble(
                "Enter minimum trust score in range [0.0, 1.0]\n"
                "of displayed search results: " );

            if( level < 0 || level > 1 ) {
                printf( "Trust level must be in the range [0.0, 1.0]\n\n" );
                }
            else {
                SettingsManager::setSetting( "searchFilterLevel",
                                             (float)level );
                resultLock->lock();
                searchResultFilterLevel = level;
                resultLock->unlock();

                printf( "Results from users with trust scores less than %.2f\n"
                       "will not be displayed.\n\n", level );
                }
            }
        
        printf( "> " );
        fflush( stdout );


        
        scanf( "%499s", readBuffer );
        }
    
    delete [] readBuffer;

    printf( "Quitting...\n" );

    primroseRemoveSearchResultsHandler( handlerID );

    
    if( currentSearchID != NULL ) {
        delete [] currentSearchID;
        currentSearchID = NULL;

        for( int i=0; i<results->size(); i++ ) {
            delete *( results->getElement( i ) );
            }
        delete results;
        results = NULL;
        }

    delete resultLock;
    

    
    primroseStop();

    return 0;
    }
