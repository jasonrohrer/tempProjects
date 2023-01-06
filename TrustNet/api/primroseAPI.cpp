/*
 * Modification History
 *
 * 2004-November-8   Jason Rohrer
 * Created.
 *
 * 2004-November-16   Jason Rohrer
 * Added host list messages.
 *
 * 2004-December-4   Jason Rohrer
 * Fixed bug in HostAddress constructor.  Downloads now work.
 *
 * 2004-December-5   Jason Rohrer
 * Improved shutdown speed by splitting up thread sleep times.
 *
 * 2004-December-7   Jason Rohrer
 * Fixed memory leaks.
 *
 * 2005-January-6   Jason Rohrer
 * Added support for mapping user names to IDs.
 * Added a makeUserID function.
 * Added return values for functions that set server trust relationships.
 *
 * 2005-January-9   Jason Rohrer
 * Added logging messages to steps of primroseStop.
 */


#include "Primrose/api/primroseAPI.h"
#include "Primrose/api/primroseAPI_internal.h"

#include "Primrose/api/MessageManager.h"
#include "Primrose/api/UserManager.h"
#include "Primrose/api/AddressCache.h"
#include "Primrose/api/PortManager.h"
#include "Primrose/api/DownloadThread.h"

#include "Primrose/common/CryptoUtils.h"

#include "minorGems/util/SettingsManager.h"
#include "minorGems/util/stringUtils.h"
#include "minorGems/util/log/AppLog.h"
#include "minorGems/util/log/FileLog.h"

#include "minorGems/system/MutexLock.h"
#include "minorGems/system/BinarySemaphore.h"
#include "minorGems/system/Thread.h"
#include "minorGems/system/FinishedSignalThread.h"
#include "minorGems/system/StopSignalThread.h"
#include "minorGems/system/FinishedSignalThreadManager.h"


#include "minorGems/network/HostAddress.h"
#include "minorGems/network/SocketUDP.h"
#include "minorGems/network/Socket.h"
#include "minorGems/network/SocketClient.h"
#include "minorGems/network/SocketServer.h"

#include "minorGems/network/web/WebClient.h"

#include "minorGems/io/file/File.h"

#include "minorGems/crypto/hashes/sha1.h"

#include "minorGems/network/web/URLUtils.h"
#include "minorGems/formats/encodingUtils.h"

#include <time.h>


// sub-directory in shared directory where file hashes are cached 
char *primroseHashDirectoryName = "Primrose_hashes";

// sub-directory in shared directory where incoming files are kept 
char *primroseIncomingDirectoryName = "Primrose_incoming";

// 64 KiB chunks
unsigned long primroseChunkSize = 65536;//32768;//16384;



AddressCache *primroseAddressCache;
PortManager *primrosePortManager;

MessageManager *primroseMessageManager;


char *primroseLocalIPAddress;

unsigned short primroseUDPPort;
unsigned short primroseTCPPort;



double primroseLocalSearchTrustLevel;
double primroseUploadTrustLevel;

// either firewalled or not_firewalled
char *primroseFirewallStatus;

unsigned long primroseMinimumAddressCacheSize;


// prototype for our search result handler function
void primrose_internalSearchResultsMessageHandler(
    struct UDPAddress *inNodeAddress, struct UserID *inUserID,
    char *inMessageText, void *inExtraArgument );

int primroseSearchResultsMessageHandlerID;


// prototype for our search handler function
void primrose_internalSearchMessageHandler(
    struct UDPAddress *inNodeAddress, struct UserID *inUserID,
    char *inMessageText, void *inExtraArgument );

int primroseSearchMessageHandlerID;


// prototype for our connection request handler function
void primrose_internalConnectionRequestMessageHandler(
    struct UDPAddress *inNodeAddress, struct UserID *inUserID,
    char *inMessageText, void *inExtraArgument );

int primroseConnectionRequestMessageHandlerID;


// prototype for our host list request handler function
void primrose_internalHostListRequestMessageHandler(
    struct UDPAddress *inNodeAddress, struct UserID *inUserID,
    char *inMessageText, void *inExtraArgument );

int primroseHostListRequestMessageHandlerID;


// prototype for our host list request handler function
void primrose_internalHostListMessageHandler(
    struct UDPAddress *inNodeAddress, struct UserID *inUserID,
    char *inMessageText, void *inExtraArgument );

int primroseHostListMessageHandlerID;


// prototype for our address catcher handler function
void primrose_internalAddressCatcherMessageHandler(
    struct UDPAddress *inNodeAddress, struct UserID *inUserID,
    char *inMessageText, void *inExtraArgument );

int primroseAddressCatcherMessageHandlerID;



/**
 * Class that wraps a search results handler function and its argument
 * for storage in our vector of handlers.
 */
class SearchResultsHandlerWrapper {

    public:

        void (*mHandlerFunction)( char *, struct UserID *, char, char *,
                                  unsigned short,
                                  unsigned short,
                                  unsigned long, char **, char **,
                                  unsigned long *, void * );
        void *mExtraArgument;
    };


MutexLock *primroseResultsHandlerLock;
SimpleVector<SearchResultsHandlerWrapper *> *primroseResultsHandlerVector;
unsigned long primroseNextFreeResultsHandlerID = 0;
SimpleVector<unsigned long> *primroseResultsHandlerIDVector;

FinishedSignalThreadManager *primroseThreadManager;


struct UserID *primroseLocalUserID;
char *primroseLocalPrivateKeyHex;



/**
 * Thread that handles a socket connection.
 *
 * @author Jason Rohrer.
 */
class ConnectionThread : public FinishedSignalThread, public StopSignalThread {

    public:

        /**
         * Constructs and starts a thread.
         *
         * @param inSocket the socket for the connection.
         *   Will be closed/destroyed when this class is destroyed.
         */
        ConnectionThread( Socket *inSocket )
            : mSocket( inSocket ) {
            start();
            }


        
        // joins and destroys this thread
        ~ConnectionThread() {
            stop();
            join();
            delete mSocket;
            }


        
        // implements the Thread interface
        void run();


    private:

        Socket *mSocket;
    };



/**
 * Thread that listens for connections on our default TCP port
 * and passes each connection to a ConnectionThread.
 *
 * @author Jason Rohrer.
 */
class ServerThread : public StopSignalThread {

    public:

        
        /**
         * Constructs and starts a thread.
         *
         * @param inPort the port to listen on.
         */
        ServerThread( unsigned short inPort )
            : mPort( inPort ) {
            this->start();
            }
        
        
        /**
         * Stops, joins, and destroys this thread.
         */
        ~ServerThread() {
            stop();
            join();
            }

        
        // implements the Thread interface
        void run();

        
    private:

        unsigned short mPort;
    };



ServerThread *primroseServerThread;



/**
 * Thread that periodically wakes up and fetches more seed addresses from
 * the trust server if needed.
 *
 * @author Jason Rohrer.
 */
class SeedAddressThread : public StopSignalThread {

    public:

        
        /**
         * Constructs and starts a thread.
         *
         * @param inSleepTime the time between seed fetches in seconds.
         */
        SeedAddressThread( unsigned long inSleepTime )
            : mSleepTime( inSleepTime ) {
            this->start();
            }
        
        
        /**
         * Stops, joins, and destroys this thread.
         */
        ~SeedAddressThread() {
            stop();
            join();
            }

        
        // implements the Thread interface
        void run();

        
    private:

        unsigned long mSleepTime;

    };



SeedAddressThread *primroseSeedAddressThread;



char compare( struct UserID *inFirstID, struct UserID *inSecondID ) {
    if( inFirstID->partA == inSecondID->partA &&
        inFirstID->partB == inSecondID->partB ) {
        return true;
        }
    else {
        return false;
        }
    }



struct UserID *copy( struct UserID *inID ) {
    struct UserID *returnID = new struct UserID;
    returnID->partA = inID->partA;
    returnID->partB = inID->partB;

    return returnID;
    }



struct UserID *makeUserID( unsigned long inPartA, unsigned long inPartB ) {
    struct UserID *returnID = new struct UserID;
    returnID->partA = inPartA;
    returnID->partB = inPartB;

    return returnID;
    }



/**
 * Converts a user ID to an 8-byte big-endian binary string.
 *
 * @param inID the ID to convert.
 *   Must be destroyed by caller.
 *
 * @return the ID as a binary string.
 *   Must be destroyed by caller.
 */
unsigned char *primrose_internalUserIDToBinary( struct UserID *inID ) {
    unsigned char *returnArray = new unsigned char[ 8 ];

    returnArray[0] = (unsigned char)( inID->partA >> 24 ) & 0xFF;
    returnArray[1] = (unsigned char)( inID->partA >> 16 ) & 0xFF;
    returnArray[2] = (unsigned char)( inID->partA >>  8 ) & 0xFF;
    returnArray[3] = (unsigned char)( inID->partA >>  0 ) & 0xFF;

    returnArray[4] = (unsigned char)( inID->partB >> 24 ) & 0xFF;
    returnArray[5] = (unsigned char)( inID->partB >> 16 ) & 0xFF;
    returnArray[6] = (unsigned char)( inID->partB >>  8 ) & 0xFF;
    returnArray[7] = (unsigned char)( inID->partB >>  0 ) & 0xFF;

    return returnArray;
    }



/**
 * Converts an 8-byte big-endian binary string to a user ID.
 *
 * @param inBinaryString the 8-byte string to convert.
 *   Must be destroyed by caller.
 *
 * @return the ID.
 *   Must be destroyed by caller.
 */
struct UserID *primrose_internalBinaryToUserID(
    unsigned char *inBinaryString ) {

    struct UserID *returnID = new struct UserID;

    returnID->partA =
        (unsigned long)( inBinaryString[0] << 24 | 
                         inBinaryString[1] << 16 |
                         inBinaryString[2] <<  8 |
                         inBinaryString[3] <<  0   );
    returnID->partB =
        (unsigned long)( inBinaryString[4] << 24 | 
                         inBinaryString[5] << 16 |
                         inBinaryString[6] <<  8 |
                         inBinaryString[7] <<  0   );
    
    return returnID;
    }



