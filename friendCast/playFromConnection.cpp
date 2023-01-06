/*
 * This file based on sample1.c from the libmpeg2 (mpeg2dec) package.
 *
 * Reads an mpeg stream from a socket connection in and displays it to a
 * ScreenGraphics.
 */

#include <stdio.h>
#include <stdlib.h>

#include <signal.h>


#include "minorGems/network/Socket.h"
#include "minorGems/network/SocketClient.h"


#include "playVideoFile.h"
#include "streamNetwork.h"
#include "streamFileTracker.h"


int serverPort = 5001;



char quitFlag = false;
void catch_int(int sig_num) {
	printf( "Quiting..." );
	quitFlag = true;
	signal( SIGINT, catch_int );
	}


// set to true by playVideoFile if we are changing channels
char channelChangeFlag = false;



void usage() {

    printf( "Usage:\n\n" );

    printf( "   playFromConnection our_port_number "
            "address_1 port_1 address_2 port_2 ...\n\n" );

    exit( 0 );
    }



#include "SDL/SDL.h"

int main( int argc, char ** argv ) {

    // let catch_int handle interrupt (^c)
    signal( SIGINT, catch_int );


    if( argc < 4 ) {
        usage();
        }

    // first arg is our port number
    int numRead = sscanf( argv[1], "%d", &serverPort );
    if( numRead != 1 ) {
        usage();
        }

    
    
    // arg 2 is server address
    // arg 3 is server port


    int port;
    sscanf( argv[3], "%d", &port );

    printf( "Trying to connect to %s:%d\n", argv[2], port );
        
    HostAddress *address = new HostAddress( stringDuplicate( argv[2] ),
                                            port );
        
    Socket *sock = SocketClient::connectToServer( address, 10000 );

    delete address;
        
    if( sock == NULL ) {
        printf( "Failed to connect to node:  %s %d\n", argv[2], port );
            }
    else {
        printf( "Connected\n" );
        
        // set socket in streamNetwork
        setSourceSocket( sock );
        
        // non-local stream of files
        setLocalFileSetStreaming( false );
        

        // now read additional args and connect to other nodes
        int i = 4;
        while( i + 1 < argc ) {
            // another pair of args

            sscanf( argv[i+1], "%d", &port );
                
            printf( "Trying to connect to %s:%d\n", argv[i], port );

            address = new HostAddress(
                stringDuplicate( argv[i] ), port );
        
            sock = SocketClient::connectToServer( address, 10000 );

            delete address;
                
            if( sock == NULL ) {
                printf( "Failed to connect to node:  %s %d\n",
                        argv[i], port );
                }
            else {
                printf( "Connected\n" );
            
                // set socket in streamNetwork
                addClientSocket( sock );
                }

            i += 2;
            }


        printf( "Starting stream network listening on port %d\n", serverPort );
        startNetwork( serverPort );
        
            
            
        // wait for first file to start coming in
        printf( "Waiting for first channel file to start streaming...\n" );
        while( !quitFlag && getCurrentFileIndex() == -1 ) {
            executeNetworkStep( 250 ); // spend quarter second      
            }
            
        // keep going until quit flag
        while( !quitFlag ) {

            if( channelChangeFlag ) {
                channelChangeFlag = false;

                // new file will be appended to end of our stream
                // file list

                // the last file on the previous channel
                int oldMaxIndex = getMaxFileIndex();
                    
                    
                // change channel in network layer
                nextChannel();
                    
                    
                // wait for first file to start coming in
                printf( "Waiting for first file to start streaming...\n" );
                while( !quitFlag && getMaxFileIndex() == oldMaxIndex ) {
                    executeNetworkStep( 250 );  // spend quarter second
                    }

                if( !quitFlag ) {

                    // jump to first file on new channel
                    jumpCurrentIndexToEnd();

                    setStartOfCurrentChannel( getCurrentFileIndex() );
                    }
                }

                
            if( !quitFlag ) {

                // play file from our current channel
                char *fileName = getFileName( getCurrentFileIndex() );

                printf( "Playing from received file %s\n", fileName );
                    
                // returns on format change or quit flag
                playVideoFile( fileName );
                    
                delete [] fileName;
                
                stepCurrentIndexToNextFile();
                }
            }
        }

    printf( "Stopping stream network.\n" );
    stopNetwork();
    
    return 0;
    }
