/*
 * Modification History
 *
 * 2004-November-12   Jason Rohrer
 * Created.
 *
 * 2004-January-6   Jason Rohrer
 * Added an implementation of getTrustSet.
 * Changed spec to make parsing trust sets cleaner.
 * Changed spec so that user IDs are always delimited by _
 * Added support for mapping user names to IDs.
 * Fixed a bug when bad user name specified.
 * Added code for caching trust sets.
 *
 * 2004-January-10   Jason Rohrer
 * Added support for expiration of cached trust scores.
 */



#include "Primrose/api/UserManager.h"

#include "Primrose/api/primroseAPI.h"
#include "Primrose/common/CryptoUtils.h"

#include "minorGems/network/web/WebClient.h"

#include "minorGems/util/SettingsManager.h"
#include "minorGems/util/stringUtils.h"

#include "minorGems/io/file/File.h"



struct UserID *UserManager::createAccount( char *inUserName,
                                           char *inPublicKeyHex,
                                           int *outReturnCode ) {
    char *serverURL =
        SettingsManager::getStringSetting( "trustServerURL" );

    if( serverURL == NULL ) {
        *outReturnCode = 2;
        return NULL;
        }


    char *urlWithData = autoSprintf(
        "%s?action=createAccount&userName=%s&publicKey=%s",
        serverURL, inUserName, inPublicKeyHex );

    delete [] serverURL;
    
    int resultLength;
    char *result = WebClient::getWebPage( urlWithData, &resultLength );

    delete [] urlWithData;
    
    if( result == NULL ) {
        *outReturnCode = 2;
        return NULL;
        }

    
    struct UserID *returnID = NULL;
    
    unsigned long partA, partB;

    int numRead = sscanf( result, "%lu_%lu", &partA, &partB );
    if( numRead == 2 ) {
        returnID = new struct UserID;
        returnID->partA = partA;
        returnID->partB = partB;
        
        *outReturnCode = 1;
        }
    else if( strstr( result, "NAME_EXISTS" ) != NULL ) {
        *outReturnCode = 3;
        }
    else {
        // bad response
        *outReturnCode = 2;
        }

    delete [] result;
    
    return returnID;    
    }


        
double UserManager::getTrustScore( struct UserID *inUserID,
                                   struct UserID *inTrustedID ) {

    // first try default trust score according to our relationship
    // with the user
    int trustRelationship = getTrustRelationship( inUserID, inTrustedID );

    if( trustRelationship == TRUSTED ) {
        return 1;
        }
    else if( trustRelationship == DISTRUSTED ) {
        return 0;
        }
    else {
        // neutral relationship
        
        // use system-wide trust score

        // trust scores expire afer mCacheExpirationInSeconds
        char *trustString =
            getUserString( inUserID, "trust", "getTrustScore",
                           mCacheExpirationInSeconds );

        double returnTrustScore = 0;
        sscanf( trustString, "%lf", &returnTrustScore );
        delete [] trustString;
    
        return returnTrustScore;
        }
    }



char *UserManager::getPublicKey( struct UserID *inUserID ) {
    return getUserString( inUserID, "key", "getPublicKey" );
    }






char *UserManager::getUserName( struct UserID *inUserID ) {
    char *name = getUserString( inUserID, "name", "getUserName" );

    if( name != NULL ) {
        // cache a name-to-id map
        saveNameToIDMap( inUserID, name );
        }
    
    return name;
    }



struct UserID *UserManager::getUserID( char *inUserName ) {

    struct UserID *returnID = NULL;

    
    // first, check local cache
    File *userDirectory = new File( NULL, "users" );

    if( !userDirectory->exists() ) {
        // create the directory
        userDirectory->makeDirectory();
        }
    
    if( userDirectory->isDirectory() ) {
        File *mapFile = userDirectory->getChildFile( inUserName );

        if( mapFile != NULL ) {
            if( mapFile->exists() ) {
                char *fileContents = mapFile->readFileContents();

                if( fileContents != NULL ) {
                    unsigned long partA, partB;

                    int numRead = sscanf( fileContents, "%lu_%lu",
                                          &partA, &partB );

                    delete [] fileContents;
            

                    if( numRead == 2 ) {
                        returnID = makeUserID( partA, partB );
                        }
                    }
                }
            delete mapFile;
            }
        }
    delete userDirectory;