/**
 * Converts two 32-bit ASCII base-10 strings a user ID.
 *
 * @param inPartAString the string representation of partA.
 *   Must be destroyed by caller.
 * @param inPartBString the string representation of partB.
 *   Must be destroyed by caller.
 *
 * @return the ID, or NULL if conversion fails.
 *   Must be destroyed by caller.
 */
struct UserID *primrose_internalStringsToUserID(
    char *inPartAString, char *inPartBString ) {

    struct UserID *returnID = new struct UserID;

    
    int numRead = sscanf( inPartAString, "%lu", &( returnID->partA ) );

    if( numRead == 1 ) {
        numRead = sscanf( inPartBString, "%lu", &( returnID->partB ) );

        if( numRead == 1 ) {
            return returnID;
            }
        }

    
    // else failed
    delete returnID;

    return NULL;
    }



/**
 * Computes the SHA1 hash of a file.
 *
 * @param inFile the file to hash.
 *   Must be destroyed by caller.
 *
 * @return the SHA1 hash as a hex-encoded string, or NULL on an error.
 *   Must be destroyed by caller.
 */
char *primrose_internalHashFile( File *inFile ) {
    
    int bufferSize = 5000;

    int bytesHashed = 0;

    int fileSize = inFile->getLength();

    
    // don't bother checking if file is empty here, since there
    // is a default SHA1 value for empty strings:
    // DA39A3EE5E6B4B0D3255BFEF95601890AFD80709

    
    char *fileName = inFile->getFullFileName();
    
    FILE *fileHandle = fopen( fileName, "rb" );

    delete [] fileName;

    if( fileHandle == NULL ) {
        return NULL;
        }


    SHA_CTX shaContext;

    SHA1_Init( &shaContext );

    
    unsigned char *buffer = new unsigned char[ bufferSize ]; 
    char error = false;

    while( bytesHashed < fileSize && !error ) {
        
        int blockSize = bufferSize;

        if( blockSize + bytesHashed > fileSize ) {
            // partial block
            blockSize = fileSize - bytesHashed;
            }
        
        int numRead = fread( buffer, 1, blockSize,
                             fileHandle );

        if( numRead == blockSize ) {
            SHA1_Update( &shaContext, buffer, blockSize );

            bytesHashed += blockSize;
            }
        else {
            error = true;
            }
        }

    fclose( fileHandle );
    delete [] buffer;
    
    unsigned char *rawDigest = new unsigned char[ SHA1_DIGEST_LENGTH ];

    SHA1_Final( rawDigest, &shaContext );


    if( error ) {
        delete [] rawDigest;
        return NULL;
        }

    // else hash is correct
    char *digestHexString = hexEncode( rawDigest, SHA1_DIGEST_LENGTH );
    
    delete [] rawDigest;

    
    return digestHexString;    
    }



/**
 * Translates a URL-safe encoded path into a file object.
 *
 * @param inEncodedPath the URL-safe path.  Must be destroyed by caller.
 *
 * @return a file object, or NULL if the file does not exist.
 *   Must be destroyed by caller.
 */
File *primrose_internalEncodedPathToFile( char *inEncodedPath ) {
    char *path = URLUtils::hexDecode( inEncodedPath );

    if( strstr( path, ".." ) ) {
        // path may be trying to escape from our file directory
        // block it

        // note:  this may also block file names with "..", like,
        // test..txt
        // but who really names files like this anyway?
        // should really look for /.. or ../, but this might not be as
        // safe (what if, on windows, someone requests \.. directly,
        //       even though this isn't a valid MUTE path)
        
        delete [] path;
        return NULL;
        }
    
    char *pathPointer = path;
    
    if( pathPointer[ strlen( pathPointer ) - 1 ] == '/' ) {
        // path ends with /, remove it
        pathPointer[ strlen( pathPointer ) - 1 ] = '\0';
        }
    if( pathPointer[0] == '/' ) {
        // path starts with /, skip it
        pathPointer = &( pathPointer[1] );
        }
    
    
    char *platformPath;
            
    // transform it into a platform-specific path
    char platformDelim = Path::getDelimeter();

    if( platformDelim == '/' ) {
        // already in platform-specific form
        platformPath = stringDuplicate( pathPointer );
        }
    else {
        char found;

        char *platformDelimString = autoSprintf( "%c", platformDelim );
        
        // replace protocol delimeters with platform-specific delimiters
        platformPath = replaceAll( pathPointer, "/",
                                   platformDelimString, &found );

        delete [] platformDelimString;
        }

    delete [] path;

    
    char *sharingPathString = primroseGetSharingPath();
    
    File *file;

    if( strlen( platformPath ) == 0 ) {
        // root directory
        file = new File( NULL, sharingPathString );
        }
    else {
        Path *sharingPath = new Path( sharingPathString );
        file = new File( sharingPath, platformPath );
        }
    delete [] sharingPathString;
    
    delete [] platformPath;
    
    if( file->exists() ) {
        return file;
        }
    else {
        AppLog::error(
            "fileTransfer",
            "File does not exist" );
        delete file;
        return NULL;
        }
    }



/**
 * Translates a file object into a URL-safe encoded path.
 *
 * Note that this call returns NULL for files that are inside
 * MUTE_hashes and MUTE_incoming directories.
 *
 * @param inFile a file object.  Must be destroyed by caller.
 *
 * @return the URL-safe path, or NULL on error.
 *   Must be destroyed by caller.
 */
char *primrose_internalFileToEncodedPath( File *inFile ) {

    char platformDelim = Path::getDelimeter();

    char *sharingPathString = primroseGetSharingPath();

    char *fullFilePathString = inFile->getFullFileName();

    
    if( strstr( fullFilePathString, primroseHashDirectoryName ) != NULL ||
        strstr( fullFilePathString, primroseIncomingDirectoryName ) != NULL ) {

        // ignore files in these directories

        delete [] sharingPathString;
        delete [] fullFilePathString;

        return NULL;        
        }
        
    
    // remove sharing path from file name
    char *pointerToSharingPath = strstr( fullFilePathString,
                                         sharingPathString );

    char *returnPathString = NULL;
    if( pointerToSharingPath != NULL ) {

        // skip the sharing path
        char *partialFilePath =
            &( fullFilePathString[ strlen( sharingPathString ) ] );


        if( strlen( partialFilePath ) > 0 ) {
            // make sure we didn't miss the final delimiter
            if( partialFilePath[0] == platformDelim ) {

                // skip the first character (delimeter
                partialFilePath = &( partialFilePath[1] );
                }
            }

        if( platformDelim == '/' ) {
            // already using universal path delimeter
            returnPathString = stringDuplicate( partialFilePath );
            }
        else {
            // replace the platform-specific path delimeter with the universal
            // delimeter "/"
            char *platformDelimString = autoSprintf( "%c", platformDelim );
            
            char found;

            returnPathString = replaceAll( partialFilePath,
                                           platformDelimString,
                                           "/",
                                           &found );

            delete [] platformDelimString;
            }
        }

    delete [] sharingPathString;
    delete [] fullFilePathString;

    return returnPathString;
    }



char primroseDoesLocalKeyExist() {

    char *publicKey = SettingsManager::getStringSetting( "localPublicKey" );
    char *privateKey = SettingsManager::getStringSetting( "localPrivateKey" );

    char returnValue = false;

    if( publicKey != NULL && privateKey != NULL ) {
        returnValue = true;
        }

    if( publicKey != NULL ) {
        delete [] publicKey;
        }
    if( privateKey != NULL ) {
        delete [] privateKey;
        }

    return returnValue;
    }



void primroseCreateLocalKeyPair( char *inRandomSeed ) {
    CryptoUtils::seedRandomGenerator( inRandomSeed );

    char *publicKey;
    char *privateKey;
    
    CryptoUtils::generateRSAKey( 368,
                                 &publicKey,
                                 &privateKey );

    SettingsManager::setSetting( "localPublicKey", publicKey );
    SettingsManager::setSetting( "localPrivateKey", privateKey );

    delete [] publicKey;
    delete [] privateKey;
    }


char primroseDoesAccountExist() {

    // account cannot exist without a key
    if( primroseDoesLocalKeyExist() ) {
        
        char valueFound;
        SettingsManager::getIntSetting( "localUserIDPartA", &valueFound );

        if( valueFound ) {
            SettingsManager::getIntSetting( "localUserIDPartB", &valueFound );

            if( valueFound ) {
                return true;
                }
            }
        }

    return false;
    }



int primroseCreateAccount( char *inUserName ) {
    int result;

    char *publicKey = SettingsManager::getStringSetting( "localPublicKey" );

    if( publicKey == NULL ) {
        return 4;
        }
    
    struct UserID *createdID =
        UserManager::createAccount( inUserName, publicKey, &result );

    delete [] publicKey;


    if( result == 1 ) {
        // save the new user ID
        SettingsManager::setSetting( "localUserIDPartA",
                                     (int)( createdID->partA ) );
        SettingsManager::setSetting( "localUserIDPartB",
                                     (int)( createdID->partB ) );

        delete createdID;
        }
    
    return result;
    }



