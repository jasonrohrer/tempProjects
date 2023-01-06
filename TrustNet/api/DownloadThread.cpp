/*
 * Modification History
 *
 * 2004-November-21   Jason Rohrer
 * Created.
 *
 * 2004-December-7   Jason Rohrer
 * Fixed memory leaks.
 */


#include "Primrose/api/primroseAPI_internal.h"

#include "Primrose/api/DownloadThread.h"

#include "minorGems/network/p2pParts/MultiSourceDownloader.h"
#include "minorGems/util/stringUtils.h"
#include "minorGems/util/SimpleVector.h"

#include "minorGems/io/file/File.h"



DownloadThread::DownloadThread(
    unsigned long inNumHosts,
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
    void *inExtraHandlerArgument )
    : mNumFileSources( inNumHosts ),
      mFileSources( new DownloadHost*[ inNumHosts ] ),
      mFileName( stringDuplicate( inFileName ) ),
      mFileHash( stringDuplicate( inFileHash ) ),
      mFileSize( inFileSize ),
      mDownloadProgressHandler( inDownloadProgressHandler ),
      mExtraHandlerArgument( inExtraHandlerArgument ),
      mFailed( false ) {

    for( int i=0; i<mNumFileSources; i++ ) {
        mFileSources[i] =
            new DownloadHost( inUserIDs[i],
                              inIPAddresses[i],
                              inTCPPorts[i],
                              inUDPPorts[i],
                              inFirewallStatuses[i] );
        }

    start();
    }



DownloadThread::~DownloadThread() {
    stop();
    join();
    
    delete [] mFileName;
    delete [] mFileHash;
    for( int i=0; i<mNumFileSources; i++ ) {
        delete mFileSources[i];
        }
    delete [] mFileSources;
    }



char DownloadThread::isCanceled() {
    // a public wrapper for isStopped
    return isStopped();
    }



int PRIMROSE_DOWNLOAD_IN_PROGRESS = MULTISOURCE_DOWNLOAD_IN_PROGRESS;
int PRIMROSE_DOWNLOAD_FAILED = MULTISOURCE_DOWNLOAD_FAILED;
int PRIMROSE_DOWNLOAD_CANCELED = MULTISOURCE_DOWNLOAD_CANCELED;



// the MultiSourceDownloader progress handler registered by a download thread
char primrose_internalDownloadThreadProgressHandler(
    int inResultCode, unsigned long inTotalBytesReceived,
    void *inExtraArgument ) {

    // extra arg is always download thread
    DownloadThread *thread = (DownloadThread *)inExtraArgument;

    // save our failure statust
    if( inResultCode == MULTISOURCE_DOWNLOAD_CANCELED ||
        inResultCode == MULTISOURCE_DOWNLOAD_FAILED ) {
        thread->mFailed = true;
        }
        
    
    // call our main progress handler
    char shouldContinue =
        thread->mDownloadProgressHandler( inResultCode,
                                          inTotalBytesReceived,
                                          thread->mExtraHandlerArgument );

    // we cancel if our handler wants to cancel -or- if our
    // thread has been canceled
    return shouldContinue && ! thread->isCanceled();
    }



