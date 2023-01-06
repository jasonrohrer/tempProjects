#include "playFromDataSource.h"


#include <inttypes.h>

extern "C" {
#include "mpeg2.h"
#include "mpeg2convert.h"    
}

#include "minorGems/graphics/ScreenGraphics.h"
#include "minorGems/graphics/GraphicBuffer.h"
#include "minorGems/util/SimpleVector.h"

#include "minorGems/network/Socket.h"
#include "minorGems/network/SocketServer.h"



#define BUFFER_SIZE 4096


int width = -1;
int height = -1;
ScreenGraphics *screen = NULL;
unsigned long *screenData = NULL;
GraphicBuffer *graphics = NULL;

SimpleVector<Socket *> clientSockets;


extern char quitFlag;



void playFromDataSource( DataSource *inSource, int inServerPortNumber ) {

    // listen on port inServerPortNumber, 100 connections queued
    SocketServer server( inServerPortNumber, 100 );
    
    uint8_t buffer[ BUFFER_SIZE ];

    // A copy of the buffer that contained the most recent sequence header.
    // Always send this first to a newly-connected client to get them started.
    uint8_t sequenceHeaderBuffer[ BUFFER_SIZE ];
    
    mpeg2dec_t * decoder;
    const mpeg2_info_t * info;
    const mpeg2_sequence_t * sequence;
    mpeg2_state_t state;
    size_t size;
    int framenum = 0;

    decoder = mpeg2_init ();
    if( decoder == NULL ) {
        fprintf( stderr, "Could not allocate a decoder object.\n" );
        exit( 1 );
        }
    info = mpeg2_info( decoder );

    sequence = info->sequence;

    
    
    size = (size_t)-1;
    do {
        state = mpeg2_parse( decoder );
        sequence = info->sequence;
        switch( state ) {
            case STATE_BUFFER: {

                // check for new incoming connections here

                // use timeout of 0 to return immediately if no
                // connection is in the queue
                Socket *sock = server.acceptConnection( 0 );

                if( sock != NULL ) {
                    // add it
                    clientSockets.push_back( sock );

                    // send our first buffer to get them started
                    sock->send( sequenceHeaderBuffer, BUFFER_SIZE );
                    }

        
                // read a new buffer of data from the file
                size = inSource->readData( buffer, BUFFER_SIZE );

                if( size != -1 ) {
                    // feed the buffer to each client
                    int numClients = clientSockets.size();
                    for( int i=0; i<numClients; i++ ) {
                        Socket *sock = *( clientSockets.getElement( i ) );
                        sock->send( buffer, size );
                        }
                    
                    // now decode the buffer locally for display
                    mpeg2_buffer( decoder, buffer, buffer + size );
                    }
                }
            break;
            case STATE_SEQUENCE:
                mpeg2_convert( decoder, mpeg2convert_rgb32, NULL );

                // the current buffer contains a sequence header
                // save it
                memcpy( sequenceHeaderBuffer, buffer, BUFFER_SIZE );
                break;
            case STATE_SLICE:
            case STATE_END:
            case STATE_INVALID_END:
                if( info->display_fbuf ) {

                    if( screen == NULL ) {
                        width = sequence->width;
                        height = sequence->height;

                        screen = new ScreenGraphics( width, height );

                        graphics = screen->getScreenBuffer();
                        screenData = graphics->getBuffer();
                        }
                    else if( width != sequence->width ||
                             height != sequence->height ) {

                        // change in image size
                
                        delete screen;
                        delete graphics;

                        width = sequence->width;
                        height = sequence->height;

                        screen = new ScreenGraphics( width, height );

                        graphics = screen->getScreenBuffer();
                        screenData = graphics->getBuffer();
                        }
            

                    // copy fbuf, which has blocks of 4 bytes per pixel, into
                    // the unsigned long screen data

                    int numPixels = width * height;
                    unsigned char *buffer =
                        (unsigned char *)( info->display_fbuf->buf[0] );

                    memcpy( screenData, buffer, 4 * numPixels );
            
                    screen->flipScreen();
                    }
                break;
            default:
                break;
            }
        } while( size != -1 && ! quitFlag );

    mpeg2_close( decoder );
    }
