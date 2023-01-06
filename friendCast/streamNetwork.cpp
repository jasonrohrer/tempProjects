#include "streamNetwork.h"
#include "streamFileTracker.h"
#include "SentFileTracker.h"

#include "minorGems/network/Socket.h"
#include "minorGems/network/SocketServer.h"
#include "minorGems/util/SimpleVector.h"
#include "minorGems/util/stringUtils.h"
#include "minorGems/system/Time.h"

#include <stdio.h>



// number of bytes in a file data chunk
int dataChunkSize = 4000;



SocketServer *server;




// a socket for each remote host
SimpleVector<Socket *> clientSocketConnections;

// an index into our stream file list for each remote host
// -1 if remote host is currently not playing from our stream
SimpleVector<long> clientStreamFileIndices;

// track the files that we have already sent to each client
SimpleVector<SentFileTracker *> clientSentFileTrackers;



// true if remote client has received last file in our stream and is
// waiting for more files to arrive
SimpleVector<char> clientStreamStalled;

// the file currently being sent to each client
SimpleVector<FILE *> clientStreamFiles;

// true if we're moving on to next file in stream, but we haven't told
// client about it yet.
SimpleVector<char> clientStreamNewFileFlags;




// index of the socket that we are receiving our locally-playing data from
int sourceSocketIndex = -1;
Socket *sourceSocket = NULL;

FILE *sourceReceivedFile = NULL;

// flag to indicate that our source socket changed and that we should
// send a new control packet
char sourceSocketChanged = false;




// accumulated bytes read from current data packet arriving from source

// packet types
#define STOP_CONTROL_PACKET         0x00
#define PLAY_CONTROL_PACKET         0x01
#define NO_CHANGE_CONTROL_PACKET    0x02

#define NEW_FILE_DATA_PACKET        0x12
#define FILE_CHUNK_DATA_PACKET      0x13

#define NO_PACKET 0xFF


class IncomingPacket {
    public:

        IncomingPacket();

        ~IncomingPacket();
        
        unsigned char mType;

        // the actual size decoded from the 4 size bytes
        int mSize;
        
        // the actual data of the incoming packet
        unsigned char *mData;

        

        /**
         * Receives more data to build this packet.
         *
         * @param inSocket the socket to read more data from.
         *   Destroyed by caller.
         *
         * @return true if this read call completes the packet,
         *   or false if more data needs to be read before packet
         *   is complete.
         */
        char receiveMoreData( Socket *inSocket );


        
    private:

        // how many packet bytes we have received (from 0 to
        // mPacketSize)
        int mBytesReceived;

        // the bytes that make up the packet's size
        unsigned char mSizeBytes[4];

        // the number of bytes, out of the 4 size bytes, that we have
        // received so far
        int mNumSizeBytesReceived;

    };




// packet bytes waiting to go out for each client
// NULL for a given client if no packet is waiting
SimpleVector< unsigned char *> clientPacketBytes;
SimpleVector<long> clientPacketSizes;

// number of bytes sent fromm each packet so far 
SimpleVector<long> clientPacketNumSent;

// packets coming in from each client connection
SimpleVector<IncomingPacket *> clientIncomingPackets;




/**
 * Reads from a socket, without blocking, to fill a buffer as much
 * as possible.
 *
 * All params must be destroyed by caller.
 *
 * @param inSocket the socket to read from.
 * @param inBuffer the buffer to fill.
 * @param inBufferSize the total size of the buffer.
 * @param inNumBytesAlreadyRead how many bytes have already been read
 *   into the buffer.
 *
 * @return the new number of bytes in the buffer so far.
 *   Returns -1 on a socket error.
 */
int socketReadToFillBuffer( Socket *inSocket,
                            unsigned char *inBuffer,
                            int inBufferSize,
                            int inNumBytesAlreadyRead );



/**
 * Processes an incoming data packet that was received from our source
 * socket.
 *
 * @param inPacket the packet to process.
 *   Destroyed by caller.
 */
