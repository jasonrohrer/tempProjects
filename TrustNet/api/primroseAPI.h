/*
 * Modification History
 *
 * 2004-November-7   Jason Rohrer
 * Created.
 *
 * 2005-January-6   Jason Rohrer
 * Added support for mapping user names to IDs.
 * Added a makeUserID function.
 * Added return values for functions that set server trust relationships.
 */



/**
 * The Primrose API.
 *
 * All calls are thread-safe.
 *
 * All function parameters must be destroyed by caller.
 * All string parameters must be \0-terminated.
 *
 * @author Jason Rohrer.
 */



#ifndef PRIMROSE_API_INCLUDED
#define PRIMROSE_API_INCLUDED



/**
 * Gets whether a local key pair exists.
 *
 * @return true if a key pair exists or false otherwise.
 */
char primroseDoesLocalKeyExist();



/**
 * Generates a local public/private RSA keypair.  Automatically generates
 * a 368-bit key, which is the minimum length key for producing SHA1-based
 * digital signatures.
 *
 * @param inRandomSeed the data to seed the random generator with.
 */
void primroseCreateLocalKeyPair( char *inRandomSeed );



/**
 * Gets whether a local account exists.
 *
 * @return true if an account exists or false otherwise.
 */
char primroseDoesAccountExist();



/**
 * Attempts to create an account on the trust server.  A local key pair
 * must exist before this function is called.
 *
 * The resulting account information is stored in the settings folder.
 *
 * @param inUserName the human-readable user name.
 *
 * @return 1 on success
 *         2 if we cannot connect to the trust server
 *         3 if the user name already exists
 *         4 if required data (i.e., public key) is missing.
 */
int primroseCreateAccount( char *inUserName );



/**
 * Starts the Primrose network.  An RSA keypair and an account must
 * exist before the network is started.
 */  
void primroseStart();



/**
 * Stops the Primrose network.
 */
void primroseStop();



/**
 * Sets the minimum trust score for remote hosts that are allowed to search
 * the files on this node.
 *
 * @param inTrustLevel the trust score in the range [0,1].
 */
void primroseSetLocalSearchTrustLevel( double inTrustLevel );



/**
 * Gets the minimum trust score for remote hosts that are allowed to search
 * the files on this node.
 *
 * @return the trust score in the range [0,1].
 */
double primroseGetLocalSearchTrustLevel();



/**
 * Sets the minimum trust score for remote hosts that are allowed to download
 * the files on this node.
 *
 * @param inTrustLevel the trust score in the range [0,1].
 */
void primroseSetUploadTrustLevel( double inTrustLevel );



/**
 * Gets the minimum trust score for remote hosts that are allowed to download
 * the files on this node.
 *
 * @return the trust score in the range [0,1].
 */
double primroseGetUploadTrustLevel();



/**
 * Gets whether the sharing path is set.
 *
 * @return true if the sharing path has been set, or false if the
 *   default path is being used.
 */
char primroseIsSharingPathSet();



/**
 * Sets the root path that this node will share files from.
 *
 * If no path is set, this node defaults to sharing files from the "files"
 * directory.
 *
 * This path is saved between startups by this API (in the settings folder).
 *
 * @param inPath the platform-dependent path, relative to the main directory
 *   of this node.
 *
 * Example: <PRE>
 * primroseSetSharingPath( "temp/myFiles" );
 * </PRE> 
 */
void primroseSetSharingPath( char *inPath );



/**
 * Gets the root path that this node is sharing files from.
 *
 * @return the platform-dependent path, relative to the main directory
 *   of this node.
 */
char *primroseGetSharingPath();



/**
 * Gets whether the firewall status has been set.
 *
 * @return true if the status has been set, or false if the default setting
 *   is being used.
 */
char primroseIsFirewallStatusSet();



/**
 * Sets whether this node is firewalled.
 *
 * @param inFirewalled true if this node is firewalled, or false if inbound
 *   connections are possible.
 */
void primroseSetFirewallStatus( char inFirewalled );



/**
 * Structure that encapsulates a 64-bit user ID.
 */
struct UserID {
        unsigned long partA;
        unsigned long partB;
    };



/**
 * Gets the local ID being used by this node.
 *
 * @return this node's user ID.
 */
struct UserID *primroseGetLocalUserID();



/**
 * Gets the user name for a user.
 *
 * @param inID the ID to get a name for.
 *
 * @return the name, or NULL if getting the name fails.
 */
char *primroseGetUserName( struct UserID *inID );



/**
 * Gets the ID for a given user name.
 *
 * @param inUserName the name to get an ID for.
 *
 * @return the ID, or NULL if getting the ID fails.
 */
struct UserID *primroseGetUserID( char *inUserName );



