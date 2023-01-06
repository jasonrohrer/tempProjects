/*
 * Modification History
 *
 * 2004-November-13   Jason Rohrer
 * Created.
 */



#include "Primrose/api/MessageManager.h"
#include "Primrose/api/primroseAPI_internal.h"


#include "minorGems/util/log/AppLog.h"
#include "minorGems/util/stringUtils.h"



MessageManager::MessageManager( unsigned short inPort )
    : mSocket( new SocketUDP( inPort ) ),
      mLock( new MutexLock() ),
      mHandlers( new SimpleVector<MessageHandlerFunctionWrapper *>() ),
      mNextFreeID( 0 ) {

    this->start();
    }



MessageManager::~MessageManager() {

    // stop this manager's thread
    stop();
    join();
    
    
    mLock->lock();

    for( int i=0; i<mHandlers->size(); i++ ) {
        delete *( mHandlers->getElement( i ) );
        }
    delete mHandlers;

    mLock->unlock();

    
    delete mSocket;

    delete mLock;
    }

    


int MessageManager::addMessageHandler(
    void (*inHandlerFunction)( struct UDPAddress *, struct UserID *,
                               char *, void * ),
    void *inExtraHandlerArgument ) {

    MessageHandlerFunctionWrapper *wrapper =
        new MessageHandlerFunctionWrapper();

    wrapper->mHandlerFunction = inHandlerFunction;
    wrapper->mExtraHandlerArgument = inExtraHandlerArgument;


    mLock->lock();

    int id = mNextFreeID;
    mNextFreeID++;
    
    wrapper->mID = id;
    
    mHandlers->push_back( wrapper );

    mLock->unlock();

    return id;
    }



void MessageManager::removeMessageHandler( int inHandlerID ) {

    mLock->lock();

    char found = false;
    for( int i=0; i<mHandlers->size() && !found; i++ ) {

        MessageHandlerFunctionWrapper *currentWrapper =
            *( mHandlers->getElement( i ) );

        if( currentWrapper->mID == inHandlerID ) {
            found = true;
            delete currentWrapper;
            mHandlers->deleteElement( i );
            }
        }

    mLock->unlock();
    }



void MessageManager::sendMessage( char *inMessageText,
                                  struct UserID *inDestinationID,
                                  struct UDPAddress *inDestinationAddress ) {

    unsigned long signatureLength;
    unsigned char *signatureBlock =
        primrose_internalGetBinarySignatureBlock(
            inMessageText,
            NULL, 0,
            inDestinationID,
            &signatureLength );

    if( signatureBlock != NULL ) {

        // full message is text part, followed by \0, followed by signature

        // length of text plus \0 termintator
        int textBlockLength = strlen( inMessageText ) + 1; 

        int fullMessageLength = textBlockLength + signatureLength; 
        unsigned char *fullMessage =
            new unsigned char[ fullMessageLength ];

        
        memcpy( fullMessage, inMessageText, textBlockLength );

        // skip text block and copy in signature block
        memcpy( &( fullMessage[ textBlockLength ] ),
                signatureBlock, signatureLength );

        delete [] signatureBlock;


        int numSent = mSocket->send( inDestinationAddress,
                                     fullMessage, fullMessageLength );
        delete [] fullMessage;
        
        if( numSent != fullMessageLength ) {
            AppLog::error( "MessageManager", "Failed to send a UDP message" );
            }
        else {
            unsigned short destinationPort;
            char *desinationIP =
                SocketUDP::extractAddress( inDestinationAddress,
                                           &destinationPort );
            
            char *logMessage =
                autoSprintf( "Sent message to (%lu,%lu) @ %s:%hu, text:\n%s",
                             inDestinationID->partA, inDestinationID->partB,
                             desinationIP, destinationPort,
                             inMessageText );
            delete [] desinationIP;
            
            AppLog::trace( "MessageManager", logMessage );
            delete [] logMessage;
            }
        }
    else {
        AppLog::error( "MessageManager", "Failed to sign message" );
        }
    }



void MessageManager::run() {
    while( ! isStopped() ) {
        // read a message from the socket

        unsigned char *message;
        struct UDPAddress *senderAddress;
        
        int numReceived =
            mSocket->receive( &senderAddress,
                              &message,                     
                              5000 );   // 5 second timeout

        if( numReceived >= 0 ) {
            // no error or timeout

            // first part of message terminated with \0 so it can
            // be treated as a string
            char *textPart = (char *)message;

            // skip the text part and its \0 terminator
            unsigned char *signatureBlock =
                &( message[ strlen( textPart ) + 1 ] );

            struct UserID *senderID =
                primrose_internalCheckSignatureAndGetSenderID(
                    textPart,
                    NULL, 0,
                    signatureBlock,
                    numReceived - strlen( textPart ) - 1 );

            if( senderID != NULL ) {
                // signature valid

                unsigned short senderPort;
                char *senderIP =
                    SocketUDP::extractAddress( senderAddress,
                                               &senderPort );
                
                char *logMessage =
                    autoSprintf( "Got valid message from "
                                 "(%lu,%lu) @ %s:%hu, text:\n%s",
                                 senderID->partA, senderID->partB,
                                 senderIP, senderPort,
                                 textPart );
                delete [] senderIP;
                
                AppLog::trace( "MessageManager", logMessage );
                delete [] logMessage;
                
                // pass message to each handler

                mLock->lock();
                for( int i=0; i<mHandlers->size(); i++ ) {

                    MessageHandlerFunctionWrapper *currentWrapper =
                        *( mHandlers->getElement( i ) );

                    currentWrapper->mHandlerFunction(
                        senderAddress,
                        senderID,
                        textPart,
                        currentWrapper->mExtraHandlerArgument );
                    }

                mLock->unlock();

                delete senderID;
                }
            else {
                char *logMessage =
                    autoSprintf( "Signature failed for message, text:\n%s",
                                 textPart );

                AppLog::detail( "MessageManager", logMessage );
                delete [] logMessage;
                }

            
            delete [] message;
            delete senderAddress;
            }
        

        }
    }