void processDataPacket( IncomingPacket *inPacket );



/**
 * Starts streaming files to a client.
 *
 * @param inClientIndex the index of the client in our client list.
 * @param inFileIndex the index of the file to start streaming.
 */ 
void startClientOnFileIndex( int inClientIndex, int inFileIndex );



/**
 * Executes a single network step and returns.
 *
 * @param inTimeLimitInMilliseconds a time limit on the step.
 *
 * @return true if there is more work waiting to be done (in other words,
 *   calling executeNetworkStepInternal again will not simply waste CPU
 *   cycles).
 */
char executeNetworkStepInternal( unsigned long inTimeLimitInMilliseconds );



void startNetwork( int inPort ) {
    server = new SocketServer( inPort, 100 );
    }



void stopNetwork() {
    delete server;

    int numClients = clientSocketConnections.size();

    for( int i=0; i<numClients; i++ ) {
        delete *( clientSocketConnections.getElement( i ) );
        delete *( clientSentFileTrackers.getElement( i ) );

        FILE *clientFile = *( clientStreamFiles.getElement( i ) );
        
        if( clientFile != NULL ) { 
            fclose( clientFile );
            *( clientStreamFiles.getElement( i ) ) = NULL;
            }

        unsigned char *packetBytes =
            *( clientPacketBytes.getElement( i ) );

        if( packetBytes != NULL ) {
            delete [] packetBytes;
            *( clientPacketBytes.getElement( i ) ) = NULL;
            }

        delete *( clientIncomingPackets.getElement( i ) );
        }

    if( sourceReceivedFile != NULL ) {
        fclose( sourceReceivedFile );
        sourceReceivedFile = NULL;
        }

    clearTracker();
    }



// wrapper that calls executeNetworkStepInternal as long as more
// work is waiting and more time is left
void executeNetworkStep( unsigned long inTimeLimitInMilliseconds ) {

    unsigned long startSeconds, startMilliseconds;
    Time::getCurrentTime( &startSeconds, &startMilliseconds );
    
    int timeLeft = inTimeLimitInMilliseconds;
    char moreWorkLeft = true;

    int numSteps = 0;
    while( moreWorkLeft && timeLeft > 0 ) {

        moreWorkLeft = executeNetworkStepInternal( timeLeft );

        timeLeft = inTimeLimitInMilliseconds -
            Time::getMillisecondsSince( startSeconds, startMilliseconds );
        
        numSteps++;
        }
    }