void primroseStart() {

    AppLog::setLog( new FileLog( "Primrose.log" ) );

    AppLog::setLoggingLevel( Log::TRACE_LEVEL );

    
    // read settings
    
    char valueFound = false;

    primroseLocalUserID = new struct UserID;

    primroseLocalUserID->partA = 
        (unsigned long)( SettingsManager::getIntSetting( "localUserIDPartA",
                                                          &valueFound ) );
    if( !valueFound ) {
        primroseLocalUserID->partA = 0;
        }

    primroseLocalUserID->partB = 
        (unsigned long)( SettingsManager::getIntSetting( "localUserIDPartB",
                                                          &valueFound ) );
    if( !valueFound ) {
        primroseLocalUserID->partB = 0;
        }

    primroseLocalPrivateKeyHex =
        SettingsManager::getStringSetting( "localPrivateKey" );
    
    if( primroseLocalPrivateKeyHex == NULL ) {
        // default to an empty key
        primroseLocalPrivateKeyHex = stringDuplicate( "" );
        }


    HostAddress *localHostAddress = HostAddress::getNumericalLocalAddress();
    primroseLocalIPAddress =
        stringDuplicate( localHostAddress->mAddressString );

    delete localHostAddress;

    
    primroseUDPPort =
        (unsigned short)( SettingsManager::getIntSetting( "udpPort",
                                                          &valueFound ) );
    if( !valueFound ) {
        primroseUDPPort = 8000;
        }

    primroseTCPPort =
        (unsigned short)( SettingsManager::getIntSetting( "tcpPort",
                                                          &valueFound ) );
    if( !valueFound ) {
        primroseTCPPort = 9000;
        }

    

    unsigned short lowPort =
        (unsigned short)( SettingsManager::getIntSetting( "portRangeLow",
                                                          &valueFound ) );
    if( !valueFound ) {
        lowPort = 9001;
        }

    unsigned short highPort =
        (unsigned short)( SettingsManager::getIntSetting( "portRangeHigh",
                                                          &valueFound ) );
    if( !valueFound ) {
        highPort = 10000;
        }

    
    primroseLocalSearchTrustLevel =
        (double)( SettingsManager::getFloatSetting( "localSearchTrustLevel",
                                                    &valueFound ) );
    if( !valueFound ) {
        // default to being very cautious
        primroseLocalSearchTrustLevel = 1;
        }

    primroseUploadTrustLevel =
        (double)( SettingsManager::getFloatSetting( "uploadTrustLevel",
                                                    &valueFound ) );
    if( !valueFound ) {
        // default to being very cautious
        primroseUploadTrustLevel = 1;
        }

    primroseFirewallStatus =
        SettingsManager::getStringSetting( "firewallStatus" );
    if( primroseFirewallStatus == NULL ) {
        // default to assuming that we're firewalled
        primroseFirewallStatus = stringDuplicate( "firewalled" );
        }

    // firewalled nodes need to build larger host lists
    if( strcmp( primroseFirewallStatus, "firewalled" ) == 0 ) {
        primroseMinimumAddressCacheSize = 10000;
        }
    else {
        primroseMinimumAddressCacheSize = 1000;
        }
    
    // construct managers
    primroseAddressCache = new AddressCache();
    primrosePortManager = new PortManager( lowPort, highPort );
    primroseMessageManager = new MessageManager( primroseUDPPort );
    primroseResultsHandlerLock = new MutexLock();
    primroseResultsHandlerVector =
        new SimpleVector<SearchResultsHandlerWrapper *>();
    primroseResultsHandlerIDVector =
        new SimpleVector<unsigned long>();

    // add our message handlers
    
    primroseSearchResultsMessageHandlerID =
        primroseMessageManager->addMessageHandler(
            primrose_internalSearchResultsMessageHandler, NULL );

    primroseSearchMessageHandlerID =
        primroseMessageManager->addMessageHandler(
            primrose_internalSearchMessageHandler, NULL );

    primroseConnectionRequestMessageHandlerID =
        primroseMessageManager->addMessageHandler(
            primrose_internalConnectionRequestMessageHandler, NULL );

    primroseHostListRequestMessageHandlerID =
        primroseMessageManager->addMessageHandler(
            primrose_internalHostListRequestMessageHandler, NULL );

    primroseHostListMessageHandlerID =
        primroseMessageManager->addMessageHandler(
            primrose_internalHostListMessageHandler, NULL );

    primroseAddressCatcherMessageHandlerID =
        primroseMessageManager->addMessageHandler(
            primrose_internalAddressCatcherMessageHandler, NULL );

    primroseThreadManager = new FinishedSignalThreadManager();

    primroseServerThread = new ServerThread( primroseTCPPort );


    unsigned long seedThreadSleepTime =
        (unsigned long)(
            SettingsManager::getIntSetting( "secondsBetweenSeedFetches",
                                            &valueFound ) );
    if( !valueFound ) {
        // default to fetching more seeds every 2 minutes
        seedThreadSleepTime = 120;
        }
    
    primroseSeedAddressThread = new SeedAddressThread( seedThreadSleepTime );
    }



void primroseStop() {
    char *loggerName = "primroseStop";
    
    AppLog::info( loggerName, "Destroying SeedAddressThread." );
    delete primroseSeedAddressThread;
    
    AppLog::info( loggerName, "Destroying ServerThread." );
    delete primroseServerThread;

    AppLog::info( loggerName, "Destroying ThreadManager." );    
    delete primroseThreadManager;

    AppLog::info( loggerName, "Removing various message handlers." );    
    primroseMessageManager->removeMessageHandler(
        primroseSearchResultsMessageHandlerID );
    
    primroseMessageManager->removeMessageHandler(
        primroseSearchMessageHandlerID );

    primroseMessageManager->removeMessageHandler(
        primroseConnectionRequestMessageHandlerID );

    primroseMessageManager->removeMessageHandler(
        primroseHostListRequestMessageHandlerID );

    primroseMessageManager->removeMessageHandler(
        primroseHostListMessageHandlerID );
    
    primroseMessageManager->removeMessageHandler(
        primroseAddressCatcherMessageHandlerID );


    AppLog::info( loggerName,
                  "Destroying any left-over search result handlers." );    
    primroseResultsHandlerLock->lock();

    int numHandlers = primroseResultsHandlerVector->size();

    char found = false;
    for( int h=0; h<numHandlers && !found; h++ ) {
        delete *( primroseResultsHandlerVector->getElement( h ) );
        }

    primroseResultsHandlerLock->unlock();

    delete primroseResultsHandlerVector;
    delete primroseResultsHandlerIDVector;
    delete primroseResultsHandlerLock;

    AppLog::info( loggerName, "Destroying AddressCache." );    
    delete primroseAddressCache;

    AppLog::info( loggerName, "Destroying PortManager." );    
    delete primrosePortManager;

    AppLog::info( loggerName, "Destroying MessageManager." );    
    delete primroseMessageManager;
    
    delete [] primroseFirewallStatus;

    delete primroseLocalUserID;
    delete [] primroseLocalPrivateKeyHex;

    delete [] primroseLocalIPAddress;

    AppLog::info( loggerName, "Stop complete." );    
    }



void primroseSetLocalSearchTrustLevel( double inTrustLevel ) {
    primroseLocalSearchTrustLevel = inTrustLevel;

    SettingsManager::setSetting( "localSearchTrustLevel",
                                 (float)inTrustLevel );
    }



double primroseGetLocalSearchTrustLevel() {
    return primroseLocalSearchTrustLevel;
    }



void primroseSetUploadTrustLevel( double inTrustLevel ) {
    primroseUploadTrustLevel = inTrustLevel;

    SettingsManager::setSetting( "uploadTrustLevel",
                                 (float)inTrustLevel );
    }



double primroseGetUploadTrustLevel() {
    return primroseUploadTrustLevel;
    }



char primroseIsSharingPathSet() {
    char *safeSetPath = SettingsManager::getStringSetting( "sharingPath" );

    if( safeSetPath != NULL ) {
        delete [] safeSetPath;
        return true;
        }
    else {
        return false;
        }    
    }



void primroseSetSharingPath( char *inPath ) {
    // encode to protect spaces and special characters
    char *safePath = URLUtils::hexEncode( inPath );
    SettingsManager::setSetting( "sharingPath", safePath );
    delete [] safePath;
    }



char *primroseGetSharingPath() {
    char *safeSetPath = SettingsManager::getStringSetting( "sharingPath" );
    if( safeSetPath != NULL ) {
        char *decodedPath = URLUtils::hexDecode( safeSetPath );
        delete [] safeSetPath;
        
        return decodedPath;
        }
    else {
        return stringDuplicate( "files" );
        }
    }



char primroseIsFirewallStatusSet() {
    char *status = SettingsManager::getStringSetting( "firewallStatus" );

    if( status != NULL ) {
        delete [] status;
        return true;
        }
    else {
        return false;
        }
    }



void primroseSetFirewallStatus( char inFirewalled ) {
    if( inFirewalled ) {
        SettingsManager::setSetting( "firewallStatus", "firewalled" );
        }
    else {
        SettingsManager::setSetting( "firewallStatus", "not_firewalled" );
        }
    }



// handler that we register to select and process search results
void primrose_internalSearchResultsMessageHandler(
    struct UDPAddress *inNodeAddress, struct UserID *inUserID,
    char *inMessageText, void *inExtraArgument ) {

    // outline of function behavior:
    // -- detect a results message
    // -- extract/parse result components
    // -- pass components to each results handler in our list

    // Example message can be seen in Primrose/doc/udpMessageFormats.txt
    
    // tokenize message
    SimpleVector<char *> *tokens = tokenizeString( inMessageText );
    unsigned long numTokens = tokens->size();

    // look at second token to detect a proper results message
    if( numTokens > 4 &&
        strcmp( *( tokens->getElement( 1 ) ), "results" ) == 0 ) {

        char *firewallStatusString = *( tokens->getElement( 0 ) );
        char *searchID = *( tokens->getElement( 2 ) );
        char *tcpPortString = *( tokens->getElement( 3 ) );
        char *numResultsString = *( tokens->getElement( 4 ) );

        char firewalled = false;
        if( strcmp( firewallStatusString, "firewalled" ) == 0 ) {
            firewalled = true;
            }

        // default to 0 if parsing fails
        unsigned short tcpPort = 0;
        sscanf( tcpPortString, "%hu", &tcpPort );

        unsigned long numResults = 0;
        sscanf( numResultsString, "%lu", &numResults );

        if( numTokens < 5 + 3 * numResults ) {
            // limit result count to available tokens
            numResults = ( numTokens - 5 ) / 3;
            }
        
        char **fileNames = new char*[ numResults ];
        char **hashes = new char*[ numResults ];
        unsigned long *sizes = new unsigned long[ numResults ];

        unsigned long i;
        for( i=0; i<numResults; i++ ) {
            int resultStartIndex = 5 + 3 * i;
            
            fileNames[i] =
                URLUtils::hexDecode(
                    *( tokens->getElement( resultStartIndex ) ) );
            
            hashes[i] = *( tokens->getElement( resultStartIndex + 1 ) );

            char *sizeString = *( tokens->getElement( resultStartIndex + 2 ) );

            // default to 0 if parsing fails
            sizes[ i ] = 0;
            sscanf( sizeString, "%lu", &( sizes[i] ) );            
            }

        unsigned short udpPort;
        char *ipAddress = SocketUDP::extractAddress( inNodeAddress,
                                                     &udpPort );
        
        // pass these results to each seach results handler
        primroseResultsHandlerLock->lock();
        int numHandlers = primroseResultsHandlerVector->size();

        for( int h=0; h<numHandlers; h++ ) {
            SearchResultsHandlerWrapper *handlerWrapper =
                *( primroseResultsHandlerVector->getElement( h ) );

            handlerWrapper->mHandlerFunction( searchID, inUserID,
                                              firewalled,
                                              ipAddress,
                                              tcpPort,
                                              udpPort,
                                              numResults,
                                              fileNames, hashes, sizes,
                                              handlerWrapper->mExtraArgument );
            }
        primroseResultsHandlerLock->unlock();

        delete [] ipAddress;
        
        // strings that these arrays point to are tokens that will
        // be destroyed below, except for file names
        for( i=0; i<numResults; i++ ) {
            delete [] fileNames[i];
            }
        delete [] fileNames;
        delete [] hashes;
        delete [] sizes;
        }
    // else ignore message

    // destroy all tokens
    for( unsigned long i=0; i<numTokens; i++ ) {
        delete [] *( tokens->getElement( i ) );
        }
    delete tokens;
    }