/**
 * Sets a positive trust for another user.  We will treat this user as
 * if s/he has a trust score of 1, no matter what the user's system-wide
 * trust score actually is.
 *
 * @param inID the other user.
 *
 * @return true if the server operation succeeds, or false otherwise.
 */
char primroseTrustUser( struct UserID *inID );



/**
 * Sets a negative trust for another user.  We will treat this user
 * as if s/he has a trust score of 0, no matter what the us's system-wide
 * trust score actually is.
 *
 * @param inID the other user.
 *
 * @return true if the server operation succeeds, or false otherwise.
 */
char primroseDistrustUser( struct UserID *inID );



/**
 * Sets a neutral trust for another user.
 *
 * We will interact with this user according to the user's system-wide
 * trust score.
 *
 * @param inID the other user.
 *
 * @return true if the server operation succeeds, or false otherwise.
 */
char primroseNeutralTrustUser( struct UserID *inID );



/**
 * Gets the trust score of a user.
 *
 * The system-wide trust score of the user will be overrided if we
 * explicitly trust or distrust the user.
 *
 * @param inID the user to get the trust score of.
 *
 * @return the trust score in the range [0,1].
 */
double primroseGetUserTrustScore( struct UserID *inID );



extern int PRIMROSE_TRUSTED;
extern int PRIMROSE_NEUTRAL;
extern int PRIMROSE_DISTRUSTED;
extern int PRIMROSE_FAILED;



/**
 * Gets our direct relationship with a user.
 *
 * @param inID the ID to check.
 *
 * @return one of PRIMROSE_TRUSTED, PRIMROSE_NEUTRAL, PRIMROSE_DISTRUSTED,
 *   or PRIMROSE_FAILED (on an error).
 */
int primroseGetTrustRelationship( struct UserID *inID );



/**
 * Gets the list of users that we explicitly trust.
 *
 * @param outNumTrusted pointer to where the size of the returned array
 *   should be returned.  Will be set to -1 on failure.
 *
 * @return an array of trusted users, or NULL if fetching the list fails.
 */
struct UserID **primroseGetTrustedUsers( int *outNumTrusted );



/**
 * Gets the list of users that we explicitly distrust.
 *
 * @param outNumDistrusted pointer to where the size of the returned array
 *   should be returned.  Will be set to -1 on failure.
 *
 * @return an array of distrusted users, or NULL if fetching the list fails.
 */
struct UserID **primroseGetDistrustedUsers( int *outNumDistrusted );



/**
 * Adds a handler for processing incoming search results.
 *
 * All search results will be passed to all registered result
 * handlers.
 *
 * All parameters passed in to handler will be destroyed by caller and
 * therefore should not be destroyed by the handler.
 *
 * The extra (void *) argument can be used to encapsulate any additional
 * state that should be associated with a particular handler.  For example,
 * it could be a pointer to a C++ object that is "unwrapped" by casting
 * inside the handler function.
 *
 * @param inHandlerFunction pointer to the handler function.
 *   This function must return void and take the following arguments:
 *   (char *inSearchID, struct UserID *inUserID, char inFirewallStatus,
 *    char *inIPAddress, unsigned short inUDPPort, unsigned short inTCPPort,
 *    unsigned long inNumResults,
 *    char **inFileNames, char **inHashes, unsigned long *inSizes,
 *    void *inExtraArgument ).
 * @param inExtraHandlerArgument pointer to an extra argument to be passed
 *   in to the handler function each time it is called.
 *   Must be destroyed by caller after the handler is removed.
 *
 * @return an ID for this handler that can be used to remove it later.
 *
 * Example: <PRE>
 * // define a handler function
 * void myHandler( char *inSearchID, struct UserID *inUserID,
 *                 char inFirewallStatus,
 *                 char *inIPAddress, unsigned short inTCPPort,
 *                 unsigned short inUDPPort,
 *                 unsigned long inNumResults,
 *                 char **inFileNames, char **inHashes, unsigned long *inSizes,
 *                 void *inExtraArgument ) {
 *     // handle message here
 *     ...
 *     }
 *
 * // elsewhere in code, register the handler function
 * // extra argument NULL since myHandler does not use it
 * int handlerID = primroseAddSearchResultsHandler( myHandler, (void *)NULL );
 * </PRE>
 */ 
unsigned long primroseAddSearchResultsHandler(
    void (*inHandlerFunction)( char *, struct UserID *, char,
                               char *,
                               unsigned short,
                               unsigned short,
                               unsigned long, char **, char **,
                               unsigned long *, void * ),
    void *inExtraHandlerArgument );



/**
 * Removes a search results handler.
 *
 * @param inHandlerID the ID of the handler to remove.
 */
