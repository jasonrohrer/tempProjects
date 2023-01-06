/*
 * Modification History
 *
 * 2001-May-11   Jason Rohrer
 * Created.
 *
 * 2001-November-13   Jason Rohrer
 * Changed to use a TemplatePageGenerator.
 *
 * 2001-November-15   Jason Rohrer
 * Added support for repository seed files.
 *
 * 2001-November-16   Jason Rohrer
 * Added support for main page templates.
 * Added support for round choice templates.
 * Added support for result choice templates.
 *
 * 2001-November-17   Jason Rohrer
 * Changed to use a DistributionExampleRepository.
 *
 * 2001-November-18   Jason Rohrer
 * Added support for passing in the number of seconds between
 * repository saves to the repository constructor.
 *
 * 2001-December-2   Jason Rohrer
 * Changed to let TemplatePageGenerator delete template files.
 *
 * 2001-December-19   Jason Rohrer
 * Added support for a user database.
 * Added a template for the login form.
 */


 
#ifndef WEB_SERVER_INCLUDED
#define WEB_SERVER_INCLUDED 

#include "minorGems/io/file/Path.h"
#include "minorGems/io/file/File.h"
#include "minorGems/io/file/FileInputStream.h"

#include "minorGems/network/Socket.h"
#include "minorGems/network/SocketServer.h"
#include "minorGems/network/SocketStream.h"

#include "RequestHandlingThread.h"
#include "ThreadHandlingThread.h"
#include "UniformExampleRepository.h"
#include "TemplatePageGenerator.h"

#include "DistributionExampleRepository.h"
#include "UserDatabase.h"


#include "minorGems/util/random/StdRandomSource.h"

#include <string.h>
#include <stdio.h>


/**
 * Main class for the microWeb web server.
 *
 * @author Jason Rohrer.
 */
class WebServer {



	public:

		/**
		 * Constructs a server, specifying a configuration file.
		 *
		 * @param inConfigurationaFile the file to read configuration
		 *   information from.  Must be destroyed by caller.
		 */
		WebServer( File *inConfigurationFile );

		~WebServer();

		/**
		 * Starts this server running and never returns.
		 */
		void runServer();

		
	private:
		int mPortNumber;
		int mMaxQueuedConnections;

		char *mMimeString;

		char *mRootPathString;

		SocketServer *mServer;
		ThreadHandlingThread *mThreadHandler;

		StdRandomSource *mRandSource;
		DistributionExampleRepository *mRepository;
		TemplatePageGenerator *mPageGenerator;
		UserDatabase *mUserDatabase;
		
		/**
		 * Reads a class configuration value from a file.
		 *
		 * @param inFile the file to read the value from.
		 * @param inKey the key marking the value in the file.
		 *   Must be destroyed by caller.
		 */
		void readConfigurationValue( FILE *inFile, char *inKey );

	};