struct UserID *primroseGetLocalUserID() {
    return copy( primroseLocalUserID );
    }



char *primroseGetUserName( struct UserID *inID ) {

    return UserManager::getUserName( inID );
    }



struct UserID *primroseGetUserID( char *inUserName ) {
    return UserManager::getUserID( inUserName );
    }



char primroseTrustUser( struct UserID *inID ) {
    return UserManager::setTrustRelationship( primroseLocalUserID,
                                              inID,
                                              UserManager::TRUSTED,
                                              primroseLocalPrivateKeyHex );
    }



char primroseDistrustUser( struct UserID *inID ) {
    return UserManager::setTrustRelationship( primroseLocalUserID,
                                              inID,
                                              UserManager::DISTRUSTED,
                                              primroseLocalPrivateKeyHex );
    }



char primroseNeutralTrustUser( struct UserID *inID ) {
    return UserManager::setTrustRelationship( primroseLocalUserID,
                                              inID,
                                              UserManager::NEUTRAL,
                                              primroseLocalPrivateKeyHex );
    }



double primroseGetUserTrustScore( struct UserID *inID ) {
    return UserManager::getTrustScore( primroseLocalUserID, inID );
    }



int PRIMROSE_TRUSTED = UserManager::TRUSTED;
int PRIMROSE_NEUTRAL = UserManager::NEUTRAL;
int PRIMROSE_DISTRUSTED = UserManager::DISTRUSTED;
int PRIMROSE_FAILED = UserManager::FAILED;


int primroseGetTrustRelationship( struct UserID *inID ) {
    return UserManager::getTrustRelationship( primroseLocalUserID, inID );
    }



struct UserID **primroseGetTrustedUsers( int *outNumTrusted ) {
    return UserManager::getTrustSet( primroseLocalUserID,
                                     UserManager::TRUSTED,
                                     outNumTrusted );
    }



struct UserID **primroseGetDistrustedUsers( int *outNumDistrusted ) {
    return UserManager::getTrustSet( primroseLocalUserID,
                                     UserManager::DISTRUSTED,
                                     outNumDistrusted );
    }



unsigned long primroseAddSearchResultsHandler(
    void (*inHandlerFunction)( char *, struct UserID *, char, char *,
                               unsigned short,
                               unsigned short,
                               unsigned long, char **, char **,
                               unsigned long *, void * ),
    void *inExtraHandlerArgument ) {
    
    // wrap the handler and its argument
    SearchResultsHandlerWrapper *wrapper = new SearchResultsHandlerWrapper();
    wrapper->mHandlerFunction = inHandlerFunction;
    wrapper->mExtraArgument = inExtraHandlerArgument;

    primroseResultsHandlerLock->lock();

    primroseResultsHandlerVector->push_back( wrapper );
    unsigned long id = primroseNextFreeResultsHandlerID;    
    primroseResultsHandlerIDVector->push_back( id );
    primroseNextFreeResultsHandlerID++;

    primroseResultsHandlerLock->unlock();

    return id;
    }



void primroseRemoveSearchResultsHandler( unsigned long inHandlerID ) {
    // search for the ID
    
    primroseResultsHandlerLock->lock();

    int numHandlers = primroseResultsHandlerVector->size();

    char found = false;
    for( int h=0; h<numHandlers && !found; h++ ) {
        if( *( primroseResultsHandlerIDVector->getElement( h ) ) ==
            inHandlerID ) {
            found = true;

            delete *( primroseResultsHandlerVector->getElement( h ) );
            primroseResultsHandlerVector->deleteElement( h );
            primroseResultsHandlerIDVector->deleteElement( h );
            }
        }

    primroseResultsHandlerLock->unlock();
    }



// the handler function registered by each search thread
void primrose_internalSearchThreadMessageHandler(
    struct UDPAddress *inNodeAddress, struct UserID *inUserID,
    char *inMessageText, void *inExtraArgument );



/**
 * Thread that keeps sending out a search request until enough results
 * come back.
 */
class SearchThread : public FinishedSignalThread, public StopSignalThread {

    public:

        /**
         * Constructs and starts a thread.
         *
         * @param inMessage the search message to send.
         *   Will be destroyed by this class.
         * @param inSearchID the ID of this search.
         *   Will be destroyed by this class.
         * @param inDesiredResultCount the number of results to try for.
         *   This thread will stop searching if this many results are received.
         * @param inMaxTimeInSeconds the maximum number of seconds to keep
         *   searching.
         * @param inTrustLevel the minimum trust score of nodes that we
         *   will send the search to, in the range [0,1].
         */
        SearchThread( char *inMessage, char *inSearchID,
                      unsigned long inDesiredResultCount,
                      unsigned long inMaxTimeInSeconds, double inTrustLevel );


        
        // stops and joins this thread
        ~SearchThread();


        
        // implements the Thread interface
        void run();


        
        /**
         * Function called by this thread's registered message handler
         * to pass a message in to this thread.
         *
         * @param inMessageText the text of the received message.
         *   Must be destroyed by caller.
         */
        void messageReceived( char *inMessageText );

        
        
    private:
        char *mMessage;
        char *mSearchID;
        unsigned long mDesiredResultCount;
        unsigned long mMaxTimeInSeconds;
        double mTrustLevel;

        MutexLock *mResultCountLock;
        unsigned long mResultCount;

        unsigned long mStartTime;
        
        /**
         * Gets the result count.
         *
         * Thread safe.
         */
        unsigned long getResultCount();

        
        // the ID of this thread's registered message handler
        int mMessageHandlerID;
        
    };


void primrose_internalSearchThreadMessageHandler(
    struct UDPAddress *inNodeAddress, struct UserID *inUserID,
    char *inMessageText, void *inExtraArgument ) {

    // extra argument is always our SearchThread
    SearchThread *ourThread = (SearchThread *)inExtraArgument;

    // pass the message text into our thread
    ourThread->messageReceived( inMessageText );
    }



SearchThread::SearchThread( char *inMessage, char *inSearchID,
                            unsigned long inDesiredResultCount,
                            unsigned long inMaxTimeInSeconds,
                            double inTrustLevel )
    : mMessage( inMessage ), mSearchID( inSearchID ),
      mDesiredResultCount( inDesiredResultCount ),
      mMaxTimeInSeconds( inMaxTimeInSeconds ), mTrustLevel( inTrustLevel ),
      mResultCountLock( new MutexLock() ), mResultCount( 0 ),
      mStartTime( time( NULL ) ) {

    // register our message handler, passing ourself as the extra argument
    mMessageHandlerID =
        primroseMessageManager->addMessageHandler(
            primrose_internalSearchThreadMessageHandler,
            (void *)this );

    start();
    }



SearchThread::~SearchThread() {
    // unregister our message handler
    primroseMessageManager->removeMessageHandler( mMessageHandlerID );

    // send stop signal
    stop();

    // wait for thread to finish
    join();
    

    delete mResultCountLock;
    delete [] mMessage;
    delete [] mSearchID;    
    }



void SearchThread::run() {
    // search at most 10000 nodes
    int desiredAddresses = 10000;

    // search in batches of 100 nodes at a time
    int batchSize = 100;
    
    int numAddresses;
    AddressAndUser **addresses =
        primroseAddressCache->getAddresses( desiredAddresses, &numAddresses );

    int addressIndex = 0;
    
    // keep going until:
    // --get stop signal or
    // --get enough results or
    // --hit time limit
    // --run out of addresses
    while( ! isStopped() &&
           getResultCount() < mDesiredResultCount &&
           time( NULL ) - mStartTime < mMaxTimeInSeconds &&
           addressIndex < numAddresses ) {

        // send search to next batch of nodes
        for( int i=0; i<batchSize && addressIndex < numAddresses; i++ ) {
            AddressAndUser *address = addresses[ addressIndex ];

            if( UserManager::getTrustScore( primroseLocalUserID,
                                            address->mUserID )
                >= mTrustLevel ) {

                // this user is trusted, send message
                
                primroseMessageManager->sendMessage( mMessage,
                                                     address->mUserID,
                                                     address->mUDPAddress );
                }
            addressIndex++;
            }
            

        // sleep 5 seconds
        StopSignalThread::sleep( 5000 );
        }

    // destroy addresses
    for( int i=0; i<numAddresses; i++ ) {
        delete addresses[i];
        }
    delete [] addresses;
    
                                            
    setFinished();
    }



void SearchThread::messageReceived( char *inMessageText ) {
    // first, check if our search ID is present in the message
    if( strstr( inMessageText, mSearchID ) != NULL ) {

        // tokenize to get result count
        SimpleVector<char *> *tokens = tokenizeString( inMessageText );
        int numTokens = tokens->size();

        if( numTokens > 4 ) {
            // token 4 is the result count
            char *numResultsString = *( tokens->getElement( 4 ) );

            // default to 0
            unsigned long numResults = 0;
            sscanf( numResultsString, "%lu", &numResults );

            mResultCountLock->lock();
            mResultCount += numResults;
            mResultCountLock->unlock();
            }

        // destroy all tokens
        for( int i=0; i<numTokens; i++ ) {
            delete [] *( tokens->getElement( i ) );
            }
        delete tokens;
        }
    }



