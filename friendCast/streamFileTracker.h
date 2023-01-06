/**
 * Removes all files from tracker.
 */
void clearTracker();



/**
 * Sets whether stream file set originated locally or not.
 *
 * @param inLocal true if local.
 */
void setLocalFileSetStreaming( char inLocal );



/**
 * Gets whether file set is local.
 *
 * @return true if local, false if remote.
 */
char getLocalFileSetStreaming();



/**
 * Gets a file name that has not been used yet.
 *
 * @return a unique file name.
 *   Destroyed by caller.
 */
char *getUniqueFileName();



/**
 * Adds a file name to the list of names tracked.
 *
 * @param inFileName the name of the file to add.
 *   Destroyed by caller.
 */
void addFileName( char *inFileName );



/**
 * Gets the current index pointer for this file series.
 *
 * @return the current index or -1 if we have no files yet.
 */
int getCurrentFileIndex();



/**
 * Sets the index of the first file in the current channel.
 *
 * @param inIndex the index.
 */
void setStartOfCurrentChannel( int inIndex );



/**
 * Gets the index of the first file in the current channel.
 * Should be used for looping.  In other words, we should not
 * loop back all the way to the beginning of the stream (back to files
 * received on previous channels).
 *
 * @return the index.
 */
int getStartOfCurrentChannel();



/**
 * Steps the current index pointer to the next file.
 */
void stepCurrentIndexToNextFile();



/**
 * Jumps the current index pointer to the end of the file list (to the
 * last file on the list).
 */
void jumpCurrentIndexToEnd();



/**
 * Gets the index of the last file in this stream.
 *
 * @return the index of the last file.
 */
int getMaxFileIndex();



/**
 * Gets the file name associated with a given index.
 *
 * @param inIndex the index.
 *
 * @return the file name.
 *   Destroyed by caller.
 */
char *getFileName( int inIndex );
