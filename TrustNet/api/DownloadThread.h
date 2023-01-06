/*
 * Modification History
 *
 * 2004-November-21   Jason Rohrer
 * Created.
 */



#ifndef DOWNLOAD_THREAD_INCLUDED
#define DOWNLOAD_THREAD_INCLUDED


#include "Primrose/api/primroseAPI.h"
#include "minorGems/system/FinishedSignalThread.h"
#include "minorGems/system/StopSignalThread.h"




/**
 * Class that encapsulates information about a download host.
 */
class DownloadHost {

    public:

        
        /**
         * Constructs a download host.
         *
         * All parameters are copied internally.
         */
        DownloadHost( struct UserID *inID,
                      char *inIPAddress,
                      unsigned short inTCPPort,
                      unsigned short inUDPPort,
                      char inFirewallStatus )
            : mID( copy( inID ) ),
              mIPAddress( stringDuplicate( inIPAddress ) ),
              mTCPPort( inTCPPort ),
              mUDPPort( inUDPPort ),
              mFirewallStatus( inFirewallStatus ) {

            }

        
        
        ~DownloadHost() {
            delete mID;
            delete [] mIPAddress;
            }

        

        struct UserID *mID;
        char *mIPAddress;
        unsigned short mTCPPort;
        unsigned short mUDPPort;
        char mFirewallStatus;

    };



/**
 * Thread that downloads a file from multiple hosts and reports progress
 * to a handler function.
 *
 * @author Jason Rohrer.
 */
class DownloadThread : public FinishedSignalThread, public StopSignalThread {

    public :

        /**
         * Constructs and starts a thread.
         *
         * Parameters are the same as primroseGetFile.  They
         * are copied internally by this thread, so they must be destroyed by
         * caller.
         */
        DownloadThread( unsigned long inNumHosts,
                        struct UserID **inUserIDs,
                        char **inIPAddresses, 
                        unsigned short *inTCPPorts,
                        unsigned short *inUDPPorts, 
                        char *inFirewallStatuses,
                        char *inFileName,
                        char *inFileHash,
                        unsigned long inFileSize,
                        char (*inDownloadProgressHandler)( int, unsigned long,
                                                           void * ),
                        void *inExtraHandlerArgument );


        
        /**
         * Stops, joins, and destroys this thread.
         */
        ~DownloadThread();

        

        /**
         * Gets whether this download has been canceled.
         *
         * @return true if canceled, or false if it is still going.
         */
        char isCanceled();


        
        // implements the Thread interface
        void run();


        // leave these as public for now to make implementing handler
        // functions easier
        int mNumFileSources;
        DownloadHost **mFileSources;
        char *mFileName;
        char *mFileHash;
        unsigned long mFileSize;

        char (*mDownloadProgressHandler)( int, unsigned long,
                                          void * );
        void *mExtraHandlerArgument;

        char mFailed;
    };



#endif