void primroseRemoveSearchResultsHandler( unsigned long inHandlerID );



/**
 * Initiates a search.  This call returns immediately. Results will be passed
 * to registered result handlers.
 *
 * @param inSearchID a unique ID for this search.
 * @param inSearchTerms the search terms, or a SHA1 hash value prefixed
 *   by "hash_".
 * @param inDesiredResultCount the desired number of results.  The search
 *   process will terminate when the desired number of results have been
 *   returned or the host list is exhausted.
 * @param inMaxTimeInSeconds the maximum time to keep searching in seconds.
 * @param inTrustLevel the minimum trust score, in the range [0,1], of
 *   nodes that we will send the search to.
 *
 * Example: <PRE>
 * primroseSearch( "234892384", "indie rock mp3", 10, 0.75 );
 * </PRE>
 */
void primroseSearch( char *inSearchID, char *inSearchTerms,
                     unsigned long inDesiredResultCount,                     
                     unsigned long inMaxTimeInSeconds,
                     double inTrustLevel );



extern int PRIMROSE_DOWNLOAD_IN_PROGRESS;
extern int PRIMROSE_DOWNLOAD_FAILED;
extern int PRIMROSE_DOWNLOAD_CANCELED;



/**
 * Get a file from a selection of hosts (both firewalled and not) by swarming.
 * This function returns immediately.  Arguments (except for
 * inExtraHandlerArgument---see below) can be destroyed by caller immediately
 * after this function returns, since they are copied internally.
 *
 * @param inNumHosts the number of hosts for the file.
 * @param inUserIDs array of pointers to UserIDs, one for each host.
 * @param inIPAddress array of IP addresses, one for each host.
 * @param inUDPPorts array of UDP ports, one for each host.
 * @param inTCPPorts array of TCP ports, one for each host.
 * @param inFirewallStatuses array of firewall status flags (true=> firewalled,
 *   false=> not_firewalled), one for each host.
 * @param inFileName the name of the file (used to save the file).
 * @param inFileHash the SHA1 hash of the file in hex-encoded ASCII.
 * @param inFileSize the size of the file in bytes.
 * @praram inDownloadProgressHandler pointer to the handler function for
 *   download progress events.
 *   This function must return true to continue the download (or false
 *   to cancel) and take the following arguments:
 *   (int inResultCode, unsigned long inTotalBytesReceived,
 *    void *inExtraArgument ).
 *   inResultCode will be set to one of PRIMROSE_DOWNLOAD_IN_PROGRESS,
 *   PRIMROSE_DOWNLOAD_FAILED, or PRIMROSE_DOWNLOAD_CANCELED.
 *   Once a FAILED or CANCELED result code is passed to the handler, the
 *   handler will never be called again, so it should destroy its extra
 *   argument, if necessary.
 * @param inExtraHandlerArgument pointer to an extra argument to be passed
 *   in to the handler function each time it is called.
 *   Must be destroyed by caller after the handler is removed.
 *
 *
 * Example: <PRE>
 * // define a handler function
 * int myHandler( int inResultCode, unsigned long inTotalBytesReceived,
 *                void *inExtraArgument ) {
 *     // handle event here
 *     ...
 *     // keep going
 *     return true;
 *     }
 *
 * // elsewhere in code, start a download
 * // extra argument NULL since myHandler does not use it
 * struct UserID **idArray = new struct UserID*[1];
 * idArray[0] = new struct UserID;
 * idArray[0]->partA = 0;
 * idArray[0]->partB = 278374;
 *
 * int handlerID = primroseGetFile(
 *                    1, idArray, { "128.77.44.1" }, { 8000 }, { 9000 },
 *                    { true },
 *                    "test.mp3",
 *                    "FB322F86DB95BE0670D06631EE27E83C5B64679A",
 *                    4873833,
 *                    myHandler, (void *)NULL );
 * delete idArray[0];
 * delete [] idArray;
 * </PRE>
 */
void primroseGetFile( unsigned long inNumHosts,
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
 * Compares two user IDs.
 *
 * @param inFirstID the first ID.
 * @param inSecondID the second ID.
 *
 * @return true if the IDs are equal, or false if they are different.
 */
char compare( struct UserID *inFirstID, struct UserID *inSecondID );



/**
 * Makes a copy of a user ID.
 *
 * @param inID the ID to copy.
 *
 * @return a copy of the ID.  Must be destroyed by caller.
 */
struct UserID *copy( struct UserID *inID );



/**
 * Makes a new user ID.
 *
 * @param inPartA the first part of the ID.
 * @param inPartB the second part of the ID.
 *
 * @return a new user ID.
 */
struct UserID *makeUserID( unsigned long inPartA, unsigned long inPartB );



#endif