unsigned long SearchThread::getResultCount() {
    mResultCountLock->lock();
    unsigned long returnValue = mResultCount;
    mResultCountLock->unlock();

    return returnValue;
    }



void primroseSearch( char *inSearchID, char *inSearchTerms,
                     unsigned long inDesiredResultCount,
                     unsigned long inMaxTimeInSeconds,
                     double inTrustLevel ) {
    // first, compose the message
    // "firewalled search 384772884 song mp3 indie rock"

    char *message = autoSprintf( "%s search %s %s",
                                 primroseFirewallStatus,
                                 inSearchID, inSearchTerms );

    SearchThread *thread =
        new SearchThread( message, stringDuplicate( inSearchID ),
                          inDesiredResultCount,
                          inMaxTimeInSeconds, inTrustLevel );
    
    primroseThreadManager->addThread( thread );
    }



void primroseGetFile( unsigned long inNumHosts,
                      struct UserID **inUserIDs,
                      char **inIPAddresses, 
                      unsigned short *inTCPPorts,
                      unsigned short *inUDPPorts, 
                      char *inFirewallStatuses,
                      char *inFileName,
                      char *inFileHash,
                      unsigned long inFileSize,
                      char (*inDownloadProgressHandler)( int, unsigned long,
                                                         void * ),
                      void *inExtraHandlerArgument ) {

    DownloadThread *thread = new DownloadThread( inNumHosts,
                                                 inUserIDs,
                                                 inIPAddresses,
                                                 inTCPPorts,
                                                 inUDPPorts,
                                                 inFirewallStatuses,
                                                 inFileName,
                                                 inFileHash,
                                                 inFileSize,
                                                 inDownloadProgressHandler,
                                                 inExtraHandlerArgument );

    
    primroseThreadManager->addThread( thread );
    }



void primrose_internalSearchMessageHandler(
    struct UDPAddress *inNodeAddress, struct UserID *inUserID,
    char *inMessageText, void *inExtraArgument ) {

    // only allow trusted users to search
    if( UserManager::getTrustScore( primroseLocalUserID, inUserID ) >=
        primroseGetLocalSearchTrustLevel() ) {


        // **
        // most of this code copied from MUTE file sharing
        // **

        
        // is the message a SearchRequest?
        // look at second token
        SimpleVector<char *> *tokens = tokenizeString( inMessageText );
        int numTokens = tokens->size();

        if( numTokens > 3 ) {
            char *typeToken = *( tokens->getElement( 1 ) );
            if( strcmp( "search", typeToken ) == 0 ) {
            
                char *searchID = *( tokens->getElement( 2 ) );

                // use the first search term as the search string
                // in case of a hash-only search
                char *searchString = *( tokens->getElement( 3 ) );

                char *logMessage = autoSprintf(
                    "Got search request for (%s), id=%s",
                    searchString, searchID );
                AppLog::detail( "fileShare",
                                logMessage );
                delete [] logMessage;
            
            
                // check if this is a hash-only search
                char hashOnlySearch = false;

                const char *hashStart = "hash_";
            
                char *pointerToHashStart = strstr( searchString,
                                                   hashStart );
                if( pointerToHashStart != NULL ) {
                    // search contains a hash-only search.
                    // extract hash, and ignore any other search terms

                    char *pointerToHash =
                        &( pointerToHashStart[ strlen( hashStart ) ] );

                    // hash should be at most 40 characters long
                    char *hashString = new char[ 41 ];

                    int numRead = sscanf( pointerToHash, "%40s", hashString );

                    if( numRead == 1 ) {
                        delete [] searchString;
                        searchString = hashString;

                        hashOnlySearch = true;
                        }
                    else {
                        delete [] hashString;
                        }
                    }

                char *resultHeader =
                    autoSprintf( "%s results %s %hu ",
                                 primroseFirewallStatus,
                                 searchID, primroseTCPPort );
                                 
            
                SimpleVector<char*> *searchTerms = new SimpleVector<char *>();
                int numSearchTerms = tokens->size() - 3;

                for( int i=3; i<tokens->size(); i++ ) {
                    searchTerms->push_back(
                        stringDuplicate( *( tokens->getElement( i ) ) ) );
                    }
            
                char *sharedPath = primroseGetSharingPath();
            
                File *sharedDirectory = new File( NULL, sharedPath );
                delete [] sharedPath;

            
                // ignore searches that contain no terms
                // only search if we have a valid shared directory
                if( numSearchTerms > 0 &&
                    sharedDirectory->exists() &&
                    sharedDirectory->isDirectory() ) {

                    // make sure hash dir exists
                    File *hashDirectory =
                        sharedDirectory->getChildFile(
                            primroseHashDirectoryName ); 

                    if( ! hashDirectory->exists() ) {
                        hashDirectory->makeDirectory();
                        }

                    // look at child files recursively

                    char found;
                    int maxDepth =
                        SettingsManager::getIntSetting( "maxSubfolderDepth",
                                                        &found );
                    if( !found || maxDepth < 0 ) {
                        // default max depth of 10
                        maxDepth = 10;
                        }
                
                    int numChildren;
                    File **childFiles =
                        sharedDirectory->getChildFilesRecursive(
                            maxDepth,
                            &numChildren );

                    if( childFiles != NULL ) {

                        SimpleVector<char *> *hits =
                            new SimpleVector<char *>();

                        // for now, trim our results so that they fit
                        // in one message

                        // messages are 512 B
                        // -- 10 bytes used by result count number
                        // -- 63 bytes used by signature block
                        // -- variable number of bytes used by results header
                        int maxResultsLength =
                            512 - 10 - 63 - strlen( resultHeader );
                        int totalResultLength = 0;

                        char hitLimit = false;

                        int i;
                        for( i=0; i<numChildren && !hitLimit; i++ ) {
                        
                            if( ! childFiles[i]->isDirectory() ) {

                                char *fileName =
                                    primrose_internalFileToEncodedPath(
                                        childFiles[i] );
                            
                                if( fileName != NULL ) {
                                                        
                                    char hitAll = true;
                                
                                    if( ! hashOnlySearch ) {
                                        // check each term
                                        for( int j=0;
                                             j<numSearchTerms && hitAll;
                                             j++ ) {

                                            char *term =
                                             *( searchTerms->getElement( j ) );

                                            if( stringLocateIgnoreCase(
                                                fileName,
                                                term ) == NULL ) {
                                            
                                                // missed this term
                                                hitAll = false;
                                                }
                                            }
                                        }
                                
                                    if( hitAll ) {

                                        // check for a cached hash

                                        // store hash of file contents using
                                        // hash of file name as the file name
                                        // we need to do this because file
                                        // names are now paths into subdirs

                                        // for example, if our file is
                                        // test/music/song.mp3
                                        // we cannot store the hash in
                                        // a file called
                                        // "Primrose_test/music/song.mp3"
                                        // since this is not a valid file name
                                    
                                        char *hashFileName =
                                            computeSHA1Digest( fileName );
                                        
                                        File *hashFile =
                                            hashDirectory->getChildFile(
                                                hashFileName );
                                    
                                        delete [] hashFileName;
                                    
                                    
                                        long hashModTime =
                                            hashFile->getModificationTime();
                                        long fileModTime =
                                            childFiles[i]->
                                               getModificationTime();
                                    
                                        char *hashString = NULL;
                                        if( hashFile->exists() &&
                                            hashModTime >= fileModTime ) {
                                            // cached hash is up-to-date
                                        
                                            hashString =
                                                hashFile->readFileContents();
                                            }
                                        else {
                                            // generate a new hash
                                            hashString =
                                                primrose_internalHashFile(
                                                    childFiles[i] );
                                        
                                            if( hashString != NULL ) {
                                                // cache it
                                                hashFile->writeToFile(
                                                    hashString );
                                                }

                                            // save a map from the hash
                                            // to a relative file path
                                            char *hashPathMapFileName =
                                                autoSprintf( "%s.path",
                                                             hashString );

                                            File *hashPathMapFile =
                                                hashDirectory->getChildFile(
                                                    hashPathMapFileName );

                                            hashPathMapFile->writeToFile(
                                                fileName );

                                            delete [] hashPathMapFileName;
                                            delete hashPathMapFile;
                                            
                                            }
                                        delete hashFile;
                                    
                                        if( hashString == NULL ) {
                                            hashString = stringDuplicate( "" );
                                            }


                                        // check hash if this is a hash-only
                                        // search
                                        char resultIsAHit = true;
                                    
                                        if( hashOnlySearch ) {
                                            char *hashTerm =
                                             *( searchTerms->getElement( 0 ) );

                                            if( stringCompareIgnoreCase(
                                                hashString,
                                                hashTerm ) != 0 ) {
                                            
                                                resultIsAHit = false;
                                                }
                                            }

                                
                                        if( resultIsAHit ) {
                                    
                                            char *encodedFileName =
                                               URLUtils::hexEncode( fileName );
                                
                                            char *resultString = autoSprintf(
                                                "%s %s %d",
                                                encodedFileName,
                                                hashString,
                                                childFiles[i]->getLength() );
                                            delete [] encodedFileName;
                                        
                                            int resultLength = strlen(
                                                resultString );
                                        
                                            if( totalResultLength +
                                                resultLength + 1 <
                                                maxResultsLength ) {

                                                // not at limit yet
                                                hits->push_back(
                                                    resultString );

                                                totalResultLength +=
                                                    resultLength + 1;
                                                }
                                            else {
                                                // hit limit
                                                hitLimit = true;
                                                delete [] resultString;
                                                }
                                            }
                                        delete [] hashString;
                                        }
                                    delete [] fileName;
                                    }
                                }
                            }

                        // destroy child files (outside previous loop so
                        // that we can jump out of previous loop using
                        // !hitLimit)
                        for( i=0; i<numChildren; i++ ) {
                            delete childFiles[i];
                            }
                        delete [] childFiles;

                    
                        int numHits = hits->size();

                        if( numHits != 0 ) {

                            logMessage = autoSprintf(
                                "Sending response with %d results, id=%s",
                                numHits, searchID );
                            AppLog::detail( "fileShare",
                                            logMessage );
                            delete [] logMessage;

                        
                            char **hitArray =
                                hits->getElementArray();
                        
                            // entries delimited by newlines
                            char *hitString = join( hitArray,
                                                    numHits,
                                                    "\n" );
                            for( int i=0; i<numHits; i++ ) {
                                delete [] hitArray[i];
                                }
                            delete [] hitArray;


                            char *message = autoSprintf(
                                "%s"
                                "%d\n"
                                "%s",
                                resultHeader,
                                numHits,
                                hitString );
                            delete [] hitString;

                            primroseMessageManager->sendMessage(
                                message,
                                inUserID,
                                inNodeAddress ); 
                        
                            delete [] message;
                            }
                        
                        delete hits;
                        }
                
                    delete hashDirectory;
                    }

                delete [] resultHeader;
                
                
                delete sharedDirectory;
                
                for( int j=0; j<numSearchTerms; j++ ) {
                    char *term =
                        *( searchTerms->getElement( j ) );
                    delete [] term;
                    }
                delete searchTerms;
                }
            }
            
        for( int i=0; i<numTokens; i++ ) {
            delete [] *( tokens->getElement( i ) );
            }
    
        delete tokens;
        
        }
    }


        