    if( returnID != NULL ) {
        return returnID;
        }


    // else fetch from trust server

    char *serverURL =
        SettingsManager::getStringSetting( "trustServerURL" );

    if( serverURL == NULL ) {
        // don't have server URL, so cannot get value
        return NULL;
        }


    char *urlWithData = autoSprintf(
        "%s?action=getUserID&userName=%s", serverURL, inUserName );

    delete [] serverURL;


    int resultLength;
    char *result = WebClient::getWebPage( urlWithData, &resultLength );

    delete [] urlWithData;

    
    if( result == NULL ) {
        // failed to connect
        
        return NULL;
        }

    unsigned long partA, partB;
    
    int numRead = sscanf( result, "%lu_%lu", &partA, &partB );
    delete [] result;

    if( numRead == 2 ) {
        returnID = makeUserID( partA, partB );
        
        // cache the result        
        saveNameToIDMap( returnID, inUserName );
        }
    
    
    return returnID;    
    }



char *UserManager::readUserFileValue( struct UserID *inUserID,
                                      char *inFileExtension,
                                      int inMaxAgeInSeconds ) {
    char *returnValue = NULL;
    
        
    File *userDirectory = new File( NULL, "users" );

    if( !userDirectory->exists() ) {
        // create the directory
        userDirectory->makeDirectory();
        }

    if( userDirectory->isDirectory() ) {
    
        char *fileName = autoSprintf( "%lu_%lu.%s",
                                      inUserID->partA,
                                      inUserID->partB,
                                      inFileExtension );
        
        File *file = userDirectory->getChildFile( fileName );
        delete [] fileName;

        if( file != NULL ) {
            if( file->exists() ) {

                char tooOld = false;
                if( inMaxAgeInSeconds != -1 ) {
                    unsigned long fileAge =
                        time( NULL ) - file->getModificationTime();

                    if( fileAge > (unsigned long)inMaxAgeInSeconds ) {
                        tooOld = true;
                        }
                    }

                if( !tooOld ) {
                    returnValue = file->readFileContents();
                    }
                }
            
            delete file;
            }
        }
    delete userDirectory;

    
    return returnValue;
    }



void UserManager::writeUserFileValue( struct UserID *inUserID,
                                      char *inFileExtension,
                                      char *inValue ) {

    File *userDirectory = new File( NULL, "users" );

    if( !userDirectory->exists() ) {
        // create the directory
        userDirectory->makeDirectory();
        }

    if( userDirectory->isDirectory() ) {
    
        char *fileName = autoSprintf( "%lu_%lu.%s",
                                      inUserID->partA,
                                      inUserID->partB,
                                      inFileExtension );
        
        File *file = userDirectory->getChildFile( fileName );
        delete [] fileName;

        if( file != NULL ) {
            file->writeToFile( inValue );
                
            delete file;
            }
        }
    delete userDirectory;
    }



void UserManager::removeUserFile( struct UserID *inUserID,
                                  char *inFileExtension ) {

    File *userDirectory = new File( NULL, "users" );

    if( !userDirectory->exists() ) {
        // create the directory
        userDirectory->makeDirectory();
        }

    if( userDirectory->isDirectory() ) {
    
        char *fileName = autoSprintf( "%lu_%lu.%s",
                                      inUserID->partA,
                                      inUserID->partB,
                                      inFileExtension );
        
        File *file = userDirectory->getChildFile( fileName );
        delete [] fileName;

        if( file != NULL ) {
            file->remove();
                
            delete file;
            }
        }
    delete userDirectory;
    }



