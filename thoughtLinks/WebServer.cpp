/*
 * Modification History
 *
 * 2002-March-12   Jason Rohrer
 * Created.
 *
 * 2002-April-5    Jason Rohrer
 * Changed to extend StopSignalThread.
 * Added timeouts to socket accept to support checking for stop signal.
 */


#include "WebServer.h"


#include "minorGems/util/log/AppLog.h"



WebServer::WebServer( int inPort, PageGenerator *inGenerator )
    : mPortNumber( inPort ), mMaxQueuedConnections( 100 ),
      mPageGenerator( inGenerator ),
      mThreadHandler( new ThreadHandlingThread() )  {

    
    mServer = new SocketServer( mPortNumber, mMaxQueuedConnections );

    this->start();
    }



WebServer::~WebServer() {
    stop();
    join();

    delete mServer;
    
    delete mThreadHandler;
    
    delete mPageGenerator;
    }



void WebServer::run() {

    
    char *logMessage = new char[100];
    
    sprintf( logMessage, "Listening for connections on port %d\n",
            mPortNumber );

    AppLog::info( "WebServer", logMessage );

    delete [] logMessage;


    char acceptFailed = false;
    
    
    // main server loop
    while( !isStopped() && !acceptFailed ) {
        
        char timedOut = true;
        
        // 5 seconds
        long timeout = 5000;
    
        Socket *sock;

        AppLog::info( "WebServer", "Waiting for connection." );
        while( timedOut && !isStopped() ) {
            sock = mServer->acceptConnection( timeout, &timedOut );
            }

        
        if( sock != NULL ) {
        
            AppLog::info( "WebServer", "Connection received." );
        
            RequestHandlingThread *thread =
                new RequestHandlingThread( sock, mPageGenerator );
            
            thread->start();
            
            mThreadHandler->addThread( thread );
            }
        else if( isStopped() ) {
            AppLog::info( "WebServer", "Received stop signal." );
            }
        else {
            AppLog::error( "WebServer", "Accepting a connection failed." );
            acceptFailed = true;
            }
        }
    
    }