inline WebServer::WebServer( File *inConfigurationFile )
	: mPortNumber( -1 ), mMaxQueuedConnections( -1 ),
	  mMimeString( NULL ), mRootPathString( NULL ),
	  mThreadHandler( new ThreadHandlingThread() ),
	  mRandSource( new StdRandomSource() ) {

	File *seedFile = new File( NULL, "repositorySeed.txt" );
		
	File *repositoryStoreFile = new File( NULL, "repository.str" );

	File *userDatabaseStoreFile = new File( NULL, "userDatabase.str" );
	
	//mRepository = new UniformExampleRepository( mRandSource, seedFile );
	mRepository = new DistributionExampleRepository( mRandSource,
													 30,
													 repositoryStoreFile,
													 seedFile );

	mUserDatabase = new UserDatabase( userDatabaseStoreFile, 30 );

	
	delete seedFile;

	File *mainPageFile = new File( NULL, "mainpage.html" );
	File *headerFile = new File( NULL, "header.html" );
	File *footerFile = new File( NULL, "footer.html" );

	File *playRoundFile = new File( NULL, "playRound.html" );
	File *roundChoiceFile = new File( NULL, "roundChoice.html" );

	File *playResultFile = new File( NULL, "playResult.html" );
	File *resultChoiceFile = new File( NULL, "resultChoice.html" );
	File *resultCorrectChoiceFile =
		new File( NULL, "resultCorrectChoice.html" );

	File *addFormFile = new File( NULL, "addForm.html" );
	File *addResultFile = new File( NULL, "addResult.html" );

	File *createUserFile = new File( NULL, "createUserForm.html" );
	File *loginFormFile = new File( NULL, "loginForm.html" );

	
	mPageGenerator = new TemplatePageGenerator(
		mRepository,
		mUserDatabase,
		mainPageFile,
		headerFile,
		footerFile,

		playRoundFile,
		roundChoiceFile,
		
		playResultFile,
		resultChoiceFile,
		resultCorrectChoiceFile,

		addFormFile,
		addResultFile,

		createUserFile,
		loginFormFile );

	
	if( inConfigurationFile->exists() ) {
		int fileNameLength;
		char *fileName =
			inConfigurationFile->getFullFileName( &fileNameLength );

		FILE *file = fopen( fileName, "r" );

		
		int numRead = 1;
		char *inKeyBuffer = new char[99];

		// read each key from the file, and parse its value
		while( numRead == 1 ) {
			numRead = fscanf( file, "%s", inKeyBuffer );

			if( numRead == 1 ) {
				readConfigurationValue( file, inKeyBuffer );
				}
			}

		// last key read failed, so done with file

		fclose( file );
		delete [] inKeyBuffer;
		}

	
	// check for uninitialized configuration values

	if( mPortNumber == -1 ) {
		mPortNumber = 80;
		printf( "using default:  port_number = %d\n", mPortNumber );
		}
	if( mMaxQueuedConnections == -1 ) {
		mMaxQueuedConnections = 100;
		printf( "using default:  max_queued_connections = %d\n",
				mMaxQueuedConnections );
		}
	if( mMimeString == NULL ) {
		mMimeString = new char[99];
		sprintf( mMimeString, "audio/mpeg" );

		printf( "using default:  mime_type = %s\n", mMimeString );
		}
	if( mRootPathString == NULL ) {
		mRootPathString = new char[99];
		sprintf( mRootPathString, "httpRoot" );

		printf( "using default:  root_path = %s\n", mRootPathString );
		}

	mServer = new SocketServer( mPortNumber, mMaxQueuedConnections );
	}



inline WebServer::~WebServer() {
	delete [] mMimeString;
	delete [] mRootPathString;

	delete mServer;
	delete mThreadHandler;

	delete mRepository;
	delete mPageGenerator;
	delete mRandSource;
	}


class dummyThread : public Thread {

	public:
		virtual void run();

	};

inline void dummyThread::run() {

	}


inline void WebServer::runServer() {

	mThreadHandler->start();

	// print a log message to stdio
	char *timestamp = RequestHandlingThread::getTimestamp();
	printf( "%s:  listening for connections on port %d\n",
			timestamp, mPortNumber );
	delete [] timestamp;

	
	// main server loop
	while( true ) {
		// printf( "waiting for a connection\n" );
		Socket *sock = mServer->acceptConnection();
		// printf( "received a connection\n" );

		RequestHandlingThread *thread =
					new RequestHandlingThread( sock, mPageGenerator );
		thread->start();

		mThreadHandler->addThread( thread );
		}
	
	}



inline void WebServer::readConfigurationValue(
	FILE *inFile, char *inKey ) {

	int numRead = 0;
	if( !strcmp( inKey, "port_number" ) ) {

		numRead = fscanf( inFile, "%d", &mPortNumber );
		}
	else if( !strcmp( inKey, "max_queued_connections" ) ) {

		numRead = fscanf( inFile, "%d", &mMaxQueuedConnections );
		}
	else if( !strcmp( inKey, "mime_type" ) ) {

		mMimeString = new char[99];
		numRead = fscanf( inFile, "%s", mMimeString );
		}
	else if( !strcmp( inKey, "root_path" ) ) {

		mRootPathString = new char[99];
		numRead = fscanf( inFile, "%s", mRootPathString );
		}
	else {
		printf( "unknown key in configuration file:\n" );
		printf( "%s\n", inKey );
		return;
		}

	// check for value read failure
	if( numRead != 1 ) {
		printf( "failed to read key value from configuration file:\n" );
		printf( "%s\n", inKey );
		}
	}




#endif
