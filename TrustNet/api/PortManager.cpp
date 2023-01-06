/*
 * Modification History
 *
 * 2004-November-11   Jason Rohrer
 * Created.
 */



#include "Primrose/api/PortManager.h"



PortManager::PortManager( unsigned short inLowPort,
                          unsigned short inHighPort )
    : mLowPort( inLowPort ), mHighPort( inHighPort ),
      mLock( new MutexLock() ),
      mPortTakenFlags( new char[ inHighPort - inLowPort + 1 ] ) {

    }



PortManager::~PortManager() {
    delete [] mPortTakenFlags;
    delete mLock;
    }



unsigned short PortManager::getPort() {
    mLock->lock();

    char found = false;
    unsigned short freePort = 0;

    for( int i=mLowPort; i<=mHighPort && !found; i++ ) {

        if( mPortTakenFlags[ i - mLowPort ] == false ) {
            mPortTakenFlags[ i - mLowPort ] = true;
            found = true;
            freePort = i;
            }
        }
    
    mLock->unlock();

    return freePort;
    }



void PortManager::freePort( unsigned short inPort ) {
    mLock->lock();

    mPortTakenFlags[ inPort - mLowPort ] = false;
    
    mLock->unlock();
    }