char executeNetworkStepInternal( unsigned long inTimeLimitInMilliseconds ) {
    char moreWorkPossibleOnSomeSocket = false;

    
    unsigned long startSeconds, startMilliseconds;
    Time::getCurrentTime( &startSeconds, &startMilliseconds );

    unsigned long millisecondsLeft = inTimeLimitInMilliseconds;

    
    // accept any pending connections

    // timeout 0
    Socket *sock = server->acceptConnection( 0 );

    while( sock != NULL ) {
        addClientSocket( sock );

        // see if more are waiting
        sock = server->acceptConnection( 0 );
        }


    // no more new connections waiting

    
    // account for time we spent accepting     
    millisecondsLeft = inTimeLimitInMilliseconds - 
        Time::getMillisecondsSince( startSeconds,
                                    startMilliseconds );


    // receive all available data from each socket (without blocking)
    int numSockets = clientSocketConnections.size();


    unsigned long millisecondsPerSocketReceiving =
        ( millisecondsLeft / 2 )   // half time spent on receiving
        / numSockets;     // spread equally among sockets

    // always at least one ms per receive
    if( millisecondsPerSocketReceiving < 1 ) {
        millisecondsPerSocketReceiving = 1;
        }
    

    for( int i=0; i<numSockets; i++ ) {

        unsigned long receiveStartTimeSec, receiveStartTimeMillisec;
        Time::getCurrentTime( &receiveStartTimeSec,
                              &receiveStartTimeMillisec );

        int timeLeft = millisecondsPerSocketReceiving;
        
        
        Socket *sock = *( clientSocketConnections.getElement( i ) );

        char moreDataAvailable = true;

        // keep receiving as long as our non-blocking receive calls
        // are fulfilled
        // Stop trying as soon as a receive call comes back only partly filled
        // (in other words, when we have exhausted the data that has arrived
        //  so far)
        // Also stop if we run out of time
        while( moreDataAvailable && timeLeft > 0 ) {

            IncomingPacket *packet =
                *( clientIncomingPackets.getElement( i ) );


            char packetDone = packet->receiveMoreData( sock );
                        
            moreDataAvailable = packetDone;

            if( packetDone ) {
                switch( packet->mType ) {
                    case STOP_CONTROL_PACKET:
                        printf( "Got STOP packet from client %d\n", i );
                        // stop sending data to this client
                        if( *( clientStreamFileIndices.getElement( i ) )
                            != -1 ) {

                            // close the file the client was streaming from
                            if( *( clientStreamFiles.getElement( i ) ) ) {
                                fclose(
                                    *( clientStreamFiles.getElement( i ) ) );
                                }
                                
                            *( clientStreamFiles.getElement( i ) ) = NULL;
                            *( clientStreamFileIndices.getElement( i ) )
                                = -1;
                            *( clientStreamNewFileFlags.getElement( i ) )
                                = false;
                            *( clientStreamStalled.getElement( i ) ) = false;


                            SentFileTracker *tracker =
                                *( clientSentFileTrackers.getElement( i ) );
                            
                            // clear the list of files we have sent to this
                            // client
                            // If they play from our channel again, we start
                            // them from scratch.
                            tracker->clearTracker();

                            // DO NOT
                            // clear last packet that was waiting to be sent
                            // since we never want to send partial packets
                            // The client is expecting only whole packets
                            }                        
                        break;
                    case PLAY_CONTROL_PACKET:
                        printf( "Got PLAY packet from client %d\n", i );
                        //printf( "Got play packet\n" );
                        // start sending data to this client
                        // if we're not already
                        if( *( clientStreamFileIndices.getElement( i ) )
                            == -1 ) {
                            
                            int currentIndex = getCurrentFileIndex();
                            
                            startClientOnFileIndex( i, currentIndex );
                            }
                        else if(
                            *( clientStreamStalled.getElement( i ) ) ) {
                            
                            // currently stalled
                            int clientFileIndex = 
                                *( clientStreamFileIndices.getElement( i ) );
                            
                            if( getMaxFileIndex() > clientFileIndex ) {
                                // more stream files have arrived
                                // can unstall this client
                                
                                clientFileIndex ++;
                                                    
                                startClientOnFileIndex( i, clientFileIndex );
                                }
                            }
                                     
                        break;
                    case NO_CHANGE_CONTROL_PACKET:
                        // do nothing
                        break;
                    case FILE_CHUNK_DATA_PACKET:
                    case NEW_FILE_DATA_PACKET:
                        if( sock == sourceSocket ) {
                            // we are receiving data packets from this source
                            processDataPacket( packet );
                            }
                        break;
                    default:
                        printf( "Unknown packet type %hhX\n", packet->mType );
                    }

                // prepare for next incoming packet
                delete packet;

                *( clientIncomingPackets.getElement( i ) ) =
                    new IncomingPacket();
                }

            // how much time is left for receiving on this socket?
            timeLeft = millisecondsPerSocketReceiving -
                Time::getMillisecondsSince( receiveStartTimeSec,
                                            receiveStartTimeMillisec );

            }  // end while( moreDataAvailable and more time left )

        if( moreDataAvailable ) {
            moreWorkPossibleOnSomeSocket = true;
            }
        
        }  // end loop over all sockets
    


    // account for time we spent receiving     
    millisecondsLeft = inTimeLimitInMilliseconds - 
        Time::getMillisecondsSince( startSeconds,
                                    startMilliseconds );
    

    unsigned long millisecondsPerSocketSending =
        millisecondsLeft   // all remaining time spent sending
        / numSockets;     // spread equally among sockets


    // always at least one ms per send
    if( millisecondsPerSocketSending < 1 ) {
        millisecondsPerSocketSending = 1;
        }
    
    
    // send data on each socket
    // send as much as possible without blocking or running out of time
    
    for( int i=0; i<numSockets; i++ ) {

        unsigned long sendStartTimeSec, sendStartTimeMillisec;
        Time::getCurrentTime( &sendStartTimeSec,
                              &sendStartTimeMillisec );

        int timeLeft = millisecondsPerSocketSending;

        
        
        Socket *sock = *( clientSocketConnections.getElement( i ) );

        char canSendMore = true;
        char stalled = false;
            
        // keep sending as long as our send calls are completely fulfilled
        // stop as soon as we hit a partial send
        // also, stop sending if we become stalled
        // also, stop if we run out of time for this socket
        while( canSendMore
               && !stalled
               && timeLeft > 0 ) {
            

            // vector of bytes in packet waiting to go out
            unsigned char *packetBytes =
                *( clientPacketBytes.getElement( i ) );
            long packetSize = *( clientPacketSizes.getElement( i ) );
            long numSent = *( clientPacketNumSent.getElement( i ) );

            // only formulate a new data packet for this client
            // if it is currently playing our stream  AND it is not stalled
            if( packetBytes == NULL
                &&
                *( clientStreamFileIndices.getElement( i ) ) != -1
                &&
                ! *( clientStreamStalled.getElement( i ) ) ) {

                // compose a new packet of data to send

                // we stick one control packet in with each data packet
                // control packets only take one byte
                
                // first 6 bytes are:
                // -- control packet
                // -- type flag for data packet
                // -- 4 size bytes for data packet
                packetBytes = new unsigned char[ 6 + dataChunkSize ];

                // first, stick our control packet in
                if( sourceSocketChanged ) {
                    if( sock == sourceSocket ) {
                        // play control packet
                        packetBytes[0] = PLAY_CONTROL_PACKET;

                        // we have dealt with the need to
                        // send a play packet to the new source
                        sourceSocketChanged = false;
                        }
                    // stop packet was sent to previous source socket
                    // when source changed
                    }
                else {
                    // no change in control
                    packetBytes[0] = NO_CHANGE_CONTROL_PACKET;
                    }
                
                int packetDataSize = dataChunkSize;
                numSent = 0;

                // read data from file for client
                FILE *file = *( clientStreamFiles.getElement( i ) );
                int numRead = fread(
                    // skip into packet past control packet, type, and
                    // 4 size bytes
                    &( packetBytes[6] ),
                    1, dataChunkSize, file );                


                if( numRead > 0 ) {
                    
                    if( *( clientStreamNewFileFlags.getElement( i ) ) ) {
                        // new file packet
                        packetBytes[1] = NEW_FILE_DATA_PACKET;

                        *( clientStreamNewFileFlags.getElement( i ) )
                            = false;
                        }
                    else {
                        // next packet of current file
                        packetBytes[1] = FILE_CHUNK_DATA_PACKET;
                        }
                    
                    // big-endian packet data size
                    packetBytes[2] = (numRead >> 24) & 0xFF;
                    packetBytes[3] = (numRead >> 16) & 0xFF;
                    packetBytes[4] = (numRead >>  8) & 0xFF;
                    packetBytes[5] = (numRead >>  0) & 0xFF;

                    packetSize = 6 + numRead;
                    }
                else {
                    // no packet (maybe end of file)
                    delete [] packetBytes;
                    packetBytes = NULL;
                    packetSize = 0;
                    numSent = 0;
                    }

                
                if( numRead != dataChunkSize ) {
                    // end of file
                    fclose( file );

                    // open next file for next packet
                        
                    int clientFileIndex =
                        *( clientStreamFileIndices.getElement( i ) );
                        
                        
                    if( clientFileIndex < getMaxFileIndex() ) {
                        clientFileIndex ++;
                        }
                    else {
                        // loop back to start of our current channel
                        clientFileIndex = getStartOfCurrentChannel();
                        }
                    
                    startClientOnFileIndex( i, clientFileIndex );

                    // watch stalled flag here to jump out of while loop
                    stalled = *( clientStreamStalled.getElement( i ) );
                            
                    }
                
                }
            

            // is packet data waiting to go out?
            if( packetBytes != NULL ) {
                // bytes are waiting to go out
                
                int numLeftToSend = packetSize - numSent;
                
                int numSentThisTime =
                    sock->send(
                        // skip past sent bytes in buffer
                        &( packetBytes[ numSent ] ),
                        numLeftToSend,
                        // no blocking
                        false );
                
                if( numSentThisTime > 0 ) {
                    numSent += numSentThisTime;
                    }

                if( numSent == packetSize ) {
                        
                    // done sending this packet
                    delete [] packetBytes;
                    packetBytes = NULL;

                    packetSize = 0;
                    numSent = 0;
                    }
                else {
                    canSendMore = false;
                    }
                }
            else {
                // no packet data waiting to go out

                // send a control packet if we need to
                if( sourceSocketChanged ) {
                    if( sock == sourceSocket ) {
                        unsigned char controlPacket = PLAY_CONTROL_PACKET;


                        int numSent = sock->send( &controlPacket, 1, false );

                        if( numSent == 1 ) {
                            // we have dealt with the need to send a play
                            // packet to the new source
                            sourceSocketChanged = false;
                            }
                        // else try sending packet again next time
                        }
                    }
                    
                canSendMore = false;
                }

            // save back into vectors
            *( clientPacketBytes.getElement( i ) ) = packetBytes;
            *( clientPacketSizes.getElement( i ) ) = packetSize;
            *( clientPacketNumSent.getElement( i ) ) = numSent;



            // how much time is left for sending on this socket?
            timeLeft = millisecondsPerSocketSending -
                Time::getMillisecondsSince( sendStartTimeSec,
                                            sendStartTimeMillisec );
            
            }  // end of while canSendMore

        
        if( canSendMore ) {
            moreWorkPossibleOnSomeSocket = true;
            }

        }  // end loop over all sockets

    return moreWorkPossibleOnSomeSocket;
    }