void primrose_internalConnectionRequestMessageHandler(
    struct UDPAddress *inNodeAddress, struct UserID *inUserID,
    char *inMessageText, void *inExtraArgument ) {

    // check trust level?
    
    // respond by making a connection on the requested port

    SimpleVector<char *> *tokens = tokenizeString( inMessageText );
    int numTokens = tokens->size();

    if( numTokens == 3 ) {
        if( strcmp( *( tokens->getElement( 1 ) ), "connectToMe" ) == 0 ) {
            // a connection request

            unsigned short tcpPort;

            int numRead = sscanf( *( tokens->getElement( 1 )  ),
                                  "%hu", &tcpPort );

            if( numRead == 1 ) {

                // connect on the tcp port
                unsigned short udpPort;
                char *ipAddress = SocketUDP::extractAddress( inNodeAddress,
                                                             &udpPort );

                
                Socket *sock =
                    primrose_internalGetConnection(
                        ipAddress,
                        udpPort,
                        tcpPort,
                        false,  // cannot be firewalled
                        inUserID );

                delete [] ipAddress;

                if( sock != NULL ) {
                    // attach a thread to this socket
                    ConnectionThread *thread = new ConnectionThread( sock );

                    primroseThreadManager->addThread( thread );
                    }
                }            
            }
        }

    for( int i=0; i<numTokens; i++ ) {
        delete [] *( tokens->getElement( i ) );
        }
    
    delete tokens;
    }



void primrose_internalHostListRequestMessageHandler(
    struct UDPAddress *inNodeAddress, struct UserID *inUserID,
    char *inMessageText, void *inExtraArgument ) {


    SimpleVector<char *> *tokens = tokenizeString( inMessageText );
    int numTokens = tokens->size();

    if( numTokens == 2 ) {

        // check if it is a Host List Request
        if( strcmp( "getHostList",
                    *( tokens->getElement( 1 ) ) ) == 0 ) {

            // send back at most 20 recent IP/port pairs,
            // as many as will fit in a message
            int numAddresses;
            AddressAndUser **addresses =
                primroseAddressCache->getAddresses( 20,
                                                    &numAddresses );

            // keep adding to the message until we are over the size limit
            char *message = autoSprintf( "%s hostList",
                                         primroseFirewallStatus );

            // messages are 512 B
            // -- 63 bytes used by signature block
            int maxMessageLength =
                512 - 63;

            char full = false;

            int i;
            for( i=0; i<numAddresses && !full; i++ ) {
                struct UserID *id = addresses[i]->mUserID;
                struct UDPAddress *udpAddress = addresses[i]->mUDPAddress;

                unsigned short udpPort;
                char *ipAddress =
                    SocketUDP::extractAddress( udpAddress,
                                               &udpPort );
                
                char *nextLine = autoSprintf( "\n%lu %lu %s %hu",
                                              id->partA, id->partB,
                                              ipAddress, udpPort );

                delete [] ipAddress;

                int lineLength = strlen( nextLine );

                int currentMessageLength = strlen( message );

                if( lineLength + currentMessageLength <= maxMessageLength ) {
                    // add the line
                    char *newMessage = concatonate( message, nextLine );
                    delete [] message;
                    message = newMessage;
                    }
                else {
                    full = true;
                    }

                delete [] nextLine;                
                }

            
            for( i=0; i<numAddresses; i++ ) {
                delete addresses[i];
                }
            delete [] addresses;


            // send back the results
            primroseMessageManager->sendMessage(
                message,
                inUserID,
                inNodeAddress );

            delete [] message;
            }
        }

    for( int i=0; i<numTokens; i++ ) {
        delete [] *( tokens->getElement( i ) );
        }
    
    delete tokens;
    }



void primrose_internalHostListMessageHandler(
    struct UDPAddress *inNodeAddress, struct UserID *inUserID,
    char *inMessageText, void *inExtraArgument ) {

    

    SimpleVector<char *> *tokens = tokenizeString( inMessageText );
    int numTokens = tokens->size();

    if( numTokens >= 2 ) {

        // check if it is a Host List
        if( strcmp( "hostList",
                    *( tokens->getElement( 1 ) ) ) == 0 ) {

            // for each quadruplet
            for( int i=2; i<numTokens - 3; i+=4 ) {
                unsigned long partA = 0;
                unsigned long partB = 0;

                char *ipAddress = new char[100];
                unsigned short port = 0;

                sscanf( *( tokens->getElement( i ) ), "%lu", &partA );
                sscanf( *( tokens->getElement( i + 1 ) ), "%lu", &partB );
                sscanf( *( tokens->getElement( i + 2 ) ), "%99s", ipAddress );
                sscanf( *( tokens->getElement( i + 3 ) ), "%hu", &port );

                struct UDPAddress *address =
                    SocketUDP::makeAddress( ipAddress, port );
                delete [] ipAddress;
                
                struct UserID *id = new struct UserID;
                id->partA = partA;
                id->partB = partB;

                if( ! compare( id, primroseLocalUserID ) ) {
                    // do not add ourself
                    primroseAddressCache->addAddress( address, id );
                    }
                delete address;
                delete id;
                }
            }
        }

    for( int i=0; i<numTokens; i++ ) {
        delete [] *( tokens->getElement( i ) );
        }
    
    delete tokens;
    }



void primrose_internalAddressCatcherMessageHandler(
    struct UDPAddress *inNodeAddress, struct UserID *inUserID,
    char *inMessageText, void *inExtraArgument ) {
    
    primroseAddressCache->addAddress( inNodeAddress, inUserID );

    if( primroseAddressCache->getCacheSize() <
        primroseMinimumAddressCacheSize ) {

        // send this host a host list request message

        // only send if this is NOT a host list or host list request message
        // to avoid message loops
        char loopDetected = false;
        
        SimpleVector<char *> *tokens = tokenizeString( inMessageText );
        int numTokens = tokens->size();

        if( numTokens >= 2 ) {
            if( strcmp( *( tokens->getElement( 1 ) ), "hostList" ) == 0 ||
                strcmp( *( tokens->getElement( 1 ) ), "getHostList" ) == 0 ) {

                loopDetected = true;
                }
            }

        for( int i=0; i<numTokens; i++ ) {
            delete [] *( tokens->getElement( i ) );
            }
        delete tokens;
        
        
        if( !loopDetected ) {
            char *message = autoSprintf( "%s getHostList",
                                         primroseFirewallStatus );

            primroseMessageManager->sendMessage(
                message,
                inUserID,
                inNodeAddress );

            delete [] message;
            }
        }
    }



Socket *primrose_internalGetConnection( char *inIPAddress,
                                        unsigned short inTCPPort,
                                        unsigned short inUDPPort,
                                        char inFirewallStatus,
                                        struct UserID *inUserID ) {

    if( inFirewallStatus ) {
        // firewalled, send a connection request by UDP

        unsigned short port = primrosePortManager->getPort();
        char *message = autoSprintf( "%s connectToMe %hu",
                                     primroseFirewallStatus,
                                     port );

        struct UDPAddress *udpAddress = SocketUDP::makeAddress( inIPAddress,
                                                                inUDPPort );
        
        primroseMessageManager->sendMessage(
                                message,
                                inUserID,
                                udpAddress );

        delete udpAddress;


        SocketServer *tempServer =
            new SocketServer( port, 1 );

        // wait 30 seconds for a connection
        char timedOut = false;
		Socket *sock = tempServer->acceptConnection( 30000, &timedOut );

        delete tempServer;
        primrosePortManager->freePort( port );

        
        if( timedOut ) {
            return NULL;
            }
        else {
            return sock;
            }
        }
    else {
        // connect directly with a 30-second timeout
        HostAddress *tcpAddress =
            new HostAddress( stringDuplicate( inIPAddress ), inTCPPort );

        char timedOut = false;
        Socket *sock = SocketClient::connectToServer( tcpAddress,
                                                      30000, &timedOut );

        delete tcpAddress;
        
        if( timedOut ) {
            return NULL;
            }
        else {
            return sock;
            }                                                      
        }

    }



/**
 * Maps a file hash to a share-directory-relative file path.
 *
 * @param inHashString the SHA1 hash of the file as ASCII hex,
 *   or NULL if mapping fails.
 *   Must be destroyed by caller.
 */
