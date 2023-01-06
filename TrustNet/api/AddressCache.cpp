/*
 * Modification History
 *
 * 2004-November-8   Jason Rohrer
 * Created.
 */



#include "Primrose/api/AddressCache.h"



AddressCache::AddressCache()
    : mLock( new MutexLock() ),
      mAddresses( new SimpleVector<AddressAndUser *>() ),
      mMaxCacheSize( 100000 ) {

    }



AddressCache::~AddressCache() {
    mLock->lock();

    unsigned long numAddresses = mAddresses->size();

    for( unsigned long i=0; i<numAddresses; i++ ) {
        delete *( mAddresses->getElement( i ) );
        }
    delete mAddresses;

    mLock->unlock();

    delete mLock;
    }


void AddressCache::addAddress( struct UDPAddress *inAddress,
                               struct UserID *inUserID ) {

    AddressAndUser *addressToAdd =
        new AddressAndUser( SocketUDP::copy( inAddress ),
                            copy( inUserID ) );

    mLock->lock();

    // delete any existing copy of this address
    unsigned long cacheSize = mAddresses->size();
    char found = false;
    
    for( unsigned long i=0; i<cacheSize && !found; i++ ) {
        AddressAndUser *otherAddress = *( mAddresses->getElement( i ) );

        if( addressToAdd->compare( otherAddress ) ) {
            // match
            delete otherAddress;
            mAddresses->deleteElement( i );
            found = true;
            }
        }
    
    
    mAddresses->push_back( addressToAdd );

    // delete the oldest address if we are overflowing
    if( (unsigned long)( mAddresses->size() ) > mMaxCacheSize ) {
        delete *( mAddresses->getElement( 0 ) );
        mAddresses->deleteElement( 0 );
        }
    
    mLock->unlock();
    }



AddressAndUser **AddressCache::getAddresses( int inNumDesired,
                                             int *outNumReturned ) {


    // start at the end of cache so that the freshest addresses are used first

    // build a vector of addresses to return
    SimpleVector<AddressAndUser *> *returnVector =
        new SimpleVector<AddressAndUser *>();

    

    mLock->lock();

    // start at end
    int cacheIndex = mAddresses->size() - 1;

    // stop when we have added inNumDesired or when we exhaust the cache
    while( returnVector->size() < inNumDesired &&
           cacheIndex >= 0 ) {

        AddressAndUser *nextAddress =
            *( mAddresses->getElement( cacheIndex ) );

        // add a copy to our return vector
        returnVector->push_back( new AddressAndUser( nextAddress ) );

        cacheIndex--;        
        }
    
    mLock->unlock();


    AddressAndUser **returnArray = returnVector->getElementArray();
    *outNumReturned = returnVector->size();
    
    delete returnVector;

    
    return returnArray;
    }



unsigned long AddressCache::getCacheSize() {

    mLock->lock();

    unsigned long cacheSize = mAddresses->size();

    mLock->unlock();

    return cacheSize;
    }


