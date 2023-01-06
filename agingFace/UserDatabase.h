/*
 * Modification History
 *
 * 2001-December-18   Jason Rohrer
 * Created.
 *
 * 2001-December-19   Jason Rohrer
 * Worked on implementation.
 * Finished initial implementation.
 * Added a check for an existing user to addUser.
 * Fixed bug in score computation.
 *
 * 2001-December-20   Jason Rohrer
 * Fixed a deadlock bug in addUser().
 */



#ifndef USER_DATABASE_INCLUDED
#define USER_DATABASE_INCLUDED



#include "User.h"

#include "minorGems/util/SimpleVector.h"

#include "minorGems/io/file/File.h"
#include "minorGems/io/file/FileOutputStream.h"
#include "minorGems/io/file/FileInputStream.h"



/**
 * A database of users.
 *
 * @author Jason Rohrer
 */
class UserDatabase {

	public:


		/**
		 * Constructs a user database.
		 *		 
		 * @param inDatabaseStoreFile the file to restore the
		 *   repository from and store the repository to periodically.
		 *   Will be destroyed when this class is destroyed.
		 * @param inSecondsBetweenSaves the number of seconds
		 *   between disk saves of the repository.
		 */
		UserDatabase( File *inDatabaseStoreFile,
					  unsigned long inSecondsBetweenSaves );


		
		~UserDatabase();



		/**
		 * Adds a user to the database.
		 *
		 * @param inUser the user to add.  Will be destroyed
		 *   when this class is destroyed.
		 *
		 * @return true iff inUser was added successfully.
		 *   If false returned, inUser's name collided with
		 *   the name of an existing user, and inUser must
		 *   be destroyed by caller.
		 */
		char addUser( User *inUser );



		/**
		 * Gets the user associated with a name.
		 *
		 * @param inName a \0-terminated string containing the user's name.
		 *   Must be destroyed by caller if non-const.
		 *
		 * @return the matching user, or NULL if there is no match.
		 *   Must NOT be destroyed by caller.
		 */
		User *getUser( char *inName );
		


		/**
		 * Gets the number of users in the database.
		 *
		 * @return the number of users.
		 */
		int getNumUsers();



		/**
		 * Gets the rank of a user.
		 *
		 * @param inName a \0-terminated string containing the user's name.
		 *   Must be destroyed by caller if non-const.
		 *
		 * @return the rank of the user, out of getNumUsers(), or
		 *   -1 if the user does not exist.
		 */
		int getUserRank( char *inName );



	protected:

		File *mDatabaseStoreFile;

		unsigned long mLastSaveTime;
		unsigned long mSecondsBetweenSaves;

		MutexLock *mLock;
		
		SimpleVector<User *> *mUserVector;


		/**
		 * Saves the database to disk.
		 *
		 * Not thread safe, so should only be called
		 * by thread safe functions.
		 */
		virtual void saveDatabase();



		/**
		 * Checks if enough time has passed to warrant a save,
		 * and saves the database if necessary.
		 *
		 * Not thread safe, so should only be called
		 * by thread safe functions.
		 */
		virtual void checkAndSaveDatabase();


		
	};



inline UserDatabase::UserDatabase( File *inDatabaseStoreFile,
								   unsigned long inSecondsBetweenSaves )
	: mSecondsBetweenSaves( inSecondsBetweenSaves ),
	  mDatabaseStoreFile( inDatabaseStoreFile ),
	  mLock( new MutexLock() ),
	  mUserVector( new SimpleVector<User *>() ) {

	
	mLastSaveTime = time( NULL );
	
	if( mDatabaseStoreFile->exists() ) {
		
		FileInputStream *inStream = new FileInputStream(
			mDatabaseStoreFile );

		long numUsers;
		inStream->readLong( &numUsers );

		for( int i=0; i<numUsers; i++ ) {
			User *user = new User();

			user->deserialize( inStream );

			mUserVector->push_back( user );
			}

		delete inStream;
		}
	}



inline UserDatabase::~UserDatabase() {
	int numUsers = mUserVector->size();
	
	for( int i=0; i<numUsers; i++ ) {
		User *user = *( mUserVector->getElement( i ) );

		delete user;
		}

	delete mUserVector;

	delete mLock;

	delete mDatabaseStoreFile;
	}



inline char UserDatabase::addUser( User *inUser ) {
	if( getUser( inUser->mName ) != NULL ) {
		// user already exists
		return false;
		}

	mLock->lock();
	
	mUserVector->push_back( inUser );

	checkAndSaveDatabase();
	
	mLock->unlock();

	return true;
	}



inline User *UserDatabase::getUser( char *inName ) {
	mLock->lock();

	checkAndSaveDatabase();
	
	int numUsers = mUserVector->size();

	for( int i=0; i<numUsers; i++ ) {
		User *user = *( mUserVector->getElement( i ) );
		
		if( !( strcmp( inName, user->mName ) ) ) {
			mLock->unlock();
			return user;
			}
		}

	// user not found
	
	mLock->unlock();
	return NULL;
	}



inline int UserDatabase::getNumUsers() {
	mLock->lock();

	checkAndSaveDatabase();
	
	int numUsers = mUserVector->size();

	mLock->unlock();

	return numUsers;
	}



inline int UserDatabase::getUserRank( char *inName ) {
	// getUser call will checkAndSaveDatabase()
	
	User *user = getUser( inName );

	if( user != NULL ) {
		mLock->lock();

		int numUsers = mUserVector->size();

		double score;
		
		// user gets worst score if s/he hasn't played yet
		if( user->mNumRoundsPlayed == 0 ) {
			score = 0;
			}
		else {
			score = user->mNumRoundsCorrect /
				(double)( user->mNumRoundsPlayed );
			}
		
		// count how many have a higher score
		int numWithHigherScore = 0;

		for( int i=0; i<numUsers; i++ ) {
			User *otherUser = *( mUserVector->getElement( i ) );
			
			double otherScore;
			if( otherUser->mNumRoundsPlayed == 0 ) {
				otherScore = 0;
				}
			else {
				otherScore = otherUser->mNumRoundsCorrect /
					(double)( otherUser->mNumRoundsPlayed );  
				}

			if( otherScore > score ) {
				numWithHigherScore++;
				}
			}			
		mLock->unlock();

		return numWithHigherScore + 1;
		}
	else {
		return -1;
		}
	}



inline void UserDatabase::checkAndSaveDatabase() {

	// check if the database needs to be saved.
	unsigned long currentTime = time( NULL );
	
	if( currentTime - mLastSaveTime > mSecondsBetweenSaves ) {
		saveDatabase();

		mLastSaveTime = currentTime;
		}

	}



inline void UserDatabase::saveDatabase() {
	printf( "Saving user database...\n" );

	File *backupFile = NULL;
	
	if( mDatabaseStoreFile->exists() ) {
		// first, make a backup of our current store file
		
		backupFile = new File( NULL, "userDatabaseBackup.nul" );

		mDatabaseStoreFile->copy( backupFile );
		}


	// now we save the current database state into the store file
			
	FileOutputStream *outStream = new FileOutputStream(
		mDatabaseStoreFile );
	
	int numUsers = mUserVector->size();

	outStream->writeLong( numUsers );

	for( int i=0; i<numUsers; i++ ) {
		User *user = *( mUserVector->getElement( i ) );

		user->serialize( outStream );
		}

	
	delete outStream;
	
	// we have successfully written a new file,
	// so delete our backup

	if( backupFile != NULL ) {
		backupFile->remove();
		delete backupFile;
		}

	printf( "...done\n" );
	}



#endif

