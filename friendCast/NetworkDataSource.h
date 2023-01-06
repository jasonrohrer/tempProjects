#include "minorGems/network/Socket.h"

#include "DataSource.h"



/**
 * Data source that reads from a network socket.
 */
class NetworkDataSource : public DataSource {

        
    public:


        
        /**
         * Constructs a data source.
         *
         * @param inSocket the socket to read from.
         *  Will be destroyed when this class is destroyed.
         */
        NetworkDataSource( Socket *inSocket );


        
        ~NetworkDataSource();

        
        
        // implements the DataSource interface
        virtual int readData( unsigned char *inBuffer,
                              int inDesiredByteCount );


        
    private:
        
        Socket *mSocket;

        int mBytesRemainingInCurrentPacket;

        char mFirstRead;

        int mBufferLength;
        int mBufferBytesReady;
        unsigned char *mBuffer;
        unsigned char *mWorkspaceBuffer;
        
        int readFromBuffer( unsigned char *inBufferToFill, int inNumBytes );

    };

