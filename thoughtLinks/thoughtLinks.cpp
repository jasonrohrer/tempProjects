/*
 * Modification History
 *
 * 2002-May-2   Jason Rohrer
 * Created.
 */


#include "WebServer.h"
#include "ThoughtLinksPageGenerator.h"


#include "minorGems/util/log/AppLog.h"
#include "minorGems/util/log/FileLog.h"
#include "minorGems/system/MutexLock.h"
#include "minorGems/system/Thread.h"

#include "minorGems/util/printUtils.h"



#include <stdio.h>
#include <stdlib.h>
#include <signal.h>



void usage( char *inAppName );



int main( char inNumArgs, char **inArgs ) {

    
    if( inNumArgs < 2 || inNumArgs > 3 ) {
        usage( inArgs[0] );
        }

    int webPort;
    int numRead = sscanf( inArgs[1], "%d", &webPort );


    if( numRead != 1 ) {
        threadPrintF( "Invalid web port number:  %s\n", inArgs[1] );
        usage( inArgs[0] );
        }

    
    if( inNumArgs == 3 ) {
        AppLog::setLog( new FileLog( inArgs[2] ) );
        }
    else {
        AppLog::setLog( new FileLog( "thoughtLinks.log" ) );
        }


    AppLog::setLoggingLevel( Log::TRACE_LEVEL );
    
    WebServer *web = new WebServer( webPort,
                                    new ThoughtLinksPageGenerator() );


    
    threadPrintF( "Enter q to quit nicely\n" );

    char readChar = 'a';
    while( readChar != 'q' && readChar != 'Q' ) {
        scanf( "%c", &readChar );
        }
    threadPrintF( "Halting...\n" );

    
    delete web;

    threadPrintF( "Done.\n" );
        
    
    return 0;
    }



void usage( char *inAppName ) {

	printf( "Usage:\n" );
	printf( "\t%s web_port [log_file]\n", inAppName );

	printf( "Examples:\n" );
	printf( "\t%s 8080 \n", inAppName );

    
	printf( "\t%s 8080 thoughtLinks.log\n", inAppName );
	
	exit( 1 );
	}


