/*
 * Modification History
 *
 * 2004-November-10   Jason Rohrer
 * Created.
 */



#ifndef PRIMROSE_PORT_MANAGER_INCLUDED
#define PRIMROSE_PORT_MANAGER_INCLUDED


#include "Primrose/api/primroseAPI.h"

#include "minorGems/system/MutexLock.h"



/**
 * Class that manages a range of ports.
 *
 * @author Jason Rohrer.
 */
class PortManager {

    public:

        /**
         * Constructs a manager.
         *
         * @param inLowPort the low end of the port range.
         * @param inHighPort the high end of the port range.
         */
        PortManager( unsigned short inLowPort, unsigned short inHighPort );

        ~PortManager();


        
        /**
         * Gets an unused port.
         *
         * @param an unused port, or 0 if there are no free ports left.
         */
        unsigned short getPort();

        

        /**
         * Frees a port.
         *
         * @param inPort the port to free.
         */
        void freePort( unsigned short inPort );


    private:

        unsigned short mLowPort;
        unsigned short mHighPort;
        
        MutexLock *mLock;
        char *mPortTakenFlags;
        
    };


#endif