char *UserManager::getUserString( struct UserID *inUserID,
                                  char *inFileExtension,
                                  char *inServerAction,
                                  int inMaxAgeInSeconds ) {

    // first check for cached data 
    
    char *returnValue = readUserFileValue( inUserID, inFileExtension,
                                           inMaxAgeInSeconds );


    if( returnValue != NULL ) {
        // found a cached copy
        
        return returnValue;
        }


    // next try fetching it from the server
    
        
    char *serverURL =
        SettingsManager::getStringSetting( "trustServerURL" );

    if( serverURL == NULL ) {
        // don't have server URL, so cannot get value
        return NULL;
        }


    char *urlWithData = autoSprintf(
        "%s?action=%s&userIDPartA=%lu&userIDPartB=%lu",
        serverURL, inServerAction, inUserID->partA, inUserID->partB );

    delete [] serverURL;


    int resultLength;
    char *result = WebClient::getWebPage( urlWithData, &resultLength );

    delete [] urlWithData;

    
    if( result == NULL ) {
        // failed to connect
        
        return NULL;
        }

    // cache the result        
    writeUserFileValue( inUserID, inFileExtension, result );

    
    return result;    
    }



char UserManager::setTrustRelationship( struct UserID *inUserID,
                                        struct UserID *inTrustedID,
                                        int inType,
                                        char *inPrivateKeyHex ) {

    char *typeString;

    switch( inType ) {
        case TRUSTED:
            typeString = "trusted";
            break;
        case NEUTRAL:
            typeString = "neutral";
            break;
        case DISTRUSTED:
            typeString = "distrusted";
            break;
        default:
            typeString = "neutral";
            break;
        }

    char *stringToSign = autoSprintf( "%lu_%lu_%s",
                                      inTrustedID->partA,
                                      inTrustedID->partB,
                                      typeString );

    char *signature = CryptoUtils::rsaSign( inPrivateKeyHex,
                                            (unsigned char *)stringToSign,
                                            strlen( stringToSign ) );
    delete [] stringToSign;
    
    if( signature == NULL ) {
        return false;
        }

    
    char *serverURL =
        SettingsManager::getStringSetting( "trustServerURL" );
    
    if( serverURL == NULL ) {
        return false;
        }
    
    char *urlWithData = autoSprintf(
        "%s?action=setTrustRelationship&userIDPartA=%lu&userIDPartB=%lu&"
        "trustedIDPartA=%lu&trustedIDPartB=%lu&type=%s&signature=%s",
        serverURL, inUserID->partA, inUserID->partB,
        inTrustedID->partA, inTrustedID->partB,
        typeString, signature );

    delete [] serverURL;
    delete [] signature;
    
    int resultLength;
    char *result = WebClient::getWebPage( urlWithData, &resultLength );

    delete [] urlWithData;
    
    if( result == NULL ) {
        return false;
        }

    char success = false;
    
    if( strcmp( result, "OK" ) == 0 ) {
        success = true;

        // cache the relationship
        writeUserFileValue( inTrustedID, "relationship", typeString );

        // clear the cached trust sets
        removeUserFile( inUserID, "trustedSet" );
        removeUserFile( inUserID, "distrustedSet" );
        }

    delete [] result;

    
    return success;
    }



int UserManager::getTrustRelationship( struct UserID *inUserID,
                                       struct UserID *inTrustedID ) {

    char *typeString = readUserFileValue( inTrustedID, "relationship" );


    if( typeString == NULL ) {

        // try fetching it from the server
    
        
        char *serverURL =
            SettingsManager::getStringSetting( "trustServerURL" );

        if( serverURL == NULL ) {
            // don't have server URL, so cannot get value
            return FAILED;
            }


        char *urlWithData = autoSprintf(
            "%s?action=setTrustRelationship&userIDPartA=%lu&userIDPartB=%lu&"
            "trustedIDPartA=%lu&trustedIDPartB=%lu&type=%s&signature=%s",
            serverURL, inUserID->partA, inUserID->partB,
            inTrustedID->partA, inTrustedID->partB );

        delete [] serverURL;
        

        int resultLength;
        char *result = WebClient::getWebPage( urlWithData, &resultLength );

        delete [] urlWithData;

    
        if( result == NULL ) {
            // failed to connect
        
            return FAILED;
            }

        
        //  check for FAILED here so that we do not cache a FAILED result
        if( strstr( result, "FAILED" ) != NULL ) {
            delete [] result;

            return FAILED;
            }
        
        // cache the result        
        writeUserFileValue( inTrustedID, "relationship", result );

        typeString = result;
        }

    int returnValue;

    // must check distrusted case first, since "trusted" is a substring
    // of "distrusted"
    if( strstr( typeString, "distrusted" ) != NULL ) {
        returnValue = DISTRUSTED;
        }
    else if( strstr( typeString, "trusted" ) != NULL ) {
        returnValue = TRUSTED;
        }
    else {
        returnValue = NEUTRAL;
        }
    
    delete [] typeString;


    return returnValue;
    }



