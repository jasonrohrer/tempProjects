

#include "minorGems/network/Socket.h"
#include "minorGems/network/SocketUDP.h"

#include "minorGems/util/stringUtils.h"
#include "minorGems/io/file/File.h"
#include "minorGems/system/Thread.h"


#include "minorGems/util/random/StdRandomSource.h"


#include <stdio.h>
#include <time.h>


unsigned long chunkSize = 400;


class ChunkReceiverThread : public Thread {
    public:

        ChunkReceiverThread( const char *inSaveFileName,
                             SocketUDP *inSocket )
        : mBytesReceived( 0 ),
          mSaveFileName( stringDuplicate( inSaveFileName ) ),
          mFILE( NULL ),
          mNumChunks( 0 ),
          mChunkMap( NULL ),
          mSocket( inSocket ),
          mRandSource( new StdRandomSource() ),
          mWaitingTally( 0 ) {

            
            }


        
        ~ChunkReceiverThread() {
            delete [] mSaveFileName;
            delete [] mChunkMap;
            fclose( mFILE );

            delete mRandSource;
            }

        void waitForChunks( int inWaitForCount ) {
            mWaitingTally = 0;
            while( mWaitingTally < inWaitForCount ) {
                Thread::sleep( 10 );
                }
            }

        // returns 0 in outNumReturned if no chunks needed, and returns NULL
        // else returns an array of chunk numbers
        unsigned long *getNeededChunkNumbers( int inNumToGet,
                                              int *outNumReturned ) {

            unsigned long *returnArray = new unsigned long[ inNumToGet ];
            
            if( mChunkMap != NULL ) {

                int numFound = 0;
                for( unsigned long i=0;
                     i<mNumChunks && numFound < inNumToGet;
                     i++ ) {
                    
                    if( ! mChunkMap[i] ) {
                        returnArray[numFound] = i;
                        numFound++;
                        }
                    }

                if( numFound > 0 ) {
                    *outNumReturned = numFound;
                    return returnArray;
                    }
                else {
                    // no chunks needed
                    delete [] returnArray;
                    *outNumReturned = -1;
                    return NULL;
                    }
                }
            else {
                // no chunk map yet... still need chunk 0
                returnArray[0] = 0;
                *outNumReturned = 1;
                
                return returnArray;
                }
            }


        
        // implements the Thread interface
        void run() {

            char done = false;

            while( !done ) {
                // receive a chunk
                struct UDPAddress *senderAddress;
                char *messageData;

                int numRead =
                    mSocket->receive( &senderAddress,
                                      (unsigned char **)( &messageData ) );

                delete senderAddress;

                // header of message ends with a null char, so we can
                // treat message as a string to process the header

                int numParts;
                char **messageParts =
                    split( (char *)messageData, " ", &numParts );

                // part 1 is chunk number, part 3 is total number of chunks
                // part 5 is total file size

                unsigned long chunkNumber = 0;
                unsigned long numChunks = 0;
                unsigned long fileSize = 0;
            
                sscanf( messageParts[1], "%lu", &chunkNumber );
                sscanf( messageParts[3], "%lu", &numChunks );
                sscanf( messageParts[5], "%lu", &fileSize );
            
                for( int i=0; i<numParts; i++ ) {
                    delete [] messageParts[i];
                    }
                delete [] messageParts;


                if( mFILE == NULL ) {
                    mNumChunks = numChunks;
                
                    // first fill the file with 0 values
                    mFILE = fopen( mSaveFileName, "wb" );

                    unsigned long bufferSize = 5000;
                    unsigned char *buffer = new unsigned char[ bufferSize ];
                
                    unsigned long i;
                    while( i<fileSize ) {
                        unsigned long writeSize = bufferSize;
                        if( fileSize - i < bufferSize ) {
                            writeSize = fileSize - i;
                            }
                        fwrite( buffer, 1, writeSize, mFILE );
                        i += writeSize;
                        }
                    delete [] buffer;
                
                    fclose( mFILE );

                    // open for reading and writing
                    mFILE = fopen( mSaveFileName, "r+b" );

                    // create the chunk map
                    mChunkMap = new char[ mNumChunks ];
                    for( i=0; i<mNumChunks; i++ ) {
                        mChunkMap[i] = false;
                        }
                    }
            

                if( chunkNumber < mNumChunks ) {
                    // write this chunk

                    // skip the header and its terminating \0
                    unsigned char *pointerToChunk =
                        (unsigned char *)(
                            &( messageData[ strlen( messageData ) + 1 ] ) );

                    unsigned long chunkSize =
                        numRead - strlen( messageData ) - 1;

                    // seek to the start of the chunk
                    unsigned long offset = chunkNumber * chunkSize;
                    fseek( mFILE, offset, SEEK_SET );

                    // write the chunk
                    fwrite( pointerToChunk, 1, chunkSize, mFILE ); 

                    if( chunkNumber % 1000 == 0 ) {
                        printf( "Got chunks up to %ld\n", chunkNumber );
                        }
                    mBytesReceived += chunkSize;

                    mWaitingTally++;
                    
                    // mark our chunk map
                    mChunkMap[ chunkNumber ] = true;
                    }

                int numNeeded;
                unsigned long *neededArray =
                    getNeededChunkNumbers( 1, &numNeeded );
                
                if( numNeeded == -1 ) {
                    // done
                    done = true;
                    }
                else {
                    delete [] neededArray;
                    }
                }
            }

