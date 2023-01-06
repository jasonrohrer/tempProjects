/**
 * Modification History
 *
 * 2003-January-10   Jason Rohrer
 * Created.
 * Changed to use -remote flag when invoking browser.
 * Added a thread that flips link list.
 */



#include <unistd.h>
#include <stdio.h>



#include "minorGems/network/Socket.h"
#include "minorGems/network/SocketServer.h"
#include "minorGems/network/SocketStream.h"

#include "minorGems/util/stringUtils.h"
#include "minorGems/system/Launcher.h"

#include "minorGems/system/Thread.h"


char **linkNames;
char **linkURLs;
SocketStream *sockStream;




class SenderThread : public Thread {
    public:
        void run() {

            int flip = 0;
            
            while( true ) {
                sockStream->writeLong( 5 );

                unsigned char* stringTermination = new unsigned char[1];
                stringTermination[0] = '\0';

                for( int i=0; i<5; i++ ) {
                    // title and URL for each
                    sockStream->writeString( linkNames[ i + 5 * flip] );
                    sockStream->write( stringTermination, 1 );

                    sockStream->writeString( linkURLs[ i + 5 * flip] );
                    sockStream->write( stringTermination, 1 );
                    
                    }

                delete [] stringTermination;
                
                sleep( 5000 );

                flip = !flip;
                }
            
            }
        
    };



int main() {

    linkNames = new char*[10];
    linkURLs  = new char*[10];

    linkNames[0] = "Google";
    linkNames[1] = "Yahoo";
    linkNames[2] = "Slashdot";
    linkNames[3] = "Sun";
    linkNames[4] = "UCSC";
    linkNames[5] = "Stanford";
    linkNames[6] = "MIT";
    linkNames[7] = "Microsoft";
    linkNames[8] = "Ebay";
    linkNames[9] = "Amazon";

    linkURLs[0] = "http://www.google.com";
    linkURLs[1] = "http://www.yahoo.com";
    linkURLs[2] = "http://www.slashdot.org";
    linkURLs[3] = "http://www.sun.com";
    linkURLs[4] = "http://www.ucsc.edu";
    linkURLs[5] = "http://www.stanford.edu";
    linkURLs[6] = "http://www.mit.edu";
    linkURLs[7] = "http://www.microsoft.com";
    linkURLs[8] = "http://www.ebay.com";
    linkURLs[9] = "http://www.amazon.com";

    int currentIndex = 0;

    
    int port = 9578;


    SocketServer *server = new SocketServer( port, 100 );

    // wait for only one connection
    printf( "Waiting for connection on port %d\n", port );
    Socket *sock = server->acceptConnection();


    if( sock != NULL ) {

        sockStream = new SocketStream( sock );
                
        // start our link writing thread
        SenderThread *thread = new SenderThread();
        thread->start();
        
        // now repeatedly receive link clicks and open them
        int bufferLength = 300;
        char *stringBuffer = new char[ bufferLength ];
        unsigned char charRead;
            
        char noError = true;

        while( noError ) {

            char **readStrings = new char*[2];

            readStrings[0] = NULL;
            readStrings[1] = NULL;
            
            for( int i=0; i<2 && noError; i++ ) {
            
                int numRead = sockStream->readByte( &charRead );
                
                int index = 0;
                while( numRead == 1 &&
                       charRead != '\0' &&
                       index < bufferLength ) {
                    
                    stringBuffer[ index ] = charRead;

                    index++;
                    numRead = sockStream->readByte( &charRead );
                }
                
                if( numRead == 1 &&
                    charRead == '\0' &&
                    index < bufferLength ) {
                    // hit end, so terminate the string
                    stringBuffer[ index ] = '\0';

                    readStrings[i] = stringDuplicate( stringBuffer );
                    }
                else {
                    // we didn't receive the end of the string
                    // there must be a problem
                    noError = false;
                    }
                }

            if( noError ) {
                // first string is browser name
                // second is URL

                printf( "Launching %s with URL %s\n",
                        readStrings[0], readStrings[1] );

                // fork a process to launch browser

                char **args = new char*[4];
                args[0] = readStrings[0];
                args[1] = "-remote";
                args[2] = new char[ strlen( readStrings[1] ) + 20 ];
                sprintf( args[2], "openURL(%s)", readStrings[1] );
                args[3] = NULL;
                
                Launcher::launchCommand( readStrings[0], args );

                delete [] args[2];
                delete [] args;
                
                }

            if( readStrings[0] != NULL ) {
                delete [] readStrings[0];
                }
            if( readStrings[1] != NULL ) {
                delete [] readStrings[1];
                }

            delete [] readStrings;
            }

        printf( "Connection broken\n" );

        delete [] stringBuffer;
        delete [] linkNames;
        delete [] linkURLs;
        
        delete sockStream;
        delete sock;
        }
    else {
        printf( "Failed to receive a connection\n" );
        }
    

    delete server;
    
    return 0;
    }



