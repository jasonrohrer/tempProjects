/*
 * Modification History
 *
 * 2001-December-18   Jason Rohrer
 * Created.
 *
 * 2001-December-19   Jason Rohrer
 * Finished initial implementation.
 */



#ifndef USER_INCLUDED
#define USER_INCLUDED



#include "minorGems/io/Serializable.h"



/**
 * A container for a KinPix  user.
 *
 * @author Jason Rohrer
 */
class User : public Serializable {

	public:


		
		/**
		 * Constructs a new user.
		 *
		 * @param inName a \0-terminated string containing the name of the
		 *   user.  Will be destroyed when this class is destroyed,
		 *   so should not be const.
		 * @param inPassword a \0-terminated string containing the password
		 *   of the user.  Will be destroyed when this class is destroyed,
		 *   so should not be const.
		 * @param inGender the gender of the user, either 'm' or 'f'.
		 * @param inAge the age of the user, in years.
		 */
		User( char *inName, char *inPassword, char inGender, int inAge );


		
		/**
		 * Constructs a user, setting all pointer parameters to NULL.
		 *
		 * Useful prior to deserialization.
		 */
		User();

		
		
		~User();
		

		
		char *mName;
		char *mPassword;

		// 'm' or 'f'
		char mGender;

		int mAge;


		
		int mNumRoundsCorrect;

		int mNumRoundsPlayed;



		// implement the Serializable interface
		virtual int serialize( OutputStream *inOutputStream );
		virtual int deserialize( InputStream *inInputStream );


	protected:


		
		/**
		 * Deletes character strings if non-NULL.
		 */
		void checkAndDelete();


		
	};



inline User::User( char *inName, char *inPassword,
				   char inGender, int inAge )
	: mName( inName ), mPassword( inPassword ), mGender( inGender ),
	  mAge( inAge ), mNumRoundsCorrect( 0 ), mNumRoundsPlayed( 0 ) {

	}



inline User::User()
	: mName( NULL ), mPassword( NULL ), mGender( 'n' ),
	  mAge( 0 ), mNumRoundsCorrect( 0 ), mNumRoundsPlayed( 0 ) {

	}



inline User::~User() {
	checkAndDelete();
	}



inline void User::checkAndDelete() {
	if( mName != NULL ) {
		delete [] mName;
		}
	if( mPassword != NULL ) {
		delete [] mPassword;
		}
	}



inline int User::serialize( OutputStream *inOutputStream ) {
	int numBytes = 0;

	long nameLength = strlen( mName );
	numBytes += inOutputStream->writeLong( nameLength );
	
 	numBytes += inOutputStream->write( (unsigned char *)mName,
									   nameLength );

	long passwordLength = strlen( mPassword );
	numBytes += inOutputStream->writeLong( passwordLength );
	
	numBytes += inOutputStream->write( (unsigned char *)mPassword,
									   passwordLength );

	long gender = mGender;	
	numBytes += inOutputStream->writeLong( gender );

	long age = mAge;
	numBytes += inOutputStream->writeLong( age );

	long correctRounds = mNumRoundsCorrect;
	numBytes += inOutputStream->writeLong( correctRounds );

	long playedRounds = mNumRoundsPlayed;
	numBytes += inOutputStream->writeLong( playedRounds );
	
	
	return numBytes;
	}
	
	
	
inline int User::deserialize( InputStream *inInputStream ) {
	int numBytes = 0;

	checkAndDelete();

	long nameLength;
	numBytes += inInputStream->readLong( &nameLength );
	mName = new char[ nameLength + 1 ];
   
 	numBytes += inInputStream->read( (unsigned char *)mName,
									   nameLength );
	mName[ nameLength ] = '\0';
	
	long passwordLength;
	numBytes += inInputStream->readLong( &passwordLength );

	mPassword = new char[ passwordLength + 1 ];
	numBytes += inInputStream->read( (unsigned char *)mPassword,
									   passwordLength );
	mPassword[ passwordLength ] = '\0';
	
	long gender;	
	numBytes += inInputStream->readLong( &gender );
	mGender = (char)gender;
	
	long age = mAge;
	numBytes += inInputStream->readLong( &age );
	mAge = (char)age;
	
	long correctRounds;
	numBytes += inInputStream->readLong( &correctRounds );
	mNumRoundsCorrect = (char)correctRounds;
	
	long playedRounds;
	numBytes += inInputStream->readLong( &playedRounds );
	mNumRoundsPlayed = (char)playedRounds;
	
	return numBytes;
	}



#endif