void setSourceSocket( Socket *inSocket ) {

    if( sourceSocketIndex != -1 ) {
        // we are switching from an existing source socket

        // make sure we send at least one stop packet through this socket,
        // since we may start playing from this source again right away,
        // and in that case we would need the remote host to restart our
        // stream for us (a stop, followed by a play, would do this)

        unsigned char *packet =
            *( clientPacketBytes.getElement( sourceSocketIndex ) );

        int packetSize =
            *( clientPacketSizes.getElement( sourceSocketIndex ) );
        
        if( packet != NULL ) {
            // new packet with one extra byte:  our STOP packet
            unsigned char *newPacket = new unsigned char[ packetSize + 1 ];

            memcpy( newPacket, packet, packetSize );

            // extra byte
            newPacket[ packetSize ] = STOP_CONTROL_PACKET;

            delete [] packet;

            *( clientPacketBytes.getElement( sourceSocketIndex ) ) =
                newPacket;

            *( clientPacketSizes.getElement( sourceSocketIndex ) ) =
                packetSize + 1;
            
            }
        else {
            // no existing packet waiting to go out on this socket
            // make one that contains just our control byte
            packet = new unsigned char[1];
            packet[0] = STOP_CONTROL_PACKET;

            *( clientPacketBytes.getElement( sourceSocketIndex ) ) =
                packet;

            *( clientPacketSizes.getElement( sourceSocketIndex ) ) = 1;
            }

        
        // close old file
        if( sourceReceivedFile != NULL ) {
            fclose( sourceReceivedFile );
            sourceReceivedFile = NULL;
            }
        }

    
    char found = false;
    int foundIndex = -1;
    int numClients = clientSocketConnections.size();

    for( int i=0; i<numClients && !found; i++ ) {
        if( *( clientSocketConnections.getElement( i ) ) == inSocket ) {
            found = true;
            foundIndex = i;
            }
        }
    
    if( !found ) {
        // add a new socket to client list
        sourceSocket = inSocket;
        sourceSocketIndex = addClientSocket( inSocket );
        }
    else {
        // point to existing socket in list 
        sourceSocketIndex = foundIndex;
        sourceSocket = inSocket;
        }

    
    // we need to send a new control packet
    sourceSocketChanged = true;
    }



