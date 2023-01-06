/*
 * This file based on sample1.c from the libmpeg2 (mpeg2dec) package.
 *
 * Reads an mpeg file in and displays it to a ScreenGraphics.
 */

#include <stdio.h>
#include <stdlib.h>

#include <signal.h>

#include "playVideoFile.h"
#include "streamFileTracker.h"
#include "streamNetwork.h"


int serverPort = 5000;


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

    printf( "   playFromFile our_port_number file1 file2 file3 ...\n\n" );

    exit( 0 );
    }



#include "SDL/SDL.h"
int main( int argc, char ** argv ) {

    // let catch_int handle interrupt (^c)
    signal( SIGINT, catch_int );


    // we are streaming from local files
    setLocalFileSetStreaming( true );

    if( argc < 3 ) {
        usage();
        }
    

    // first arg is our port number
    int numRead = sscanf( argv[1], "%d", &serverPort );
    if( numRead != 1 ) {
        usage();
        }
    
    
    // each arg is a file name

    int numFiles = argc - 2;

    char **mpgFiles = new char*[ numFiles ];
    
    for( int i=2; i<argc; i++ ) {

        int index = i-2;

        mpgFiles[ index ] = argv[i];

        // add name to tracker
        addFileName( mpgFiles[ index ] );
        }


    printf( "Starting stream network listening on port %d\n", serverPort );
    startNetwork( serverPort );
    
    
    int fileIndex = 0;
    
    // keep going until quit flag
    while( !quitFlag ) {

        channelChangeFlag = false;
        
        char *fileName = getFileName( getCurrentFileIndex() );
        
        // returns on format change or quit flag
        playVideoFile( fileName );

        delete [] fileName;
        
        stepCurrentIndexToNextFile();
        }

    delete [] mpgFiles;

    printf( "Stopping stream network.\n" );
    stopNetwork();
    
    return 0;
    }