struct UserID **UserManager::getTrustSet( struct UserID *inUserID,
                                          int inType,
                                          int *outSetSize ) {
    // in case of failure
    *outSetSize = -1;
    struct UserID **returnArray = NULL;
    
    char *typeString;

    if( inType == TRUSTED ) {
        typeString = "trusted";
        }
    else {
        typeString = "distrusted";
        }


    // extension of set file
    char *setName = autoSprintf( "%sSet", typeString );


    char *setString = NULL;
    
    // first, look for cached copy (only cache trust sets for our local ID)
    struct UserID *localID = primroseGetLocalUserID();
    char isLocalID = compare( localID, inUserID );
    delete localID;

    char setWasCached = false;
    
    if( isLocalID ) {
        
        setString = readUserFileValue( inUserID, setName );

        if( setString != NULL ) {
            setWasCached = true;
            }
        }


    if( setString == NULL ) {
        // no cached copy... ask server
        
        char *serverURL =
            SettingsManager::getStringSetting( "trustServerURL" );

        if( serverURL == NULL ) {
            // don't have server URL, so cannot get value
            return NULL;
            }

        
        char *urlWithData = autoSprintf(
            "%s?action=listTrustSet&userIDPartA=%lu&userIDPartB=%lu&type=%s",
            serverURL, inUserID->partA, inUserID->partB, typeString );

        delete [] serverURL;
        

        int resultLength;
        setString = WebClient::getWebPage( urlWithData, &resultLength );

        delete [] urlWithData;

    
        if( setString == NULL ) {
            // failed to connect
            return NULL;
            }
        }

    // now parse set string, regardless of whether it was cached or
    // provided by the server

    // if the server response was NONE, then this parsing code will produce
    // the correct result (an empty list)

    SimpleVector<char *> *tokens = tokenizeString( setString );
    
    SimpleVector<struct UserID *> *scannedIDs =
        new SimpleVector<struct UserID *>();

    int numTokens = tokens->size();

    for( int i=0; i<numTokens; i++ ) {

        unsigned long partA, partB;

        char *token = *( tokens->getElement( i ) );
        int numRead = sscanf( token, "%lu_%lu", &partA, &partB );

        delete [] token;
        
        if( numRead == 2 ) {
            struct UserID *id = new struct UserID;
            id->partA = partA;
            id->partB = partB;

            scannedIDs->push_back( id );
            }
        }
    delete tokens;
    
    
    *outSetSize = scannedIDs->size();

    returnArray = scannedIDs->getElementArray();

    delete scannedIDs;


    // cache the result if it is for local ID and was not already cached
    if( isLocalID && !setWasCached ) {
        writeUserFileValue( inUserID, setName, setString );
        }

    delete [] setName;
    delete [] setString;
    
    return returnArray;    
    }



void UserManager::saveNameToIDMap( struct UserID *inUserID,
                                   char *inUserName ) {
    
    File *userDirectory = new File( NULL, "users" );

    if( !userDirectory->exists() ) {
        // create the directory
        userDirectory->makeDirectory();
        }
    
    if( userDirectory->isDirectory() ) {
        File *mapFile = userDirectory->getChildFile( inUserName );
        
        if( mapFile != NULL ) {
            char *fileContents = autoSprintf( "%lu_%lu",
                                              inUserID->partA,
                                              inUserID->partB );
            mapFile->writeToFile( fileContents );
            
            delete [] fileContents;
            delete mapFile;
            }
        }
    delete userDirectory;
    }