// the MultiSourceDownloader get chunk function registered by a download thread
unsigned char *primrose_internalDownloadThreadGetChunk(
    void *inFileSource, void *inFileDescriptor,
    unsigned long inChunkNumber, unsigned long inChunkSize ) {

    // cast our arguments
    DownloadHost *host = (DownloadHost *)inFileSource;
    char *fileHash = (char *)inFileDescriptor;

    // get a connection to this host
    Socket *sock = primrose_internalGetConnection( host->mIPAddress,
                                                   host->mTCPPort,
                                                   host->mUDPPort,
                                                   host->mFirewallStatus,
                                                   host->mID );

    if( sock != NULL ) {
        // not_firewalled getChunk FB322F86DB95BE0670D06631EE27E83C5B64679A 15

        extern char *primroseFirewallStatus;
        
        char *message = autoSprintf( "%s getChunk %s %lu",
                                     primroseFirewallStatus,
                                     fileHash,
                                     inChunkNumber );
        char *signatureBlock =
            primrose_internalGetTextSignatureBlock(
                message,
                NULL, 0,
                host->mID );

        // send message plus \0
        // followed by
        // signature plus \0
        sock->send( (unsigned char *)message,
                    strlen( message ) + 1 );
        sock->send( (unsigned char *)signatureBlock,
                    strlen( signatureBlock ) + 1 );

        delete [] message;
        delete [] signatureBlock;

        // now read the file chunk
        unsigned long chunkSize = 0;

        // first, read the text header
        char *textHeader = (char *)( primrose_internalReadBlock( sock, -1 ) );

        if( textHeader != NULL ) {
            SimpleVector<char *> *tokens =
                tokenizeString( textHeader );
            int numTokens = tokens->size();
        
            if( numTokens == 5 ) {
                sscanf( *( tokens->getElement(4) ), "%lu", &chunkSize );
                }
            for( int i=0; i<numTokens; i++ ) {
                delete [] *( tokens->getElement( i ) );
                }
            delete tokens;
            }
        else {
            // fill with dummy data to force sig check to fail below
            textHeader = stringDuplicate( "" );
            }

        // make sure chunk size is correct
        if( chunkSize != inChunkSize ) {
            chunkSize = inChunkSize;
            }
        
        // next, read the binary chunk data
        unsigned char *binaryChunkData =
            primrose_internalReadBlock( sock, chunkSize );

        if( binaryChunkData == NULL ) {
            // fill with dummy data to force sig check to fail below
            binaryChunkData = new unsigned char[ chunkSize ];
            }

        // finally, read the text signature block
        char *textSignature =
            (char *)( primrose_internalReadBlock( sock, -1 ) );

        delete sock;
        
        if( textSignature == NULL ) {
            // fill with dummy data to force sig check to fail below
            textSignature = stringDuplicate( "" );
            }

        struct UserID *senderID = 
            primrose_internalCheckSignatureAndGetSenderID(
                textHeader,
                binaryChunkData, chunkSize,
                textSignature );
        
        char sigCorrect = false;
        // make sure senderID matches (in other words, signature is correct)
        if( senderID != NULL ) {
            if( compare( senderID, host->mID ) ) {
                sigCorrect = true;
                }
            delete senderID;
            }

        delete [] textHeader;
        
        delete [] textSignature;

        if( sigCorrect ) {
            return binaryChunkData;
            }
        else {
            delete [] binaryChunkData;
            return NULL;
            }

        }
    else {
        return NULL;
        }
    }



void DownloadThread::run() {
    File *incomingFile = NULL;
    File *finalFile = NULL;

    
    char *sharingPath = primroseGetSharingPath();

    File *sharingDirectory = new File( NULL, sharingPath );
    delete [] sharingPath;

    if( sharingDirectory->exists() && sharingDirectory->isDirectory() ) {
        
        // put file in Primrose_incoming for now
        File *incomingDir =
            sharingDirectory->getChildFile( "Primrose_incoming" );

        if( incomingDir != NULL ) {
            if( ! incomingDir->exists() ) {
                incomingDir->makeDirectory();
                }

            if( incomingDir->exists() && incomingDir->isDirectory() ) {

                incomingFile = incomingDir->getChildFile( mFileName );
                }
            delete incomingDir;
            }

        finalFile = sharingDirectory->getChildFile( mFileName );
        }
    
    delete sharingDirectory;

    
    if( incomingFile != NULL && finalFile != NULL ) {
        char *incomingPath = incomingFile->getFullFileName();

        extern unsigned long primroseChunkSize;
        
        multiSourceGetFile( (void *)mFileHash,
                            mFileSize,
                            primroseChunkSize,
                            mNumFileSources,
                            (void **)mFileSources,
                            primrose_internalDownloadThreadGetChunk,
                            primrose_internalDownloadThreadProgressHandler,
                            (void *)this,  // thread is extra argument
                            incomingPath );

        // when function returns, our download is complete or failed

        if( !mFailed ) {
            // move download out of incoming directory
            incomingFile->copy( finalFile );
            incomingFile->remove();
            }
        
        delete [] incomingPath;
        }
    else {
        // could not get proper local files for saving download
        mDownloadProgressHandler( PRIMROSE_DOWNLOAD_FAILED,
                                  0, mExtraHandlerArgument );
        }

    if( incomingFile != NULL ) {
        delete incomingFile;
        }
    if( finalFile != NULL ) {
        delete finalFile;
        }
    
    }
