/*
 * Modification History
 *
 * 2002-May-5   Jason Rohrer
 * Created.
 * Changed to default to http:// if no URL type specified.
 * Added support for the Host: header.
 *
 * 2002-May-7   Jason Rohrer
 * Added support for pages that have been moved permanently.
 */


#include "WebClient.h"

#include "minorGems/util/log/AppLog.h"
#include "minorGems/util/stringUtils.h"
#include "minorGems/util/SimpleVector.h"


char *WebClient::getWebPage( char *inURL ) {

    char *returnString = NULL;
    
    
    char *startString = "http://";

    char *urlCopy = stringDuplicate( inURL );

    
    char *urlStart = strstr(  urlCopy, startString );

    char *serverStart;
    
    if( urlStart == NULL ) {
        // no http:// at start of URL
        serverStart = urlCopy;
        }
    else {
        serverStart = &( urlStart[ strlen( startString ) ] );
        }
    
    // find the first occurrence of "/", which is the end of the
    // server name

    char *serverNameCopy = stringDuplicate( serverStart );
        
    char *serverEnd = strstr( serverNameCopy, "/" );

    char *getPath = strstr( serverStart, "/" );

        
    if( serverEnd == NULL ) {
        serverEnd = &( serverStart[ strlen( serverStart ) ] );
        getPath = "/";
        }
    // terminate the url here to extract the server name
    serverEnd[0] = '\0';

    int portNumber = 80;

        // look for a port number
    char *colon = strstr( serverNameCopy, ":" );
    if( colon != NULL ) {
        char *portNumberString = &( colon[1] );
                
        int numRead = sscanf( portNumberString, "%d",
                              & portNumber );
        if( numRead != 1 ) {
            portNumber = 80;
            }

        // terminate the name here so port isn't taken as part
        // of the address
        colon[0] = '\0';
        }

    HostAddress *host = new HostAddress(
        stringDuplicate( serverNameCopy ),
        portNumber );
            
    Socket *sock = SocketClient::connectToServer( host );

    if( sock != NULL ) {
        SocketStream *stream = new SocketStream( sock );

        // send the request
        stream->writeString( "GET " );
        stream->writeString( getPath );
        stream->writeString( " HTTP/1.0\r\n" );
        stream->writeString( "Host: " );
        stream->writeString( serverNameCopy );
        stream->writeString( "\r\n\r\n" );


        // the simplest thing to do is to read upto the
        // socket close first, then extract the content
        char *received = receiveData( stream );

        char *content = NULL;
        
        // watch for redirection headers
        if( strstr( received, "302 Found" ) != NULL ||
            strstr( received, "301 Moved Permanently" ) != NULL ) {

            // call ourself recursively to fetch the redirection
            char *locationTag = "Location: ";
            char *locationTagStart = strstr( received, locationTag );

            if( locationTagStart != NULL ) {

                char *locationStart =
                    &( locationTagStart[ strlen( locationTag ) ] );

                // replace next \r with \0
                char *nextChar = locationStart;
                while( nextChar[0] != '\r' && nextChar[0] != '\0' ) {
                    nextChar = &( nextChar[1] );
                    }
                nextChar[0] = '\0';

                content = getWebPage( locationStart ); 
                }                        
            }

        char *contentStartString = "\r\n\r\n";
        if( content == NULL ) {
            // extract the content from what we've received
            char *contentStart = strstr( received, contentStartString );

            if( contentStart != NULL ) {
                content =
                    &( contentStart[ strlen( contentStartString ) ] );

                returnString = stringDuplicate( content );
                }
            }
        else {
            // we already obtained our content recursively
            returnString = stringDuplicate( content );
            delete [] content;
            }

        
        delete [] received;
                               
        delete stream;
        delete sock;
        }

    delete host;

        
    delete [] serverNameCopy;

    delete [] urlCopy;


    
    return returnString;
    }



char *WebClient::receiveData( SocketStream *inSocketStream ) {

    SimpleVector<char> *receivedVector =
            new SimpleVector<char>();

    char connectionBroken = false;
    long bufferLength = 5000;
    unsigned char *buffer = new unsigned char[ bufferLength ];

    while( !connectionBroken ) {

        int numRead = inSocketStream->read( buffer, bufferLength );

        if( numRead != bufferLength ) {
            connectionBroken = true;
            }

        if( numRead > 0 ) {
            for( int i=0; i<numRead; i++ ) {
                receivedVector->push_back( buffer[i] );
                }
            }
        }

    delete [] buffer;

    // copy our vector into an array
    int receivedSize = receivedVector->size();
    char *received = new char[ receivedSize + 1 ];
        
    int i;
    for( int i=0; i<receivedSize; i++ ) {
        received[i] = *( receivedVector->getElement( i ) );
        }
    received[ receivedSize ] = '\0';
                
    delete receivedVector;

    
    return received;
    }
