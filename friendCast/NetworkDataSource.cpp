#include "NetworkDataSource.h"



NetworkDataSource::NetworkDataSource( Socket *inSocket )
    : mSocket( inSocket ),
      mBytesRemainingInCurrentPacket( 0 ),
      mFirstRead( true ) {

    mBufferLength = 100000;
    mBufferBytesReady = 0;
    mBuffer = new unsigned char[ mBufferLength ];
    mWorkspaceBuffer = new unsigned char[ mBufferLength ];
    }



NetworkDataSource::~NetworkDataSource() {
    delete mSocket;

    delete [] mBuffer;
    delete [] mWorkspaceBuffer;
    }



int NetworkDataSource::readFromBuffer( unsigned char *inBufferToFill,
                                       int inNumBytes ) {

    int numReadTotal = 0;
    
    
    // first, read whatever is left in buffer
    int numToReadFromBuffer = inNumBytes;

    if( numToReadFromBuffer > mBufferBytesReady ) {
        numToReadFromBuffer = mBufferBytesReady;
        }
    
    // copy into output buffer
    memcpy( inBufferToFill, mBuffer, numToReadFromBuffer );

    // move remaining bytes
    int numBytesRemaining = mBufferBytesReady - numToReadFromBuffer;
    
    unsigned char *pointerToRemainingBytes =
        &( mBuffer[ numToReadFromBuffer ] );

    // copy into beginning of workspace buffer
    memcpy( mWorkspaceBuffer, pointerToRemainingBytes, numBytesRemaining );

    // swap buffers
    unsigned char* tempBufferPointer = mWorkspaceBuffer;
    mWorkspaceBuffer = mBuffer;
    mBuffer = tempBufferPointer;
    
    // save new size
    mBufferBytesReady = numBytesRemaining;


    numReadTotal += numToReadFromBuffer;
    
    
    // next, if buffer was not full enough, read directly from socket

    if( inNumBytes > numToReadFromBuffer ) {

        printf( "Buffer empty\n" );

        
        int numLeftToRead = inNumBytes - numToReadFromBuffer;

        unsigned char *pointerToRemainingSpace =
            &( inBufferToFill[ numToReadFromBuffer ] );

        int numRead = mSocket->receive( pointerToRemainingSpace,
                                        numLeftToRead, -1 );

        if( numRead < 0 ) {
            printf( "Failed to read from socket\n" );
            return numRead;
            }
        else {
            numReadTotal += numRead;
            }
        }

    return numReadTotal;
    }



int NetworkDataSource::readData( unsigned char *inBuffer,
                                 int inDesiredByteCount ) {

    if( mFirstRead ) {
        // fill buffer

        printf( "Filling buffer\n" );

        int numRead = mSocket->receive( mBuffer, mBufferLength, -1 );

        if( numRead != mBufferLength ) {
            printf( "Failed to fill buffer\n" );
            return -1;
            }

        mBufferBytesReady = numRead;

        mFirstRead = false;
        }
    else {
        // check if we should try to refill buffer

        int bytesNeeded = mBufferLength - mBufferBytesReady;

        if( bytesNeeded > 0 ) {

            unsigned char *emptySpotInBuffer =
                &( mBuffer[ mBufferBytesReady ] );
            
            // use timeout of 0 to avoid blocking
            // just get whatever data is available now
            int numRead =
                mSocket->receive( emptySpotInBuffer, bytesNeeded, 0 );

            if( numRead > 0 ) {
                mBufferBytesReady += numRead;
                }
            }
        }

    
    
    if( mBytesRemainingInCurrentPacket == 0 ) {
        // need to start on next packet

        unsigned char flag;

        int numRead = readFromBuffer( &flag, 1 );

        if( numRead != 1 ) {
            return -1;
            }
        
        switch( flag ) {
            case 0x01:
                // format change starting in next packet
                return -2;
                break;
            case 0x00:
                // start of packet
                // read 4-byte big endian size
                unsigned char sizeBytes[4];
                numRead = readFromBuffer( sizeBytes, 4 );

                if( numRead != 4 ) {
                    return -1;
                    }

                // pack bytes into int
                mBytesRemainingInCurrentPacket =
                    sizeBytes[0] << 24 |
                    sizeBytes[1] << 16 |
                    sizeBytes[2] << 8  |
                    sizeBytes[3];                
                break;
            default:
                // unknown flag received
                return -1;
                break;
            }
        }

    
    // cut read off at end of current packet
    int numToRead = inDesiredByteCount;
    if( numToRead > mBytesRemainingInCurrentPacket ) {
        numToRead = mBytesRemainingInCurrentPacket;
        }
    
    int sizeRead = readFromBuffer( inBuffer, numToRead );

    if( sizeRead != numToRead ) {
        return -1;
        }

    mBytesRemainingInCurrentPacket -= sizeRead;
    
    return numToRead;
    }