void nextChannel() {
    int newIndex = sourceSocketIndex + 1;

    int maxIndex = clientSocketConnections.size() - 1;
    
    if( newIndex > maxIndex ) {
        // wrap around
        newIndex = 0;
        }

    setSourceSocket( *( clientSocketConnections.getElement( newIndex ) ) );
    }



int socketReadToFillBuffer( Socket *inSocket,
                            unsigned char *inBuffer,
                            int inBufferSize,
                            int inNumBytesAlreadyRead ) {

    // more to read
    int numLeft = inBufferSize -
        inNumBytesAlreadyRead;
                    
    int numReceived = inSocket->receive(
        // skip in buffer past bytes received
        &( inBuffer[ inNumBytesAlreadyRead ] ),
        numLeft,
        // no timeout (return right away)
        0 );

    if( numReceived >= 0 ) {
        return inNumBytesAlreadyRead + numReceived;
        }
    else if( numReceived == -2 ) {
        // timeout
        return inNumBytesAlreadyRead;
        }
    else {
        // error
        return -1;
        }
    }



void processDataPacket(  IncomingPacket *inPacket ) {

    if( inPacket->mType == NEW_FILE_DATA_PACKET ) {

        // close existing file
        if( sourceReceivedFile != NULL ) {
            fclose( sourceReceivedFile );
            }

        char *newFileName = getUniqueFileName();

        sourceReceivedFile = fopen( newFileName, "wb" );

        if( sourceReceivedFile != NULL ) {
            // add this file to stream
            addFileName( newFileName );
            }
        else {
            printf( "Error:  failed to open file %s\n", newFileName );
            delete [] newFileName;
            }
        }

    if( sourceReceivedFile != NULL ) {
        // write packet to current file

        int dataSize = inPacket->mSize;
    
        int numWritten =
            fwrite( inPacket->mData, 1, dataSize, sourceReceivedFile );

        if( numWritten != dataSize ) {
            printf( "Error:  failed to write %d bytes to file.\n",
                    dataSize );
            }
        }
    // else skip this packet until a NEW_FILE_DATA_PACKET causes
    // us to create a new receive file
    
    }



