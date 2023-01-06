/*
 * Modification History
 *
 * 2004-November-7   Jason Rohrer
 * Created.
 *
 * 2004-November-12   Jason Rohrer
 * Removed unused MessagFilter support.
 */



#ifndef PRIMROSE_MESSAGE_MANAGER_INCLUDED
#define PRIMROSE_MESSAGE_MANAGER_INCLUDED



#include "Primrose/api/primroseAPI.h"

#include "minorGems/util/SimpleVector.h"
#include "minorGems/system/MutexLock.h"
#include "minorGems/system/StopSignalThread.h"

#include "minorGems/network/SocketUDP.h"


#include <stdio.h>



/**
 * Wrapper class that encapsulates a handler function and its extra
 * argument.
 */
class MessageHandlerFunctionWrapper {
    public:

        int mID;
        
        void (*mHandlerFunction)( struct UDPAddress *, struct UserID *,
                                  char *, void * );

        void *mExtraHandlerArgument;
    };



/**
 * Class that manages incoming and outgoing UDP messages.
 *
 * This class verifies signatures on incoming messages and signs outgoing
 * messages.  Incoming messages with bad signatures are dropped.
 *
 * @author Jason Rohrer.
 */
class MessageManager : public StopSignalThread {

    public:

        /**
         * Constructs a manager and starts it.
         *
         * @parm inPort the port to receive UDP messages on.
         */
        MessageManager( unsigned short inPort );


        
        /**
         * Stops and destroys this manager.
         */
        ~MessageManager();

        

        /**
         * Adds a handler for incoming messages.
         *
         * All verified messages  will be passed to all registered handlers.
         *
         * All parameters passed in to handler will be destroyed by caller and
         * therefore should not be destroyed by the handler.
         *
         * The extra (void *) argument can be used to encapsulate any
         * additional state that should be associated with a particular
         * handler.  For example, it could be a pointer to a C++ object that
         * is "unwrapped" by casting inside the handler function.
         *
         * @param inHandlerFunction pointer to the handler function.
         *   This function must return void and take the following arguments:
         *   (UDPAddress *inNodeAddress, struct UserID *inUserID,
         *    char *inMessageText, void *inExtraArgument ).
         * @param inExtraHandlerArgument pointer to an extra argument to be
         *   passed in to the handler function each time it is called.
         *   Must be destroyed by caller after the handler is removed.
         *
         * @return an ID for this handler that can be used to remove it later.
         */
        int addMessageHandler(
            void (*inHandlerFunction)( struct UDPAddress *, struct UserID *,
                                       char *, void * ),
            void *inExtraHandlerArgument );

        
        
        /**
         * Removes a message handler.
         *
         * @param inHandlerID the ID of the handler to remove.
         */
        void removeMessageHandler( int inHandlerID );

        
        
        /**
         * Signs a message with our key and sends it.
         *
         * @param inMessageText the text portion of the message.
         *   Must be destroyed by caller.
         * @param inDestinationID the user ID of destination node.
         *   Must be destroyed by caller.
         * @param inDestinationAddress the address of the destination node.
         *   Must be destroyed by caller.
         */
        void sendMessage( char *inMessageText,
                          struct UserID *inDestinationID,
                          struct UDPAddress *inDestinationAddress );


        
        // implements the Thread interface
        void run();

        
        
    private:
        
        SocketUDP *mSocket;

        MutexLock *mLock;
        SimpleVector<MessageHandlerFunctionWrapper *> *mHandlers;

        int mNextFreeID;
        
    };


#endif
