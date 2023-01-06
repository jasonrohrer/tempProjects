
#include "minorGems/util/SimpleVector.h"



/**
 * Class that tracks which files we have sent to a client so far.
 *
 * @author Jason Rohrer.
 */
class SentFileTracker {

    public:

        ~SentFileTracker();


        /**
         * Checks if we have sent a particular file to the client.
         *
         * If this call returns "false", the file is added to our
         * list of files sent.
         *
         * @param inFileName the name of the file to check.
         *   Destroyed by caller.
         *
         * @return true if already sent, false if not yet sent (and thus
         *   added to the tracker).
         */
        char sentFileYet( char *inFileName );


        
        /**
         * Clear the list of files tracked as "sent."
         */
        void clearTracker();


        
    private:

        SimpleVector<char*> mFileNames;

    };
