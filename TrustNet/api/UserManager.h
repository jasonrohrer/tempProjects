/*
 * Modification History
 *
 * 2004-November-7   Jason Rohrer
 * Created.
 *
 * 2004-January-10   Jason Rohrer
 * Added support for expiration of cached trust scores.
 */



#ifndef PRIMROSE_USER_MANAGER_INCLUDED
#define PRIMROSE_USER_MANAGER_INCLUDED



/**
 * Class that manages trust ratings and keys for users.
 *
 * @author Jason Rohrer.
 */
class UserManager {

    public:


        
        /**
         * Creates an account on the trust server.
         *
         * @parm inUserName the human-readable user name.
         *   Must be destroyed by caller.
         * @parm inPublicKeyHex the public portion of the user's RSA key as
         *   ASCII hex.
         *   Must be destroyed by caller.
         * @param outReturnCode pointer to where the return code should be
         *   returned.  Will be set to one of:
         *   1 = success
         *   2 = failed to connect to trust server
         *   3 = user name already exists
         *
         * @return the created userID, or NULL if user creation fails.
         *   Must be destroyed by caller.
         */
        static struct UserID *createAccount( char *inUserName,
                                             char *inPublicKeyHex,
                                             int *outReturnCode );

        
        
        /**
         * Gets the trust score of a user.
         *
         *
         * @param inUserID the trusting user's ID.
         *   Must be destroyed by caller.
         * @param inTrustedID the trusted user's ID.
         *   Must be destroyed by caller.
         *
         * @return the trust score of the user in the range [0,1].
         */
        static double getTrustScore( struct UserID *inUserID,
                                     struct UserID *inTrustedID );

        

        /**
         * Gets the public RSA key of a user.
         *
         * @param inUserID the user's ID.
         *   Must be destroyed by caller.
         *
         * @return the RSA key as hex-encoded ASCII, or NULL if getting
         *   the key fails.
         *   Must be destroyed by caller.
         */
        static char *getPublicKey( struct UserID *inUserID );


        
        /**
         * Gets the human-readable name of a user.
         *
         * @param inUserID the user's ID.
         *   Must be destroyed by caller.
         *
         * @return the user name, or NULL if getting the name fails.
         *   Must be destroyed by caller.
         */
        static char *getUserName( struct UserID *inUserID );


        
        /**
         * Gets the ID for a given user name.
         *
         * @param inUserName the user's name.
         *   Must be destroyed by caller.
         *
         * @return the ID, or NULL if getting the ID fails.
         *   Must be destroyed by caller.
         */
        static struct UserID *getUserID( char *inUserName );
        

        
        static const int TRUSTED = 1;
        static const int NEUTRAL = 0;
        static const int DISTRUSTED = -1;


        /**
         * Sets a trust relationship on the trust server.
         *
         * The trust relationships are used by the trust server to compute
         * system-wide trust scores for each user.  Also, we will locally
         * treat TRUSTED users as having a trust score of 1 and DISTRUSTED
         * users as having a trust score of 0.  NEUTRAL users will be
         * treated according to their system-wide trust score.
         * 
         *
         * @param inUserID the trusting user's ID.
         *   Must be destroyed by caller.
         * @param inTrustedID the trusted user's ID.
         *   Must be destroyed by caller.
         * @param inType one of TRUSTED, NEUTRAL, or DISTRUSTED.
         * @param inPrivateKeyHex the trusting user's private key, hex-encoded.
         *   Must be destroyed by caller.
         *
         * @return true on success, or false on failure.
         */
        static char setTrustRelationship( struct UserID *inUserID,
                                          struct UserID *inTrustedID,
                                          int inType,
                                          char *inPrivateKeyHex );

        

        static const int FAILED = -2;