char *primrose_internalMapHashToFilePath( char *inHashString ) {
    char *sharedPath = primroseGetSharingPath();
            
    File *sharedDirectory = new File( NULL, sharedPath );
    delete [] sharedPath;

    char *mappedFilePath = NULL;
            
    if( sharedDirectory->exists() &&
        sharedDirectory->isDirectory() ) {

        // make sure hash dir exists
        File *hashDirectory =
            sharedDirectory->getChildFile(
                primroseHashDirectoryName ); 

        if( hashDirectory->exists() ) {

            char *hashPathMapFileName =
                autoSprintf( "%s.path",
                             inHashString );
            
            File *hashPathMapFile =
                hashDirectory->getChildFile(
                    hashPathMapFileName );

            if( hashPathMapFile->exists() ) {
                mappedFilePath = hashPathMapFile->readFileContents();
                }
            
            delete [] hashPathMapFileName;
            delete hashPathMapFile;
            }
        
        delete hashDirectory;
        }
    delete sharedDirectory;

    return mappedFilePath;
    }



void ServerThread::run() {
    
    SocketServer *server = new SocketServer( mPort, 100 );

    char *loggerName = "primroseServerThread";
    
    
    char timedOut = true;

    // 5 seconds
    long timeout = 5000;
    
    Socket *received = NULL;

    
    AppLog::detail( loggerName, "Waiting for connection." );
    while( timedOut && !isStopped() ) {
        received = server->acceptConnection( timeout, &timedOut );
        }

    
    while( !isStopped() ) {

        if( received != NULL ) {
            AppLog::info( loggerName, "Connection received." );

            // attach a thread to this socket
            ConnectionThread *thread = new ConnectionThread( received );
            
            primroseThreadManager->addThread( thread );

            received = NULL;
            }
        else {
            AppLog::error( loggerName,
                           "Failed to accept a connection." );
            }

        
        AppLog::detail( loggerName, "Waiting for connection." );
        timedOut = true;
        while( timedOut && !isStopped() ) {
            received = server->acceptConnection( timeout, &timedOut );
            }
        }

    if( isStopped() ) {
        AppLog::info( loggerName, "Got stop signal." );
        }

    if( received != NULL ) {
        delete received;
        }
    
    delete server;

    AppLog::detail( loggerName, "run function returning." );
    }



void ConnectionThread::run() {

    // "not_firewalled getChunk FB322F86DB95BE0670D06631EE27E83C5B64679A 15"

    SimpleVector<unsigned char> *receivedBytes =
        new SimpleVector<unsigned char>();

    char error = false;
    char seenOneTerminator = false;
    char seenTwoTerminators = false;

    unsigned char *buffer = new unsigned char[1];
    
    while( !isStopped() &&
           !error &&
           !seenTwoTerminators ) {

        // receive with a 10 second timeout
        int numRead = mSocket->receive( buffer, 1,
                                        10000 );

        if( numRead != 1 ) {
            error = true;
            }
        else {

            receivedBytes->push_back( buffer[0] );

            if( buffer[0] == '\0' ) {
                if( !seenOneTerminator ) {
                    seenOneTerminator = true;
                    }
                else {
                    seenTwoTerminators = true;
                    }
                }
            }
        }

    delete [] buffer;
    
    if( isStopped() || error ) {
        // do nothing, just return
        delete receivedBytes;
        return;
        }


    unsigned char *receivedArray = receivedBytes->getElementArray();

    delete receivedBytes;

    // received array should be ASCII with two terminators

    // construct a pointer to each string
    
    char *firstString = (char *)receivedArray;

    // skip first string
    char *secondString =
        (char *)( &( receivedArray[ strlen( firstString ) + 1 ] ) );

    SimpleVector<char *> *firstTokens = tokenizeString( firstString );
    SimpleVector<char *> *secondTokens = tokenizeString( secondString );

    int numFirstTokens = firstTokens->size();
    int numSecondTokens = secondTokens->size();

    if( numFirstTokens == 4 &&
        strcmp( "getChunk", *( firstTokens->getElement( 1 ) ) ) == 0 &&
        numSecondTokens == 5 ) {

        // a chunk request message with a proper signature block

        struct UserID *senderID =
            primrose_internalCheckSignatureAndGetSenderID(
                firstString,
                NULL, 0,  // no binary block in a chunk request
                secondString );

        if( senderID != NULL &&
            UserManager::getTrustScore( primroseLocalUserID, senderID )
            >= primroseUploadTrustLevel ) {
            // upload permitted

            // token 2 is hash
            char *fileHash = *( firstTokens->getElement( 2 ) );

            // token 3 is the chunk number

            unsigned long chunkNumber = 0;

            sscanf( *( firstTokens->getElement( 3 ) ),
                    "%lu", &chunkNumber );

            
            char *filePath = primrose_internalMapHashToFilePath( fileHash );

            if( filePath != NULL ) {

                char *sharedPath = primroseGetSharingPath();
            
                File *sharedDirectory = new File( NULL, sharedPath );
                delete [] sharedPath;
                
                File *file = sharedDirectory->getChildFile( filePath );

                delete sharedDirectory;
                
                unsigned long numChunks = 0;
                unsigned long fileSize = 0;
                char *fullFilePath = NULL;
                
                if( file != NULL && file->exists() ) {
                    fileSize = file->getLength();

                    numChunks = fileSize / primroseChunkSize;

                    if( fileSize % primroseChunkSize != 0 ) {
                        // extra partial chunk
                        numChunks += 1;
                        }

                    fullFilePath = file->getFullFileName();
                    }
                delete file;

                
                if( fullFilePath != NULL &&
                    chunkNumber < numChunks ) {
                            
                    unsigned long offset = chunkNumber * primroseChunkSize;

                    unsigned long thisChunkSize = primroseChunkSize;

                    if( fileSize - offset < primroseChunkSize ) {
                        // partial final chunk
                        thisChunkSize = fileSize - offset;
                        }

                    FILE *fileHandle = fopen( fullFilePath, "rb" );

                    fseek( fileHandle, offset, SEEK_SET );
                            
                    unsigned char *chunkData =
                        new unsigned char[ thisChunkSize ];

                    fread( chunkData,
                           1, thisChunkSize, fileHandle );
                    
                    fclose( fileHandle );

                    
                    
                    char *textHeader = autoSprintf(
                        "%s chunk %s %lu %lu",
                        primroseFirewallStatus, fileHash,
                        chunkNumber, thisChunkSize );
                    
                    char *signatureBlock = 
                        primrose_internalGetTextSignatureBlock(
                            textHeader,
                            chunkData, thisChunkSize,
                            senderID );   // receiver ID is sender of request
                    
                    if( signatureBlock != NULL ) {
                        // now we have all message components

                        // write them on the socket

                        // include the \0 terminator
                        mSocket->send( (unsigned char *)textHeader,
                                       strlen( textHeader ) + 1 );
                    
                        mSocket->send( chunkData, thisChunkSize );

                        // include the \0 terminator
                        mSocket->send( (unsigned char *)signatureBlock,
                                       strlen( signatureBlock ) + 1 );

                        delete [] signatureBlock;

                        // flush the socket
                        mSocket->sendFlushBeforeClose( 5000 );
                        }

                    
                    delete [] textHeader;

                    delete [] chunkData;
                    }

                if( fullFilePath != NULL ) {
                    delete [] fullFilePath;
                    fullFilePath = NULL;
                    }
                
                delete [] filePath;
                }
            
            
            }
        
        delete senderID;
        }
        

    int i;

    for( i=0; i<numFirstTokens; i++ ) {
        delete [] *( firstTokens->getElement( i ) );
        }
    delete firstTokens;

    for( i=0; i<numSecondTokens; i++ ) {
        delete [] *( secondTokens->getElement( i ) );
        }
    delete secondTokens;
    
    
    delete [] receivedArray;    
    }



void SeedAddressThread::run() {

    while( !isStopped() ) {

        if( primroseAddressCache->getCacheSize() <
            primroseMinimumAddressCacheSize ) {

            char *serverURL =
                SettingsManager::getStringSetting( "trustServerURL" );

            if( serverURL == NULL ) {
                // stop thread... we have no server URL
                AppLog::error(
                    "SeedAddressThread",
                    "Trust server URL not set." );
                return;
                }

            
            if( strcmp( primroseFirewallStatus, "not_firewalled" ) == 0 ) {
                // post our address

                char *stringToSign = autoSprintf( "%s_%hu",
                                                  primroseLocalIPAddress,
                                                  primroseUDPPort );

                char *signature =
                    CryptoUtils::rsaSign( primroseLocalPrivateKeyHex,
                                          (unsigned char *)stringToSign,
                                          strlen( stringToSign ) );
                delete [] stringToSign;
    
                if( signature != NULL ) {
                    
                    char *urlWithData = autoSprintf(
                        "%s?action=postAddress&userIDPartA=%lu&"
                        "userIDPartB=%lu&ipAddress=%s&port=%hu&"
                        "signature=%s",
                        serverURL, primroseLocalUserID->partA,
                        primroseLocalUserID->partB,
                        primroseLocalIPAddress, primroseUDPPort,
                        signature );
                    
                    delete [] signature;
                    
    
                    int resultLength;
                    char *result = WebClient::getWebPage( urlWithData,
                                                          &resultLength );

                    delete [] urlWithData;
    
                    if( result != NULL ) {
                        // ignore result... either it is OK or FAILED
                        delete [] result;
                        }
                    }
                }

            // fetch seed addresses
            char *urlWithData = autoSprintf( "%s?action=getHostList",
                                             serverURL );
            delete [] serverURL;

            int resultLength;
            char *result = WebClient::getWebPage( urlWithData,
                                                  &resultLength );

            delete [] urlWithData;

            if( result != NULL ) {
                SimpleVector<char*> *resultTokens = tokenizeString( result );
                delete [] result;

                for( int i=0; i<resultTokens->size(); i++ ) {
                    char *token = *( resultTokens->getElement( i ) );
                    
                    int numParts;
                    char **parts = split( token, "_", &numParts );

                    if( numParts == 4 ) {
                        unsigned long partA = 0;
                        unsigned long partB = 0;

                        unsigned short port = 8000;

                        sscanf( parts[0], "%lu", &partA );
                        sscanf( parts[1], "%lu", &partB );
                        sscanf( parts[3], "%hu", &port );

                        struct UserID *id = new struct UserID;
                        id->partA = partA;
                        id->partB = partB;

                        struct UDPAddress *address =
                            SocketUDP::makeAddress( parts[2], port );

                        if( ! compare( id, primroseLocalUserID ) ) {
                            // do not add ourself to the cache
                            primroseAddressCache->addAddress( address, id );
                            }
                        delete address;
                        delete id;                        
                        }

                    for( int j=0; j<numParts; j++ ) {
                        delete [] parts[j];
                        }
                    delete [] parts;
                    
                    delete [] token;
                    }

                delete resultTokens;                
                }
            
            }
        
        // sleep in 5 second chunks to catch stop signal
        unsigned long netSleep = 0;
        unsigned long goalSleep = 1000 * mSleepTime;
        while( ! isStopped() && netSleep < goalSleep ) {

            unsigned long sleepTime = 5000;
            
            if( goalSleep - netSleep < sleepTime ) {
                sleepTime = goalSleep - netSleep;
                }
            sleep( sleepTime );
            netSleep += sleepTime;
            }
        }
    
    }



