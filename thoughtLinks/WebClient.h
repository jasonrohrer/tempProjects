/*
 * Modification History
 *
 * 2002-May-5   Jason Rohrer
 * Created.
 * Added a utility function for receiving data.
 */


 
#ifndef WEB_CLIENT_INCLUDED
#define WEB_CLIENT_INCLUDED 



#include "minorGems/network/HostAddress.h"
#include "minorGems/network/Socket.h"
#include "minorGems/network/SocketClient.h"
#include "minorGems/network/SocketStream.h"


#include <string.h>
#include <stdio.h>


/**
 * A class that implements a basic web client.
 *
 * @author Jason Rohrer.
 */
class WebClient {



    public:


        
        /**
         * Gets a web page.
         *
         * @param inURL the URL to get as a \0-terminated string.
         *   Must be destroyed by caller if non-const.
         *
         * @return the fetched web page as a \0-terminated string,
         *   or NULL if fetching the page fails.
         *   Must be destroyed by caller if non-NULL.
         */
        static char *getWebPage( char *inURL );



    protected:


        
        /**
         * Receives data on a connection until the connection is closed.
         *
         * @param inSocketStream the stream to read from.
         *   Must be destroyed by caller.
         *
         * @return the received data as a \0-terminated string.
         *   Must be destroyed by caller.
         */
        static char *receiveData( SocketStream *inSocketStream );

        
        
    };




#endif