        /**
         * Gets a trust relationship.
         *
         * @param inUserID the trusting user's ID.
         *   Must be destroyed by caller.
         * @param inTrustedID the trusted user's ID.
         *   Must be destroyed by caller.
         *
         * @return one of TRUSTED, NEUTRAL, DISTRUSTED, or FAILED.
         */
        static int getTrustRelationship( struct UserID *inUserID,
                                         struct UserID *inTrustedID );



        /**
         * Gets a list of trusted or distrusted users.
         *
         * @param inUserID the trusting user's ID.
         *   Must be destroyed by caller.
         * @param inType one of TRUSTED or DISTRUSTED.
         * @param outSetSize pointer to where the length of the
         *   returned array should be returned.  Will be set to -1
         *   on failur.
         *
         * @return an array of user IDs, or NULL on failure.
         *   Array and IDs must be destroyed by caller.
         */
        static struct UserID **getTrustSet( struct UserID *inUserID,
                                            int inType,
                                            int *outSetSize );

        
        
    private:


        
        // how long (in seconds) before cached trust scores expire
        // and are re-fetched from the server.
        // 86400 seconds = 24 hours
        static const int mCacheExpirationInSeconds = 86400;

        
        
        /**
         * Reads the value of a user file.
         *
         * @param inUserID the ID of the user.
         *   Must be destroyed by caller.
         * @param inFileExtension the file extension to use.
         *   For example, an extension of "name" would read
         *   the value from the ".name" file for the user ID.
         *   Must be destroyed by caller.
         * @param inMaxAgeInSeconds the maximum age in seconds of the file,
         *   or -1 to ignore age.  Older files are ignored (reading fails).
         *   Defaults to -1.
         *
         * @return the read file contents, or NULL if reading fails.
         *   Must be destroyed by caller.
         */
        static char *readUserFileValue( struct UserID *inUserID,
                                        char *inFileExtension,
                                        int inMaxAgeInSeconds = -1 );


        /**
         * Writes a value to a user file.
         *
         * @param inUserID the ID of the user.
         *   Must be destroyed by caller.
         * @param inFileExtension the file extension to use.
         *   For example, an extension of "name" would read
         *   the value from the ".name" file for the user ID.
         *   Must be destroyed by caller.
         * @param inValue the value to write.
         *   Must be destroyed by caller.
         */
        static void writeUserFileValue( struct UserID *inUserID,
                                        char *inFileExtension,
                                        char *inValue );



        /**
         * Removes user file.
         *
         * @param inUserID the ID of the user.
         *   Must be destroyed by caller.
         * @param inFileExtension the file extension to use.
         *   For example, an extension of "name" would read
         *   the value from the ".name" file for the user ID.
         *   Must be destroyed by caller.
         */
        static void removeUserFile( struct UserID *inUserID,
                                    char *inFileExtension );

        

        /**
         * Gets a string value for a user from either the local file cache
         * or the remote trust server.
         *
         * @param inUserID the ID of the user.
         *   Must be destroyed by caller.
         * @param inFileExtension the file extension to use.
         *   For example, an extension of "name" would read
         *   the value from the ".name" file for the user ID.
         *   Must be destroyed by caller.
         * @param inServerAction the action to execute on the server.
         *   Must be destroyed by caller.
         * @param inMaxAgeInSeconds the maximum age in seconds of locally
         *   cached values, or -1 to ignore age.  Older values are re-fetched
         *   from the server.  Defaults to -1.
         * @return the user string value, or NULL if getting a value fails.
         *   Must be destroyed by caller.
         */
         static char *getUserString( struct UserID *inUserID,
                                     char *inFileExtension,
                                     char *inServerAction,
                                     int inMaxAgeInSeconds = -1 );



        /**
         * Caches a mapping from a user name to an ID.
         *
         * @param inUserID the ID.
         *   Must be destroyed by caller.
         * @param inUserName the name.
         *   Must be destroyed by caller.
         */
        static void saveNameToIDMap( struct UserID *inUserID,
                                     char *inUserName );


        
    };



#endif