/**
 * Checks the signature of a message and gets the trust score of the sender.
 *
 * @param inTextHeader the ASCII header portion of the message.
 * @param inBinaryBlock the optional binary block in the message, or NULL
 *   if the message has no binary block.
 * @param inBinaryBlockLength the size of the binary block.  Must be 0
 *   if inBinaryBlock is NULL.
 * @param inSenderID the sender ID.
 * @param inReceiverID the receiver ID.
 * @param inSignatureHex the ASCII hex of the RSA signature.
 *
 * @return true if the signature is valid, or false otherwise.
 */
char primrose_internalCheckSignature(
    char *inTextHeader,
    unsigned char *inBinaryBlock, unsigned long inBinaryBlockLength,
    struct UserID *inSenderID, struct UserID *inReceiverID,
    char *inSignatureHex ) {
    

    char *senderKey = UserManager::getPublicKey( inSenderID );

    int headerLength = strlen( inTextHeader );
    
    int dataLength =
        headerLength +
        inBinaryBlockLength +
        8;  // 8 bytes for the binary receiver ID

    unsigned char *fullDataBlock =
        new unsigned char[ dataLength ];
    
    // fill data block
    memcpy( fullDataBlock, inTextHeader, headerLength );

    if( inBinaryBlock != NULL ) {
        memcpy( &( fullDataBlock[ headerLength ] ), 
                inBinaryBlock, inBinaryBlockLength );
        }

    unsigned char *binaryReceiverID =
        primrose_internalUserIDToBinary( inReceiverID );

    memcpy( &( fullDataBlock[ headerLength + inBinaryBlockLength ] ),
            binaryReceiverID, 8 );

    delete [] binaryReceiverID;
    
    char signatureCorrect =
        CryptoUtils::rsaVerify( senderKey,
                                fullDataBlock, dataLength,
                                inSignatureHex );

    delete [] fullDataBlock;
    delete [] senderKey;
    
    if( signatureCorrect ) {
        return true;
        }
    else {
        return false;
        }    
    }



struct UserID *primrose_internalCheckSignatureAndGetSenderID(
    char *inTextHeader,
    unsigned char *inBinaryBlock, unsigned long inBinaryBlockLength,
    unsigned char *inSignatureBlock, unsigned long inSignatureBlockLength ) {

    if( inSignatureBlockLength < 16 ) {
        // not enough bytes in signature
        return NULL;
        }
    
    // first 8 bytes are sender ID
    struct UserID *senderID =
        primrose_internalBinaryToUserID( inSignatureBlock );

    // next 8 bytes are receiver ID
    struct UserID *receiverID =
        primrose_internalBinaryToUserID( &( inSignatureBlock[8] ) );

    // remainder is signature hex
    char *signatureHex = hexEncode( &( inSignatureBlock[16] ),
                                    inSignatureBlockLength - 16 );

    char signatureValid =
        primrose_internalCheckSignature(
            inTextHeader,
            inBinaryBlock, inBinaryBlockLength,
            senderID, receiverID, signatureHex );

    delete receiverID;
    delete [] signatureHex;

    if( signatureValid ) {
        return senderID;
        }
    else {
        delete senderID;
    
        return NULL;
        }
    }



struct UserID *primrose_internalCheckSignatureAndGetSenderID(
    char *inTextHeader,
    unsigned char *inBinaryBlock, unsigned long inBinaryBlockLength,
    char *inSignatureTextBlock ) {

    SimpleVector<char *> *signatureTokens =
        tokenizeString( inSignatureTextBlock );

    int numTokens = signatureTokens->size();

    
    struct UserID *returnValue = NULL;

    
    if( numTokens == 5 ) {
        // first two are partA and partB of sender ID
        struct UserID *senderID =
            primrose_internalStringsToUserID(
                *( signatureTokens->getElement( 0 ) ),
                *( signatureTokens->getElement( 1 ) ) );

        if( senderID != NULL ) {
            // next two are partA and partB of receiver ID
            struct UserID *receiverID =
                primrose_internalStringsToUserID(
                    *( signatureTokens->getElement( 2 ) ),
                    *( signatureTokens->getElement( 3 ) ) );

            if( receiverID != NULL ) {
                // last token is signature hex
                char *signatureHex = *( signatureTokens->getElement( 4 ) );
                
                char signatureValid =
                    primrose_internalCheckSignature(
                        inTextHeader,
                        inBinaryBlock, inBinaryBlockLength,
                        senderID, receiverID, signatureHex );

                if( signatureValid ) {
                    returnValue = copy( senderID );
                    }
                
                delete receiverID;
                }
            delete senderID;
            }
        }
    
    for( int i=0; i<numTokens; i++ ) {
        delete [] *( signatureTokens->getElement( i ) );
        }
    delete signatureTokens;

    
    return returnValue;
    }



unsigned char *primrose_internalGetBinarySignatureBlock(
    char *inTextHeader,
    unsigned char *inBinaryBlock, unsigned long inBinaryBlockLength,
    struct UserID *inReceiverID,
    unsigned long *outBlockLength ) {


    unsigned char *returnBlock = NULL;
    *outBlockLength = 0;
    
    
    unsigned long headerLength = strlen( inTextHeader );

    
    unsigned long dataLength = headerLength + inBinaryBlockLength + 8;
    
    unsigned char *fullDataBlock =
        new unsigned char[ dataLength ];

    // fill data block
    memcpy( fullDataBlock, inTextHeader, headerLength );

    if( inBinaryBlock != NULL ) {
        memcpy( &( fullDataBlock[ headerLength ] ), 
                inBinaryBlock, inBinaryBlockLength );
        }

    unsigned char *binaryReceiverID =
        primrose_internalUserIDToBinary( inReceiverID );

    memcpy( &( fullDataBlock[ headerLength + inBinaryBlockLength ] ),
            binaryReceiverID, 8 );

    

    
    char *signatureHex = CryptoUtils::rsaSign(
        primroseLocalPrivateKeyHex,
        fullDataBlock, dataLength );

    delete [] fullDataBlock;

    
    if( signatureHex != NULL ) {

        unsigned char *signatureBinary = hexDecode( signatureHex );
        unsigned long signatureBinaryLength = strlen( signatureHex ) / 2;

        unsigned char *binarySenderID =
            primrose_internalUserIDToBinary( primroseLocalUserID );

        *outBlockLength = 16 + signatureBinaryLength;

        returnBlock = new unsigned char[ *outBlockLength ];

        // fill block
        memcpy( returnBlock, binarySenderID, 8 );
        memcpy( &( returnBlock[8] ), binaryReceiverID, 8 );
        memcpy( &( returnBlock[16] ), signatureBinary, signatureBinaryLength );

        delete [] binarySenderID;
        delete [] signatureBinary;
        
        delete [] signatureHex;
        }

    
    delete [] binaryReceiverID;

    return returnBlock;
    }



char *primrose_internalGetTextSignatureBlock(
    char *inTextHeader,
    unsigned char *inBinaryBlock, unsigned long inBinaryBlockLength,
    struct UserID *inReceiverID ) {


    unsigned long binaryBlockLength;
    unsigned char *binaryBlock =
        primrose_internalGetBinarySignatureBlock(
            inTextHeader,
            inBinaryBlock, inBinaryBlockLength,
            inReceiverID,
            &binaryBlockLength );

    if( binaryBlock == NULL ) {
        return NULL;
        }

    // extract parts
    struct UserID *senderID =
        primrose_internalBinaryToUserID( binaryBlock );

    struct UserID *receiverID =
        primrose_internalBinaryToUserID( &( binaryBlock[8] ) );

    char *signatureHex = hexEncode( &( binaryBlock[16] ),
                                    binaryBlockLength - 16 );

    delete [] binaryBlock;
    
    char *returnString =
        autoSprintf(
            "%lu %lu\n"
            "%lu %lu\n"
            "%s",
            senderID->partA, senderID->partB,
            receiverID->partA, receiverID->partB,
            signatureHex );

    delete senderID;
    delete receiverID;
    delete [] signatureHex;

    return returnString;
    }



unsigned char *primrose_internalReadBlock( Socket *inSocket,
                                           long inLength ) {

    // first deal with explicit length case
    if( inLength != -1 ) {
        unsigned char *buffer = new unsigned char[ inLength ];

        // 20 second timeout
        long numRead = inSocket->receive( buffer, inLength,
                                          20000 );

        if( numRead != inLength ) {
            delete [] buffer;
            return NULL;
            }
        else {
            return buffer;
            }
        }


    // otherwise, read up to the first terminator
    SimpleVector<unsigned char> *receivedBytes =
        new SimpleVector<unsigned char>();

    char error = false;
    char seenOneTerminator = false;

    unsigned char *buffer = new unsigned char[1];
    
    while( !error &&
           !seenOneTerminator ) {

        // receive with a 10 second timeout
        int numRead = inSocket->receive( buffer, 1,
                                         10000 );

        if( numRead != 1 ) {
            error = true;
            }
        else {

            receivedBytes->push_back( buffer[0] );

            if( buffer[0] == '\0' ) {
                seenOneTerminator = true;
                }
            }
        }

    delete [] buffer;

    unsigned char *receivedArray = receivedBytes->getElementArray();

    delete receivedBytes;
    
    if( error ) {
        delete [] receivedArray;
        return NULL;
        }
    else {
        return receivedArray;
        }
    }

