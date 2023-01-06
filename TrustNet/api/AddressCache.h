/*
 * Modification History
 *
 * 2004-November-8   Jason Rohrer
 * Created.
 */



#ifndef PRIMROSE_ADDRESS_CACHE_INCLUDED
#define PRIMROSE_ADDRESS_CACHE_INCLUDED


#include "Primrose/api/primroseAPI.h"

#include "minorGems/network/SocketUDP.h"
#include "minorGems/system/MutexLock.h"
#include "minorGems/util/SimpleVector.h"



/**
 * Container class for address/user pair.
 *
 * @author Jason Rohrer.
 */
class AddressAndUser {

    public:
        
        /**
         * Constructs a pair.
         *
         * @param inUDPAddress the address.
         *   Will be destroyed when this class is destroyed.
         * @param inUserID the user.
         *   Will be destroyed when this class is destroyed.
         */
        AddressAndUser( struct UDPAddress *inUDPAddress,
                        struct UserID *inUserID )
            : mUDPAddress( inUDPAddress ), mUserID( inUserID ) {

            }


        
        /**
         * Constructs a pair, making a deep copy of another pair.
         *
         * @param inOtherPair the pair to copy.
         *   Must be destroyed by caller.
         */
        AddressAndUser( AddressAndUser *inOtherPair )
            : mUDPAddress( SocketUDP::copy( inOtherPair->mUDPAddress ) ),
              mUserID( copy( inOtherPair->mUserID ) ) {

            }

        
        
        ~AddressAndUser() {
            delete mUDPAddress;
            delete mUserID;
            }

        

        /**
         * Compares this pair with another pair.
         *
         * @param inOtherPair the pair to compare this pair with.
         *   Must be destroyed by caller.
         */
         char compare( AddressAndUser *inOtherPair ) {
            if( SocketUDP::compare( inOtherPair->mUDPAddress, mUDPAddress ) &&
                ::compare( inOtherPair->mUserID, mUserID ) ) {
                return true;
                }
            else {
                return false;
                }
            }

        
        
        struct UDPAddress *mUDPAddress;

        struct UserID *mUserID;
    };



/**
 * Class that tracks UDP addresses that we have seen.
 *
 * @author Jason Rohrer.
 */
class AddressCache {

    public:

        /**
         * Constructs a cache.
         */
        AddressCache();

        ~AddressCache();


        
        /**
         * Adds an address/user pair to this cache.
         *
         * @param inAddress the address to add.
         *   Copied internally, so must be destroyed by caller.
         * @param inUserID the ID to add.
         *   Copied internally, so must be destroyed by caller.
         */
        void addAddress( struct UDPAddress *inAddress,
                         struct UserID *inUserID );
        

        
        /**
         * Gets a set of addresses.
         *
         * @param inNumDesired the number of addresses desired.
         * @param outNumReturned pointer to where the number of addresses
         *   should be returned.  The number is less than or equal to
         *   inNumDesired.
         *
         * @return an array of addresses.  Array and addresses must
         *   be destroyed by caller.
         */
        AddressAndUser **getAddresses( int inNumDesired,
                                       int *outNumReturned );

        

        /**
         * Gets the number of addresses that are currently in this cache.
         *
         * @return the number of addresses.
         */
        unsigned long getCacheSize();



    private:
        MutexLock *mLock;
        
        SimpleVector<AddressAndUser *> *mAddresses;

        unsigned long mMaxCacheSize;
        unsigned long mMinCacheSize;
    };



#endif