        unsigned long mBytesReceived;

    private:
        char *mSaveFileName;
        FILE *mFILE;
        unsigned long mNumChunks;
        char *mChunkMap;
        SocketUDP *mSocket;
        StdRandomSource *mRandSource;

        int mWaitingTally;
        
    };


int main( int inNumArgs, char **inArgs ) {

    Socket::initSocketFramework();
    
    
    // arg 1 is port we run on
    unsigned short ourPort = 8000;

    if( inNumArgs > 1 ) {
        sscanf( inArgs[1], "%hu", &ourPort );
        }

    // Start our UDP endpoint
    SocketUDP *socket = new SocketUDP( ourPort );
    

    if( inNumArgs < 3 ) {
        // go into server mode

        printf( "Server mode\n" );


        while( true ) {
            // receive a chunk request
            struct UDPAddress *senderAddress;
            char *messageData;

            int numRead =
                socket->receive( &senderAddress,
                                 (unsigned char **)( &messageData ) );

            if( numRead != -1 ) {

                // copy message data into a true string
                char *receivedString = new char[ numRead + 1 ];
                receivedString[ numRead ] = '\0';

                memcpy( (void *)receivedString, (void *)messageData, numRead );

                delete [] messageData;


                unsigned short theirPort;
                char *theirIPAddress =
                    SocketUDP::extractAddress( senderAddress, &theirPort );
                

                // split based on whitespace
                int numParts;
                char **messageParts =
                    split( receivedString, " ", &numParts );
                
                if( numParts > 0 ) {
                    // first part is file name

                    char *fileName = messageParts[0];

                    /*
                    if( numParts == 1 ) {

                        // send them a message telling them how
                        // many chunks we have
                        File *file = new File( NULL, fileName );

                        unsigned long numChunks = 0;
                        
                        if( file->exists() ) {
                            unsigned long length = file->getLength();

                            numChunks = length / chunkSize;

                            if( length % chunkSize != 0 ) {
                                // extra partial chunk
                                numChunks += 1;
                                }
                            }

                        printf( "Sending chunk count of file %s to"
                                "%s:%hu\n", fileName,
                                theirIPAddress, theirPort );

                        char *ourMessage =
                            autoSprintf(
                                
                        
                        }
                    */
                        
                    if( numParts >= 2 ) {
                        // second part, and additional parts, should be chunk
                        // numbers

                        for( int i=1; i<numParts; i++ ) {
                            unsigned long chunkNumber = 0;

                            sscanf( messageParts[i], "%lu", &chunkNumber );
                        
                        

                            File *file = new File( NULL, fileName );

                            unsigned long numChunks = 0;
                            unsigned long fileSize = 0;
                        
                            if( file->exists() ) {
                                fileSize = file->getLength();

                                numChunks = fileSize / chunkSize;

                                if( fileSize % chunkSize != 0 ) {
                                    // extra partial chunk
                                    numChunks += 1;
                                    }
                                }
                            delete file;

                        
                            if( chunkNumber < numChunks ) {
                                /*
                                printf( "Sending chunk %lu of file %s to "
                                        "%s:%hu\n", chunkNumber, fileName,
                                        theirIPAddress, theirPort );
                                */
                                char *ourHeader = autoSprintf(
                                    "%s %ld of %ld chunks %ld totalBytes",
                                    fileName, chunkNumber, numChunks,
                                    fileSize );

                            
                                unsigned long offset = chunkNumber * chunkSize;

                                unsigned long thisChunkSize = chunkSize;

                                if( fileSize - offset < chunkSize ) {
                                    // partial chunk
                                    thisChunkSize = fileSize - offset;
                                    }

                                FILE *fileHandle = fopen( fileName, "rb" );

                                fseek( fileHandle, offset, SEEK_SET );
                            
                                unsigned char *chunkData =
                                    new unsigned char[ thisChunkSize ];

                                fread( chunkData,
                                       1, thisChunkSize, fileHandle );

                                fclose( fileHandle );

                            
                                // pack header and chunk data into message body
                                // separate them with \0
                                unsigned long messageLength =
                                    strlen( ourHeader ) + 1 +
                                    thisChunkSize;
                            
                                unsigned char *messageBody =
                                    new unsigned char[ messageLength ];

                                // include the \0 at the end of the header
                                memcpy( messageBody, ourHeader,
                                        strlen( ourHeader ) + 1 );

                                // skip the header and its trailing \0
                                unsigned char *chunkStartPointer =
                                    &( messageBody[
                                        strlen( ourHeader ) + 1 ] );

                                memcpy( chunkStartPointer, chunkData,
                                        thisChunkSize );

                                delete [] ourHeader;
                                delete [] chunkData;

                                // send the message
                                socket->send( senderAddress,
                                              messageBody, messageLength );

                                delete [] messageBody;
                                }
                            else {
                                printf(
                                    "Got request for out-of-range chunks\n" );
                                printf( "   File %s only has %lu chunks but "
                                        "chunk %lu requested\n",
                                        fileName, numChunks, chunkNumber );
                                }
                            }
                        }
                    }
                
                for( int i=0; i<numParts; i++ ) {
                    delete [] messageParts[i];
                    }
                delete [] messageParts;
                
                delete [] theirIPAddress;
                delete senderAddress;
                }
                
            }
        }
    else {

        // arg 2 is IP of remote host
        char *remoteIP = "127.0.0.1";
        if( inNumArgs > 2 ) {
            remoteIP = inArgs[2];
            }
        
        // arg 3 is port of remote host
        unsigned short remotePort = 8000;
        if( inNumArgs > 3 ) {
            sscanf( inArgs[3], "%hu", &remotePort );
            }
                
        // arg 4 is file name to get
        char *fileName = "test";
        if( inNumArgs > 4 ) {
            fileName = inArgs[4];
            }

        // arg 5 is save file name
        char *saveFileName = "localFile";
        if( inNumArgs > 5 ) {
            saveFileName = inArgs[5];
            }

        struct UDPAddress *remoteAddress =
            SocketUDP::makeAddress( remoteIP, remotePort );
        
        char done = false;
        unsigned long bytesReceived = 0;
        
        FILE *fileHandle = fopen( saveFileName, "wb" );
        
        unsigned long startTime = time( NULL );

        ChunkReceiverThread *thread =
            new ChunkReceiverThread( saveFileName, socket );

        thread->start();
        
        
        while( !done ) {
            // send a request for a chunk
            int numRequested = 500;
            int numNumbersNeeded;
            unsigned long *neededChunkNumbers =
                thread->getNeededChunkNumbers( numRequested,
                                               &numNumbersNeeded );

            if( numNumbersNeeded != -1 ) {
                // compose our list as a string
                char **numberStrings = new char*[ numNumbersNeeded ];
                int i;
                for( i=0; i<numNumbersNeeded; i++ ) {
                    numberStrings[i] = autoSprintf( "%ld",
                                                 neededChunkNumbers[ i ] );
                    }
                char *numberList =
                    join( numberStrings, numNumbersNeeded, " " );
                for( i=0; i<numNumbersNeeded; i++ ) {
                    delete [] numberStrings[i];
                    }
                delete [] numberStrings;
                
                
                char *request = autoSprintf( "%s %s", fileName,
                                             numberList );

                delete [] numberList;
                
                socket->send( remoteAddress,
                              (unsigned char *)request, strlen( request ) );

                delete [] request;

                
                delete [] neededChunkNumbers;

                
                // wait for a fraction of our chunks to come in
                //if( numNumbersNeeded > 2 ) {
                //int waitFor = numNumbersNeeded;
                    //(int)( numNumbersNeeded * .80 );
                    //thread->waitForChunks( waitFor );
                    /*    }
                else {
                    // just wait
                    Thread::sleep( 100 );
                    }
                    */
                Thread::sleep( 600 );
                }
            else {
                // all chunks received
                done = true;
                }
            }
        
        thread->join();
        bytesReceived = thread->mBytesReceived;
        delete thread;
        
        unsigned long netTime = time( NULL ) - startTime;

        printf( "Got %lu bytes in %lu seconds\n"
                "%f KiB/second\n",
                bytesReceived, netTime,
                ( (float)bytesReceived / (float)netTime ) / 1000 );
        
        fclose( fileHandle );

        
        delete remoteAddress;
        
        }

    delete socket;
    
    return 0;
    }