int addClientSocket( Socket *inSocket ) {
    clientSocketConnections.push_back( inSocket );
    // start off not playing our stream
    clientStreamFileIndices.push_back( -1 );
    clientSentFileTrackers.push_back( new SentFileTracker() );
    clientStreamStalled.push_back( false );
    clientStreamFiles.push_back( NULL );
    clientStreamNewFileFlags.push_back( false );
    
    // start off with no packet waiting
    clientPacketBytes.push_back( NULL );
    clientPacketSizes.push_back( 0 );
    clientPacketNumSent.push_back( 0 );

    clientIncomingPackets.push_back( new IncomingPacket() );
    
    
    return clientSocketConnections.size() - 1;
    }



void startClientOnFileIndex( int inClientIndex, int inFileIndex ) {

    if( inFileIndex < getStartOfCurrentChannel() ) {
        // push client forward so it can catch
        // up with our current channel
        inFileIndex = getStartOfCurrentChannel();
        }

    
    char *fileName = getFileName( inFileIndex );

    SentFileTracker *tracker =
        *( clientSentFileTrackers.getElement( inClientIndex ) );

    char alreadySent = tracker->sentFileYet( fileName );

    
    char returnToStartIndex = false;
    int startIndex = inFileIndex;

    
    while( alreadySent && !returnToStartIndex ) {

        delete [] fileName;

        inFileIndex ++;

        if( inFileIndex > getMaxFileIndex() ) {

            // loop back to start of current channel,
            // not all the way back to zero
            inFileIndex = getStartOfCurrentChannel();
            }

        if( inFileIndex != startIndex ) {
            fileName = getFileName( inFileIndex );

            alreadySent = tracker->sentFileYet( fileName );
            }
        else {
            returnToStartIndex = true;
            }
        }

    if( !alreadySent ) {
        FILE *file = fopen( fileName, "rb" );

        delete [] fileName;
    
        *( clientStreamFiles.getElement( inClientIndex ) ) = file;
        *( clientStreamStalled.getElement( inClientIndex ) )
            = false;
        *( clientStreamNewFileFlags.getElement( inClientIndex ) )
            = true;

        *( clientStreamFileIndices.getElement( inClientIndex ) ) = inFileIndex;
        }
    else {
        // exhausted all possible files to send

        // stall client
        *( clientStreamFiles.getElement( inClientIndex ) ) = NULL;

        *( clientStreamFileIndices.getElement( inClientIndex ) ) =
            getMaxFileIndex();
        
        *( clientStreamStalled.getElement( inClientIndex ) ) = true;

        printf( "Client %d stalled\n", inClientIndex );
        }
    }
 


IncomingPacket::IncomingPacket()
    : mType( NO_PACKET ),
      mSize( -1 ),
      mData( NULL ),
      mBytesReceived( 0 ),
      mNumSizeBytesReceived( 0 ) {

    }



IncomingPacket::~IncomingPacket() {
    if( mData != NULL ) {
        delete [] mData;
        mData = NULL;
        }
    }



char IncomingPacket::receiveMoreData( Socket *inSocket ) {

    if( mType == NO_PACKET ) {
        // receive packet type

        unsigned char typeByte;

        int numReceived = inSocket->receive( &typeByte, 1, 0 );
        if( numReceived == 1 ) {
            mType = typeByte;        
            }
        }

    switch( mType ) {
        case NO_PACKET:
            // still haven't received type byte
            return false;
            break;
        case STOP_CONTROL_PACKET:
        case PLAY_CONTROL_PACKET:
        case NO_CHANGE_CONTROL_PACKET:
            // a one-byte packet
            return true;
            break;
        default:
            break;
        }


    // if we made it through switch without returning, we are in the middle
    // of a data packet

    // do we have our four size bytes yet?
    if( mSize == -1 ) {
                
        mNumSizeBytesReceived = 
            socketReadToFillBuffer(
                inSocket,
                mSizeBytes,
                4,
                mNumSizeBytesReceived );
            
        if( mNumSizeBytesReceived == 4 ) {
            // now have all 4 bytes
                
            // pack bytes into int
            mSize =
                mSizeBytes[0] << 24 |
                mSizeBytes[1] << 16 |
                mSizeBytes[2] << 8  |
                mSizeBytes[3];
                
            // allocate buffer for packet
            mData =
                new unsigned char[ mSize ];
            }
        }
        
    if( mSize != -1 ) {
            
        // do we have all packet data yet?
        if( mBytesReceived
            < mSize ) {
                
            mBytesReceived = 
                socketReadToFillBuffer(
                    inSocket,
                    mData,
                    mSize,
                    mBytesReceived );
            }
            
        // now see if we are done
        if( mBytesReceived
            ==  mSize ) {

            return true;
            }
        }

    // if we got here, we haven't finished reading the data packet
    return false;
    }


