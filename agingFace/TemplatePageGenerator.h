/*
 * Modification History
 *
 * 2001-November-14   Jason Rohrer
 * Created.
 *
 * 2001-November-15   Jason Rohrer
 * Added template-generated play pages.
 *
 * 2001-November-16   Jason Rohrer
 * Added support for main page templates.
 * Added functions for replacing all tags in a string at once.
 * Added serial numbers to all links to force page reloads.
 * Added support for add pair form templates.
 * Added support for better parameterization of play round
 * choices and responses in the templates.
 * Abstracted NUMCHOICES parsing into a function.
 * Changed to limit height of all images.
 *
 * 2001-November-17   Jason Rohrer
 * Added support for add result templates.
 * Made images larger.
 *
 * 2001-November-18   Jason Rohrer
 * Added missing post of response to repository.
 *
 * 2001-November-19   Jason Rohrer
 * Added support for displaying example stats.
 *
 * 2001-December-2   Jason Rohrer
 * Changed page titles.
 * Changed to re-read template files before generating each page.
 * Changed "young" and "old" variable names to "first" and "second".
 *
 * 2001-December-8   Jason Rohrer
 * Changed from KinPic to KinPix.
 *
 * 2001-December-17   Jason Rohrer
 * Made all image borders 0.
 * Changed displayed difficulty to be out of 100.
 * Changed to insert choice stats into choice html.
 *
 * 2001-December-19   Jason Rohrer
 * Added support for a user database.
 * Added a function for parsing url arguments.
 * Changed generateHeader to handle user name and password and serial number
 * links.
 * Added user verification.
 * Added a template for the login form.
 * Added support for creating users.
 * Added user stats to header generation.
 * There is still a bug in the user database.
 *
 * 2001-December-20   Jason Rohrer
 * Made strcmp usage more clear.
 * Fixed a major bug in parseNumChoices().
 * Removed user verification print statements.
 * Fixed a bad indentation.
 * Added a print statement for every processed request.
 * Fixed a string buffer sizing bug in generatePlayPage().
 * Fixed several potentially dangerous string buffer sizing bugs.
 * Fixed a memory leak in processCreateUser().
 *
 * 2002-January-2   Jason Rohrer
 * Fixed typo in generatePageHead interface.
 * Added username and password to generatePageFoot.
 * Added serial numbers to generatePageFoot.
 * Fixed a memory leak in serial number string generation.
 * Fixed a bug in generatePageFoot.
 * Added a function for generating a page from a file determined at run-time.
 * Fixed a bug in verifyUser.
 * Fixed a bug in generateGeneralFilePage.
 * Fixed the title of the page created by generateGeneralFilePage.
 */



#ifndef TEMPLATE_PAGE_GENERATOR_INCLUDED
#define TEMPLATE_PAGE_GENERATOR_INCLUDED



#include "minorGems/io/OutputStream.h"
#include "minorGems/io/file/File.h"
#include "minorGems/io/file/FileInputStream.h"

#include "minorGems/util/SimpleVector.h"
#include "minorGems/system/MutexLock.h"


#include "PageGenerator.h"
#include "ExampleRepository.h"
#include "UserDatabase.h"

#include <stdio.h>
#include <string.h>



/**
 * An implementation of the page generator interface
 * that uses file templates.
 *
 * @author Jason Rohrer
 */
class TemplatePageGenerator : public PageGenerator {

	public:
		

		/**
		 * Constructs a page generator.
		 *
		 * All template files will be destroyed when
		 * this class is destroyed.
		 *
		 * @param inRepository the repository to use.
		 *   Must be destroyed by caller after this class is destroyed.
		 * @param inUserDatabase the user database to use.
		 *   Must be destroyed by caller after this class is destroyed.
		 * @param inMainPageTemplateFile the file to use for
		 *   the body of the main page.
		 * @param inHeaderTemplateFile the file to use for
		 *   all page headers.
		 * @param inFooterTemplateFile the file to use for all
		 *   page footers.
		 * @param inPlayRoundTemplateFile the file to use
		 *   for the body of the play round page.
		 * @param inRoundChoiceTemplateFile the file to use
		 *   for the HTML surrounding each choice on the play round page.
		 * @param inPlayResultTemplateFile the file to use
		 *   for the body of the result page for a play round.
		 * @param inResultChoiceTemplateFile the file to use
		 *   for the HTML surrounding each choice on the play result page.
		 * @param inResultCorrectChoiceTemplateFile the file to use
		 *   for the HTML surrounding the correct choice on the
		 *   play result page.
		 * @param inAddFormTemplateFile the file to use
		 *   for the body of the add pair form page.
		 * @param inAddResultTemplateFile the file to use
		 *   for the body of the result page for adding a pair.
		 * @param inCreateUserFormTemplateFile the file to use
		 *   for the body of the user creation form page.
		 * @param inLoginFormTemplateFile the file to use
		 *   for the body of the user creation form page.
		 */
		TemplatePageGenerator( ExampleRepository *inRepository,
							   UserDatabase *inUserDatabase,
							   File *inMainPageTemplateFile,
							   File *inHeaderTemplateFile,
							   File *inFooterTemplateFile,
							   File *inPlayRoundTemplateFile,
							   File *inRoundChoiceTemplateFile,
							   File *inPlayResultTemplateFile,
							   File *inResultChoiceTemplateFile,
							   File *inResultCorrectChoiceTemplateFile,
							   File *inAddFormTemplateFile,
							   File *inAddResultTemplateFile,
							   File *inCreateUserFormTemplateFile,
							   File *inLoginFormTemplateFile );


		
		~TemplatePageGenerator();

		
		
		// implements the PageGenerator interface
		virtual void generatePage( char *inGetRequestPath,
								   OutputStream *inOutputStream );
		

		
	protected:
		ExampleRepository *mRepository;
		UserDatabase *mUserDatabase;
		
		File *mMainPageTemplateFile;
		File *mHeaderTemplateFile;
		File *mFooterTemplateFile;
		File *mPlayRoundTemplateFile;
		File *mRoundChoiceTemplateFile;
		File *mPlayResultTemplateFile;
		File *mResultChoiceTemplateFile;
		File *mResultCorrectChoiceTemplateFile;
		File *mAddFormTemplateFile;
		File *mAddResultTemplateFile;
		File *mCreateUserFormTemplateFile;
		File *mLoginFormTemplateFile;
		
		char *mMainPageString;
		char *mHeaderString;
		char *mFooterString;

		char *mPlayRoundString;
		char *mRoundChoiceString;

		char *mPlayResultString;
		char *mPlayResultChoiceString;
		char *mPlayResultCorrectChoiceString;
		
		char *mAddFormString;
		char *mAddResultString;

		char *mCreateUserFormString;
		char *mLoginFormString;
		
		/**
		 * Deletes the string buffers if they are not NULL.
		 */
		virtual void deleteStrings();


		
		// vector of examples for which we have not received a response yet
		SimpleVector< Example* > *mPendingExampleVector;
		MutexLock *mVectorLock;

		unsigned long mNextAvailableSerialNumber;
		MutexLock *mPageSerialNumberLock;
		
		
		/**
		 * Generates the head of a page, up to the opening <BODY> tag.
		 *
		 * @param inTitle the title of the page.
		 *   Must be destroyed by caller if non-const.
		 * @param inUsername the \0-terminated name of the current user.
		 *   Must be destroyed by caller if not const.
		 * @param inPassword the \0-terminated password of the current user.
		 *   Must be destroyed by caller if not const.
		 * @param inOutputStream the stream to write the generated head to.
		 *   Must be destroyed by caller.
		 */
		virtual void generatePageHead(
			char *inTitle,
			char *inUsername, char *inPassword,
			OutputStream *inOutputStream );


		
		/**
		 * Generates the foot of a page, starting with the closing </BODY> tag.
		 *
		 * @param inUsername the \0-terminated name of the current user.
		 *   Must be destroyed by caller if not const.
		 * @param inPassword the \0-terminated password of the current user.
		 *   Must be destroyed by caller if not const.
		 * @param inOutputStream the stream to write the generated head to.
		 *   Must be destroyed by caller.
		 */
		virtual void generatePageFoot(
			char *inUsername, char *inPassword,
			OutputStream *inOutputStream );



		/**
		 * Generates a page by interpreting the portion of the get path
		 * before the first '?' as the start of a .html file name.
		 *
		 * @param inGetRequestPath the get request to process.
		 *   Must be destroyed by caller.
		 *   For example, if inGetPath = "/name?username=temp&password=temp",
		 *   the body of the generated page would be read from the file
		 *   name.html.
		 * @param inOutputStream the stream to write the generated page to.
		 *   Must be destroyed by caller.
		 */
		virtual void generateGeneralFilePage(
			char *inGetRequestPath, OutputStream *inOutputStream );

		

		/**
		 * Generates a page stating that a request was malformed.
		 *
		 * @param inOutputStream the stream to write the generated page to.
		 *   Must be destroyed by caller.
		 */
		virtual void generateMalformedRequestPage(
			OutputStream *inOutputStream );



		/**
		 * Processes a login.
		 *
		 * @param inLoginRequest the request to process.
		 *   Must be destroyed by caller.
		 * @param inOutputStream the stream to write the generated page to.
		 *   Must be destroyed by caller.
		 */
		virtual void processLogin(
			char *inLoginRequest, OutputStream *inOutputStream );



		/**
		 * Generates the form page for user creation.
		 *
		 * @param inOutputStream the stream to write the generated page to.
		 *   Must be destroyed by caller.
		 */
		virtual void generateCreateUserForm( OutputStream *inOutputStream );



		/**
		 * Processes a user creation request.
		 *
		 * @param inRequest the request to process.
		 *   Must be destroyed by caller.
		 * @param inOutputStream the stream to write the generated page to.
		 *   Must be destroyed by caller.
		 */
		virtual void processCreateUser( char *inRequest,
										OutputStream *inOutputStream );
		
		
		
		/**
		 * Processes an add request and generates a response page.
		 *
		 * @param inAddRequest the add request to process.
		 *   Must be destroyed by caller.
		 * @param inOutputStream the stream to write the generated page to.
		 *   Must be destroyed by caller.
		 */
		virtual void processAddRequest(
			char *inAddRequest, OutputStream *inOutputStream );


		
		/**
		 * Generates the main page.
		 *
		 * @param inOutputStream the stream to write the generated page to.
		 *   Must be destroyed by caller.
		 */
		virtual void generateMainPage(
			OutputStream *inOutputStream );



		/**
		 * Generates the play page.
		 *
		 *
		 * @param inPlayPageRequest the request to process.
		 *   Must be destroyed by caller.
		 * @param inOutputStream the stream to write the generated page to.
		 *   Must be destroyed by caller.
		 */
		virtual void generatePlayPage(
			char *inPlayPageRequest, OutputStream *inOutputStream );

		
		
		/**
		 * Processes a user play response and generates a solution page.
		 *
		 * @param inPlayResponse the play response request to process.
		 *   Must be destroyed by caller.
		 * @param inOutputStream the stream to write the generated page to.
		 *   Must be destroyed by caller.
		 */
		virtual void processPlayResponse(
			char *inPlayResponse, OutputStream *inOutputStream );


		
		/**
		 * Generates the body of a solution page.
		 *
		 * @param inExample the example the user is responding to.
		 *   Must be destroyed by caller.
		 * @param inChoiceIndex the index amoung the choices
		 *   that the used picked.
		 * @param inOutputStream the stream to write the generated page to.
		 *   Must be destroyed by caller.
		 */
		virtual void generatePlayResultPageBody(
			Example *inExample, int inChoiceIndex,
			OutputStream *inOutputStream );

		

		/**
		 * Generates the add form page.
		 *
		 *
		 * @param inAddFormRequest the request to process.
		 *   Must be destroyed by caller.
		 * @param inOutputStream the stream to write the generated page to.
		 *   Must be destroyed by caller.
		 */
		virtual void generateAddForm(
			char *inAddFormRequest, OutputStream *inOutputStream );



		/**
		 * Verifies that a user exists and has specified the correct
		 * password.
		 *
		 * Generates a complete error page if the user does not exist.
		 *
		 * @param inUsername the \0-terminated name of the current user.
		 *   Must be destroyed by caller if not const.
		 * @param inPassword the \0-terminated password of the current user.
		 *   Must be destroyed by caller if not const.
		 * @param inOutputStream the stream to write the generated page to.
		 *   Must be destroyed by caller.
		 *
		 * @return true iff the user passes verification.
		 *   If verification fails, a complete error page is
		 *   generated to inOutputStream.
		 */
		virtual char verifyUser( char *inUsername, char *inPassword,
								 OutputStream *inOutputStream );

		

		/**
		 * Parses an argument from a form submission URL.
		 *
		 * For example, if inString is
		 *    /login?username=test&password=letmein
		 * A call to parseArgument( inString, "password" ) would return
		 *    "letmein"
		 *
		 * Note that the value of the argument is read after the '='
		 * and up to the first '&', or up to the end of the string.
		 *
		 * @param inString the \0-terminated string to parse an argument from.
		 *    Must be destroyed by caller if non-const.
		 * @param inArgument the \0-terminated argument to search for.
		 *    Must be destroyed by caller if non-const.
		 *
		 * @return the value of the argument, or NULL if the argument
		 *    was not found.  Must be destroyed by caller if non-NULL.
		 */
		virtual char *parseArgument( char *inString, char *inArgument );

		

		/**
		 * Removes explicit hex (for example, %2A) from a string,
		 * replacing it with its ASCII equivalent.
		 *
		 * @param inString the string to process.
		 *   Must be destroyed by caller.
		 *
		 * @return a version of the inString with hex replaced by ascii.
		 *   Must be destroyed by caller.
		 */
		virtual char *removeExplicitHex( char *inString );



		/**
		 * Reads the contents of a file into a c-string.
		 *
		 * @param inFile the file to read.
		 *   Must be destroyed by caller.
		 * @return a c-string containing the contents
		 *   of the file, or NULL if the file does not exist.
		 *   Must be destroyed by caller if non-NULL.
		 */
		virtual char *readFileToString( File *inFile );



		/**
		 * Creates a new string by inserting a string
		 * into an existing string at the first occurence
		 * of a tag.
		 *
		 * All parameters must be destroyed by caller
		 * if non-const.
		 *
		 * @param inField the string containing the
		 *   occurrence of the tag to be replaced.
		 * @param inTag the tag string to replace.
		 * @param inStringToInsert the string
		 *   to insert at the first occurrence of inTag.
		 *
		 * @return a new string equivalent to inField
		 *   with the first occurrence of inTag
		 *   replaced by inStringToInsert.
		 *   Must be destroyed by caller.
		 */
		virtual char *insertStringForTag( char *inField,
										  char *inTag,
										  char *inStringToInsert );


		/**
		 * Replaces all occurrences of a tag with another string.
		 *
		 * All parameters must be destroyed by caller
		 * if non-const.
		 *
		 * @param inField the string containing the
		 *   occurrence of the tags to be replaced.
		 * @param inTag the tag string to replace.
		 * @param inStringToInsert the string
		 *   to insert for each occurrence of inTag.
		 *
		 * @return a new string equivalent to inField
		 *   with all occurrences of inTag
		 *   replaced by inStringToInsert.
		 *   Must be destroyed by caller.
		 */
		virtual char *replaceAllTags( char *inField,
									  char *inTag,
									  char *inStringToInsert );

		
		

		/**
		 * Counts the number of occurrences of a tag in a string
		 *
		 * All parameters must be destroyed by caller
		 * if non-const.
		 *
		 * @param inField the string containing the
		 *   occurrence of the tags to be counted.
		 * @param inTag the tag string to count.
		 *
		 * @return the number of tags in inField.
		 */
		virtual int countTags( char *inField,
								 char *inTag );



		/**
		 * Adds an example to the vector of pending examples.
		 *
		 * Implementation is thread-safe.
		 *
		 * @param inExample the example to add.
		 *   Should not be destroyed by caller until
		 *   after a matching call to removePendingExample.
		 */
		virtual void addPendingExample( Example *inExample );



		/**
		 * Removes an example from the vector of pending examples.
		 *
		 * Implementation is thread-safe.
		 *
		 * @param inExampleSerialNumber the serial number of the
		 *   example to remove.
		 *
		 * @return the removed example, or NULL if no example
		 *   matching inExampleSerialNumber is found.
		 *   Must be destroyed bye caller if non-NULL.
		 */
		virtual Example *removePendingExample(
			unsigned long inExampleSerialNumber );



		/**
		 * Gets the next free page serial number.
		 *
		 * @return the next free serial number.
		 *   Each call to this function will return
		 *   a unique value.
		 */
		virtual unsigned long getPageSerialNumber();



		/**
		 * Parses the number indicated in the <!--#NUMCHOICES=x-->
		 * tag.
		 *
		 * @param inField the string containing the tag.
		 *   Must be destroyed by caller.
		 *
		 * @return the number indicated in the tag.
		 *   0 is returned if the tag is not found.
		 */
		virtual int parseNumChoices( char *inField );



		/**
		 * Reads all template files into their
		 * respective string buffers.
		 *
		 * If the current string buffers are not NULL,
		 * they will be destroyed by this call.
		 */
		virtual void readTemplateFiles();



		/**
		 * Copies a string into a newly created string.
		 *
		 * @param inString the string to copy.  Must be
		 *   destroyed by caller if not const.
		 *
		 * @return a copy of inString.  Must be destroyed by caller.
		 */
		virtual char *copyString( char *inString );


		
	};



inline TemplatePageGenerator::TemplatePageGenerator(
	ExampleRepository *inRepository,
	UserDatabase *inUserDatabase,
	File *inMainPageTemplateFile,
	File *inHeaderTemplateFile,
	File *inFooterTemplateFile,
	File *inPlayRoundTemplateFile,
	File *inRoundChoiceTemplateFile,
	File *inPlayResultTemplateFile,
	File *inResultChoiceTemplateFile,
	File *inResultCorrectChoiceTemplateFile,
	File *inAddFormTemplateFile,
	File *inAddResultTemplateFile,
	File *inCreateUserFormTemplateFile,
	File *inLoginFormTemplateFile )
	: mRepository( inRepository ),
	  mUserDatabase( inUserDatabase ),
	  mPendingExampleVector( new SimpleVector< Example *>() ),
	  mVectorLock( new MutexLock() ),
	  mNextAvailableSerialNumber( 0 ),
	  mPageSerialNumberLock( new MutexLock() ),
	  mMainPageTemplateFile( inMainPageTemplateFile ),
	  mHeaderTemplateFile( inHeaderTemplateFile ),
	  mFooterTemplateFile( inFooterTemplateFile ),
	  mPlayRoundTemplateFile( inPlayRoundTemplateFile ),
	  mRoundChoiceTemplateFile( inRoundChoiceTemplateFile ),
	  mPlayResultTemplateFile( inPlayResultTemplateFile ),
	  mResultChoiceTemplateFile( inResultChoiceTemplateFile ),
	  mResultCorrectChoiceTemplateFile( inResultCorrectChoiceTemplateFile ),
	  mAddFormTemplateFile( inAddFormTemplateFile ),
	  mAddResultTemplateFile( inAddResultTemplateFile ),
	  mCreateUserFormTemplateFile( inCreateUserFormTemplateFile ),
	  mLoginFormTemplateFile( inLoginFormTemplateFile ) {

	mMainPageString = NULL;
	mHeaderString = NULL; 
	mFooterString = NULL; 

	mPlayRoundString = NULL; 
	mRoundChoiceString = NULL; 

	mPlayResultString = NULL; 
	mPlayResultChoiceString = NULL; 
	mPlayResultCorrectChoiceString = NULL;

	mAddFormString = NULL; 
	mAddResultString = NULL;

	mCreateUserFormString = NULL;
	mLoginFormString = NULL;
	
	// read contents of all files into string buffers
	readTemplateFiles();

   	
	}



inline TemplatePageGenerator::~TemplatePageGenerator() {
	deleteStrings();
	
	delete mMainPageTemplateFile;
	delete mHeaderTemplateFile;
	delete mFooterTemplateFile;
	delete mPlayRoundTemplateFile;
	delete mRoundChoiceTemplateFile;
	delete mPlayResultTemplateFile;
	delete mResultChoiceTemplateFile;
	delete mResultCorrectChoiceTemplateFile;
	delete mAddFormTemplateFile;
	delete mAddResultTemplateFile;

	delete mCreateUserFormTemplateFile;
	delete mLoginFormTemplateFile;
	
	int numPendingExamples = mPendingExampleVector->size();
	for( int i=0; i<numPendingExamples; i++ ) {
		Example *example =
			*( mPendingExampleVector->getElement( i ) );

		delete example;
		}
	
	delete mPendingExampleVector;
	delete mVectorLock;			  	
	delete mPageSerialNumberLock;
	}



inline void TemplatePageGenerator::deleteStrings() {
	if( mMainPageString != NULL ) {
		delete [] mMainPageString;
		mMainPageString = NULL;
		}
	if( mHeaderString != NULL ) {
		delete [] mHeaderString;
		mHeaderString = NULL;
		}
	if( mFooterString != NULL ) {
		delete [] mFooterString;
		mFooterString = NULL;
		}

	if( mPlayRoundString != NULL ) {
		delete [] mPlayRoundString;
		mPlayRoundString = NULL;
		}
	if( mRoundChoiceString != NULL ) {
		delete [] mRoundChoiceString;
		mRoundChoiceString = NULL;
		}
	
	if( mPlayResultString != NULL ) {
		delete [] mPlayResultString;
		mPlayResultString = NULL;
		}
	if( mPlayResultChoiceString != NULL ) {
		delete [] mPlayResultChoiceString;
		mPlayResultChoiceString = NULL;
		}
	if( mPlayResultCorrectChoiceString != NULL ) {
		delete [] mPlayResultCorrectChoiceString;
		mPlayResultCorrectChoiceString = NULL;
		}
	
	if( mAddFormString != NULL ) {
		delete [] mAddFormString;
		mAddFormString = NULL;
		}
	if( mAddResultString != NULL ) {
		delete [] mAddResultString;
		mAddResultString = NULL;
		}

	
	if( mCreateUserFormString != NULL ) {
		delete [] mCreateUserFormString;
		mCreateUserFormString = NULL;
		}
	if( mLoginFormString != NULL ) {
		delete [] mLoginFormString;
		mLoginFormString = NULL;
		}
	}


		
inline void TemplatePageGenerator::generatePage(
	char *inGetRequestPath, OutputStream *inOutputStream ) {

	printf( "Processing GET request:  %s\n", inGetRequestPath ); 
	
	// reread template files
	readTemplateFiles();
	
	/*
	  Add request of the form:
	  /submitadd?first=http://first.url&second=http://second.url
	*/

	// look for a "/login" string
	char *pointerToLoginRequest = strstr( inGetRequestPath, "/login" );
	
	// look for a "/submitadd" string
	char *pointerToAddRequest = strstr( inGetRequestPath, "/submitadd" );

	// look for a "/formadd" string
	char *pointerToAddForm = strstr( inGetRequestPath, "/formadd" );

	// look for a "/formcreateuser" string
	char *pointerToCreateUserForm
		= strstr( inGetRequestPath, "/formcreateuser" );

	// look for a "/createuser" string
	char *pointerToCreateUserRequest
		= strstr( inGetRequestPath, "/createuser" );
	
	// look for a "/response" string
	char *pointerToPlayResponse = strstr( inGetRequestPath, "/response" );

	// look for a "/play" string
	char *pointerToPlay = strstr( inGetRequestPath, "/play" );

	
	if( pointerToAddRequest != NULL ) {
		// make sure the "/submitadd" starts at the beginning of our get path
		if( pointerToAddRequest == inGetRequestPath ) {
			processAddRequest( pointerToAddRequest, inOutputStream );
			}
		else {
			printf( "  Bad request\n" );
			generateMalformedRequestPage( inOutputStream );
			}
		}
	else if( pointerToAddForm != NULL ) {
		// make sure the "/formadd" starts at the beginning of our get path
		if( pointerToAddForm == inGetRequestPath ) {
			generateAddForm( pointerToAddForm, inOutputStream );
			}
		else {
			printf( "  Bad request\n" );
			generateMalformedRequestPage( inOutputStream );
			}
		}
	else if( pointerToLoginRequest != NULL ) {
		// make sure the "/login" starts at the beginning of our get path
		if( pointerToLoginRequest == inGetRequestPath ) {
			processLogin( pointerToLoginRequest, inOutputStream );
			}
		else {
			printf( "  Bad request\n" );
			generateMalformedRequestPage( inOutputStream );
			}
		}
	else if( pointerToCreateUserRequest != NULL ) {
		// make sure the "/createuser" starts at the beginning of our get path
		if( pointerToCreateUserRequest == inGetRequestPath ) {
			processCreateUser( pointerToCreateUserRequest, inOutputStream );
			}
		else {
			printf( "  Bad request\n" );
			generateMalformedRequestPage( inOutputStream );
			}
		}
	else if( pointerToCreateUserForm != NULL ) {
		// make sure the "/formcreateuser" starts at the
		// beginning of our get path
		if( pointerToCreateUserForm == inGetRequestPath ) {
			generateCreateUserForm( inOutputStream );
			}
		else {
			printf( "  Bad request\n" );
			generateMalformedRequestPage( inOutputStream );
			}
		}
	else if( pointerToPlayResponse != NULL ) {
		// make sure the "/response" starts at the beginning of our get path
		if( pointerToPlayResponse == inGetRequestPath ) {
			processPlayResponse( pointerToPlayResponse, inOutputStream );
			}
		else {
			printf( "  Bad request\n" );
			generateMalformedRequestPage( inOutputStream );
			}
		}
	else if( pointerToPlay != NULL ) {
		generatePlayPage( pointerToPlay, inOutputStream );
		}
	else if( inGetRequestPath[0] == '/' && strlen( inGetRequestPath ) == 1 ) {
		// generate main page
		generateMainPage( inOutputStream );
		}
	else {
		//default:  a general file page
		generateGeneralFilePage( inGetRequestPath, inOutputStream );
		}
	
	}



inline void TemplatePageGenerator::generateGeneralFilePage(
	char *inGetRequestPath, OutputStream *inOutputStream ) {

	char *username = parseArgument( inGetRequestPath, "username" );
	char *password = parseArgument( inGetRequestPath, "password" );


	if( !verifyUser( username, password, inOutputStream ) ) {
		if( username != NULL ) {
			delete [] username;
			}
		if( password != NULL ) {
			delete [] password;
			}
		// error page already generated
		return;
		}
		
	
	generatePageHead( "KinPix", username, password,
					  inOutputStream );
	

	// temp path shorter than inGetRequestPath by 1 (since we strip '/')
	char *tempPath = new char[ strlen( inGetRequestPath ) ];
	// strip off starting '/'
	strcpy( tempPath, &( inGetRequestPath[1] ) );

	char *pointerToQuestionMark = strstr( tempPath, "?" );

	if( pointerToQuestionMark != NULL ) {
		// prematurely terminate tempPath
		pointerToQuestionMark[0] = '\0';
		}

	char *fileName = new char[ strlen( tempPath ) + 6 ];
	sprintf( fileName, "%s.html", tempPath );

	File *file = new File( NULL, fileName );

	if( file->exists() ) {
	
		char *fileContents = readFileToString( file );

		inOutputStream->write( (unsigned char*)fileContents,
							   strlen( fileContents ) );
		delete [] fileContents;
		}
	
	delete file;
	delete [] fileName;
	delete [] tempPath;
	
	
	generatePageFoot( username, password, inOutputStream );

	delete [] username;
	delete [] password;
	}



inline void TemplatePageGenerator::generateMainPage(
	OutputStream *inOutputStream ) {

	generatePageHead( "KinPix", "", "", inOutputStream );

	if( mMainPageString != NULL ) {

		char *processedPage = insertStringForTag( mMainPageString,
												  "<!--#LOGINFORM-->",
												  mLoginFormString );
		
		inOutputStream->write( (unsigned char*)processedPage,
							   strlen( processedPage ) );

		delete [] processedPage;
		}

	generatePageFoot( "", "", inOutputStream );
	}



char TemplatePageGenerator::verifyUser( char *inUsername, char *inPassword,
										OutputStream *inOutputStream ) {

	if( inUsername != NULL && inPassword != NULL ) {

		if( strcmp( inUsername, "" ) == 0 ) {
			// always pass the blank user
			return true;
			}
		
		User *user = mUserDatabase->getUser( inUsername );

		if( user != NULL ) {
			if( strcmp( user->mPassword, inPassword ) == 0 ) {
				// password matches
				return true;
				}
			}
		}

	// else there was some failure in verification
	
	generatePageHead( "User verification failed", "", "",
					  inOutputStream );

	char *buffer;
	
	if( inUsername != NULL ) {
		buffer = new char[ 100 + strlen( inUsername ) ];
		sprintf( buffer,
				 "<CENTER><H2>Verification failed for user:  %s</H2></CENTER>",
				 inUsername );
		}
	else {
		buffer = new char[ 100];
		sprintf( buffer,
				 "<CENTER><H2>Verification failed.</H2></CENTER>" );
		}

	
	inOutputStream->write( (unsigned char *)buffer, strlen( buffer ) );
		
	delete [] buffer;

	
	// allow the user to log in again
	inOutputStream->write( (unsigned char *)mLoginFormString,
						   strlen( mLoginFormString ) );

	
	generatePageFoot( "", "", inOutputStream );		

	return false;
	}





inline void TemplatePageGenerator::processLogin(
	char *inLoginRequest, OutputStream *inOutputStream ) {

	char *username = parseArgument( inLoginRequest, "username" );
	char *password = parseArgument( inLoginRequest, "password" );


	if( !verifyUser( username, password, inOutputStream ) ) {
		if( username != NULL ) {
			delete [] username;
			}
		if( password != NULL ) {
			delete [] password;
			}
		// error page already generated
		return;
		}
		
	
	generatePageHead( "Login Result", username, password,
					  inOutputStream );

	char *buffer = new char[ 100 + strlen( username ) ];
	sprintf( buffer, "<CENTER><H2>You are now logged in as %s</H2></CENTER>",
			 username );

	inOutputStream->write( (unsigned char *)buffer, strlen( buffer ) );

	delete [] buffer;
	
	generatePageFoot( username, password, inOutputStream );

	
	if( username != NULL ) {
		delete [] username;
		}
	if( password != NULL ) {
		delete [] password;
		}
	}



inline void TemplatePageGenerator::generateCreateUserForm(
	OutputStream *inOutputStream ) {
	
	generatePageHead( "Create an account", "", "", inOutputStream );


	// name field empty
	char *processedPage = insertStringForTag( mCreateUserFormString,
											  "<!--#NAME_FIELD_VALUE-->",
											  "" );

	char *temp = processedPage;

	// age field empty
	processedPage = insertStringForTag( processedPage,
											  "<!--#AGE_FIELD_VALUE-->",
											  "" );
	delete [] temp;


	// female gender checked
	
	temp = processedPage;

	// female CHECKED
	processedPage = insertStringForTag(
		processedPage,
		"<!--#GENDER_FEMALE_CHECKED-->",
		"CHECKED" );

	delete [] temp;
	

	temp = processedPage;

	// male not CHECKED
	processedPage = insertStringForTag(
		processedPage,
		"<!--#GENDER_MALE_CHECKED-->",
		"" );

	delete [] temp;

	
	inOutputStream->write( (unsigned char*)processedPage,
						   strlen( processedPage ) );
			
	generatePageFoot( "", "", inOutputStream );

	delete [] processedPage;
	}



inline void TemplatePageGenerator::processCreateUser(
	char *inRequest,
	OutputStream *inOutputStream ) {

	char *username = parseArgument( inRequest, "username" );
	char *password1 = parseArgument( inRequest, "password1" );
	char *password2 = parseArgument( inRequest, "password2" );
	
	char *genderString = parseArgument( inRequest, "gender" );
	char *ageString = parseArgument( inRequest, "age" );

	int age;
	int numRead = sscanf( ageString, "%d", &age );

	char *errorMessage = new char[ 100 + strlen( username ) ];
	char isError = false;


	if( !isError && strcmp( username, "" ) == 0 ) { 
		sprintf( errorMessage,
				 "Username must be at least 1 character long" );
		isError = true;
		}
	
	if( !isError && mUserDatabase->getUser( username ) != NULL ) {
		// user already exists
		sprintf( errorMessage,
				 "The username %s already exists.", username );
		isError = true;
		}
	
	if( !isError && numRead == 0 ) {
		sprintf( errorMessage, "Age must be a number." );
		isError = true;
		}
	
	
	if( !isError && strcmp( password1, password2 ) != 0 ) {
		// passwords differ
		sprintf( errorMessage, "Passwords do not match." );
		isError = true;
		}

	if( !isError ) {
		
		char *usernameCopy = copyString( username );

		char *passwordCopy = copyString( password1 );

		User *user = new User( usernameCopy, passwordCopy,
							   genderString[0], age );

		// check again if user exists as we add, since
		// our current method is not thread safe, but user addition is
		if( !mUserDatabase->addUser( user ) ) {
			// user already exists
			sprintf( errorMessage,
					 "The username %s already exists.", username );
			isError = true;
		
			delete user; 
			}
		}

	if( !isError ) {
		// log the user in

		// long enough to accomodate 255 char login and pass
		char *loginString = new char[ 100 + strlen( username )
									 + strlen( password1 ) ];

		sprintf( loginString, "/login?username=%s&password=%s",
				 username, password1 );
		
		processLogin( loginString, inOutputStream );

		delete [] loginString;
		}
	else {
		// generate a login form with data in tact

		generatePageHead( "Account creation failed", "", "", inOutputStream );

		
		// refill name field
		char *processedPage = insertStringForTag( mCreateUserFormString,
												  "<!--#NAME_FIELD_VALUE-->",
												  username );
		
		char *temp = processedPage;
	
		// refill age field
		processedPage = insertStringForTag( processedPage,
											"<!--#AGE_FIELD_VALUE-->",
											ageString );
		delete [] temp;
		

		char *checkedString = "CHECKED";
		char *uncheckedString = "";

		char *maleCheckedString = uncheckedString;
		char *femaleCheckedString = checkedString;
		
		if( genderString[0] == 'm' ) {
			maleCheckedString = checkedString;
			femaleCheckedString = uncheckedString;
			}
		
		// female gender
		
		temp = processedPage;
		
		// female CHECKED
		processedPage = insertStringForTag(
			processedPage,
			"<!--#GENDER_FEMALE_CHECKED-->",
			femaleCheckedString );
		
		delete [] temp;
		
		
		temp = processedPage;
		
		// male 
		processedPage = insertStringForTag(
			processedPage,
			"<!--#GENDER_MALE_CHECKED-->",
			maleCheckedString );
		
		delete [] temp;


		temp = processedPage;
	
		// insert error message
		processedPage = insertStringForTag( processedPage,
											"<!--#ERROR_MESSAGE-->",
											errorMessage );
		delete [] temp;

		
		inOutputStream->write( (unsigned char*)processedPage,
							   strlen( processedPage ) );
		
		generatePageFoot( "", "", inOutputStream );
		
		delete [] processedPage;
		
		}

	delete [] username;
	delete [] password1;
	delete [] password2;
	delete [] genderString;
	delete [] ageString;

	delete [] errorMessage;
	}



inline void TemplatePageGenerator::generatePlayPage(
	char *inPlayPageRequest, OutputStream *inOutputStream ) {

	char *username = parseArgument( inPlayPageRequest, "username" );
	char *password = parseArgument( inPlayPageRequest, "password" );

	if( !verifyUser( username, password, inOutputStream ) ) {
		if( username != NULL ) {
			delete [] username;
			}
		if( password != NULL ) {
			delete [] password;
			}
		// error page already generated
		return;
		}
	
	// general play request

	generatePageHead( "Play Round", username, password, inOutputStream );
	
	
	if( mPlayRoundString != NULL ) {

		int numChoices = parseNumChoices( mPlayRoundString );
		
		if( numChoices != 0 ) {
			
			if( mRepository->getNumPairs() < numChoices ) {
				char *buffer = new char[ 100 ];

				sprintf( buffer,
						 "Not enough pairs in database to play.<BR>" );
				inOutputStream->write(
					(unsigned char *)buffer, strlen( buffer ) );

				delete [] buffer;
				}
			else {
				// we have successfully extracted numChoices
				// from the template, and there are enough pairs
				// in the database

				// get an example from the repository with dummies
				Example *example =
					mRepository->getExample( numChoices - 1 );

				int exampleSerialNumber = example->getSerialNumber();
			
			
				// we can leave the <!--#NUMCHOICES=x--> tag
				// in the document since it is a comment

				// first, replace the stimulus tag
				char *stimulusURL = example->getStimulusURL();
			
				char *stimulusHTML =
					new char[ 80 + strlen( stimulusURL ) ];
				sprintf( stimulusHTML, "<IMG SRC=%s BORDER=0 HEIGHT=200>",
						 stimulusURL );
			
				char *processedTemplate =
					insertStringForTag( mPlayRoundString,
										"<!--#STIMULUS-->",
										stimulusHTML );

				delete [] stimulusHTML;


						
						
				char **choiceURLs = example->getChoiceURLs();
			
				for( int i=0; i<numChoices; i++ ) {

					char *choiceURL = choiceURLs[i];
				
					// the tag for this choice
					char *numberedChoiceTag =
						new char[ 40 + numChoices ];
					sprintf( numberedChoiceTag,
							 "<!--#CHOICE%d-->", i + 1 );

					char *choiceHTML =
						new char[ 200 + strlen( choiceURL )
								+ strlen( username )
								+ strlen( password ) ];

					sprintf(
						choiceHTML,
						"<A HREF=response?username=%s&password=%s&example=%d&choice=%d><IMG SRC=%s BORDER=0 HEIGHT=200></A>",
						username, password,
						exampleSerialNumber, i, choiceURL );

					char *choiceHTML2 =
						insertStringForTag( mRoundChoiceString,
											"<!--#CHOICE-->",
											choiceHTML );
							
					char *newProcessedTemplate
						= insertStringForTag( processedTemplate,
											  numberedChoiceTag,
											  choiceHTML2 );
							
					delete [] processedTemplate;

					processedTemplate = newProcessedTemplate;

					delete [] numberedChoiceTag;
					delete [] choiceHTML;
					delete [] choiceHTML2;
					}

				inOutputStream->write(
					(unsigned char *)processedTemplate,
					strlen( processedTemplate ) );

				delete [] processedTemplate;

				addPendingExample( example );
				}
			 
			}
		else {
			printf( "<!--#NUMCHOICES= x --> tag not found in" );
			printf( "\"play round\" template\n" );
			}
		}
	
	generatePageFoot( username, password, inOutputStream );

	
	if( username != NULL ) {
		delete [] username;
		}
	if( password != NULL ) {
		delete [] password;
		}
	}



inline void TemplatePageGenerator::processPlayResponse (
	char *inPlayResponse, OutputStream *inOutputStream ) {

	printf( "Processing play response:  %s\n", inPlayResponse );


	char *username = parseArgument( inPlayResponse, "username" );
	char *password = parseArgument( inPlayResponse, "password" );

	if( !verifyUser( username, password, inOutputStream ) ) {
		if( username != NULL ) {
			delete [] username;
			}
		if( password != NULL ) {
			delete [] password;
			}
		// error page already generated
		return;
		}
	
	
	generatePageHead( "Play Round Response", username, password,
					  inOutputStream );


	char *exampleIntString = parseArgument( inPlayResponse, "example" );
	char *choiceIntString = parseArgument( inPlayResponse, "choice" );
	

	if( exampleIntString != NULL ) {
		
		if( choiceIntString != NULL ) {
			
			unsigned long exampleNumber;
			
			int numRead = sscanf( exampleIntString, "%lu",
								  &exampleNumber );
			
			if( numRead == 1 ) {
				int choiceNumber;
				
				numRead = sscanf( choiceIntString, "%d", &choiceNumber );
				
				if( numRead == 1 ) {
					
					Example *example =
						removePendingExample( exampleNumber );
					
					if( example != NULL ) {
						
						generatePlayResultPageBody( example, choiceNumber,
													inOutputStream );

						mRepository->postResponse( example, choiceNumber );

						if( strcmp( username, "" ) != 0 ) {
							User *user = mUserDatabase->getUser( username );

							user->mNumRoundsPlayed ++;

							if( example->getCorrectResponseIndex()
								== choiceNumber ) {
								user->mNumRoundsCorrect ++;
								}
							}
						
						delete example;
						}
					else {
						char *buffer = new char[40];
						sprintf( buffer,
								 "<FONT COLOR=#000000>" );
						inOutputStream->write( (unsigned char *)buffer,
											   strlen( buffer ) );
						sprintf( buffer, "Your example has expired." );
						inOutputStream->write( (unsigned char *)buffer,
											   strlen( buffer ) );
						sprintf( buffer,
								 "</FONT>" );
						inOutputStream->write( (unsigned char *)buffer,
											   strlen( buffer ) );
						
						delete [] buffer;
						}
					}					
				}
			delete [] choiceIntString;
			}
		delete [] exampleIntString;
		}
		
		
	generatePageFoot( username, password, inOutputStream );

	if( username != NULL ) {
		delete [] username;
		}
	if( password != NULL ) {
		delete [] password;
		}
	}



inline void TemplatePageGenerator::generatePlayResultPageBody (
	Example *inExample, int inChoiceIndex,
	OutputStream *inOutputStream ) {


	// copied from generatePlayPage, with small modifications (yuck!)
	if( mPlayResultString != NULL ) {

		int numChoices = parseNumChoices( mPlayResultString );
		
		if( numChoices != 0 ) {
			
			// we have successfully extracted numChoices
			// from the template
						
			// we can leave the <!--#NUMCHOICES=x--> tag
			// in the document since it is a comment

			// first, replace the stimulus tag
			char *stimulusURL = inExample->getStimulusURL();
			
			char *stimulusHTML =
				new char[ 80 + strlen( stimulusURL ) ];
			sprintf( stimulusHTML, "<IMG SRC=%s BORDER=0 HEIGHT=200>",
					 stimulusURL );
			
			char *processedTemplate =
				insertStringForTag( mPlayResultString,
									"<!--#STIMULUS-->",
									stimulusHTML );

			delete [] stimulusHTML;

			// insert stats
			char *stimulusStatHTML = new char[100];
			sprintf( stimulusStatHTML,
					 "pair difficulty = %.0lf",
					 100 * inExample->getTargetPairWeight() );
					 
			char *newProcessedTemplate =
				insertStringForTag( processedTemplate,
									"<!--#STIMULUSSTATS-->",
									stimulusStatHTML );
			delete [] processedTemplate;
			processedTemplate = newProcessedTemplate;			
			
			delete [] stimulusStatHTML;
			
						
			char **choiceURLs = inExample->getChoiceURLs();

			int correctChoiceIndex = inExample->getCorrectResponseIndex();
			
			for( int i=0; i<numChoices; i++ ) {

				char *choiceURL = choiceURLs[i];
				
				// the tag for this choice
				char *numberedChoiceTag =
					new char[ 40 + numChoices ];
				sprintf( numberedChoiceTag,
						 "<!--#CHOICE%d-->", i + 1 );


				// buffer for stat HTML, which
				// is formatted differently depending
				// on whether currenct choice is correct or not
				char *statHTML = new char[100];

				
				char *choiceHTML =
					new char[ 40 + strlen( choiceURL ) ];

				sprintf( choiceHTML, "<IMG SRC=%s BORDER=0 HEIGHT=200>",
						 choiceURL );

				char *choiceHTML2;
				if( correctChoiceIndex == i ) {
					choiceHTML2 = insertStringForTag(
						mPlayResultCorrectChoiceString,
						"<!--#CHOICE-->",
						choiceHTML );

					// format stats
					sprintf( statHTML, "" );
					}
				else {
					choiceHTML2 = insertStringForTag(
						mPlayResultChoiceString,
						"<!--#CHOICE-->",
						choiceHTML );

					// format stats
					sprintf( statHTML,
							 "difficulty = %.0lf",
							 100 * inExample->getChoiceWeight( i ) );
					}
				

				// insert stat html into choice html
				char *temp = insertStringForTag(
					choiceHTML2,
					"<!--#CHOICESTATS-->",
					statHTML );
				delete [] choiceHTML2;
				choiceHTML2 = temp;

				delete [] statHTML;
				
				
				// insert choice html
				newProcessedTemplate
					= insertStringForTag( processedTemplate,
										  numberedChoiceTag,
										  choiceHTML2 );
							
				delete [] processedTemplate;

				processedTemplate = newProcessedTemplate;

				delete [] numberedChoiceTag;
				delete [] choiceHTML;
				delete [] choiceHTML2;
				}

			inOutputStream->write(
				(unsigned char *)processedTemplate,
				strlen( processedTemplate ) );

			delete [] processedTemplate;
			}
		else {
			printf( "<!--#NUMCHOICES= x --> tag not found in" );
			printf( "\"play result\" template\n" );
			}
		}
	}



inline void TemplatePageGenerator::generateAddForm(
	char *inAddFormRequest, OutputStream *inOutputStream ) {

	char *username = parseArgument( inAddFormRequest, "username" );
	char *password = parseArgument( inAddFormRequest, "password" );

	if( !verifyUser( username, password, inOutputStream ) ) {
		if( username != NULL ) {
			delete [] username;
			}
		if( password != NULL ) {
			delete [] password;
			}
		// error page already generated
		return;
		}
	
	generatePageHead( "Add an Image Pair", username, password,
					  inOutputStream );


	if( mAddFormString != NULL ) {

		// insert username and password into form html

		// insert user name
		char *processedForm = replaceAllTags(
			mAddFormString, "<!--#USERNAME-->", username  );
		
		// insert password
		char *temp = processedForm;
		processedForm = replaceAllTags(
			processedForm, "<!--#PASSWORD-->", password  );
		delete [] temp;
		
		inOutputStream->write( (unsigned char*)processedForm,
							   strlen( processedForm ) );

		delete [] processedForm;
		}
		
	generatePageFoot( username, password, inOutputStream );

	
	if( username != NULL ) {
		delete [] username;
		}
	if( password != NULL ) {
		delete [] password;
		}
	}



inline void TemplatePageGenerator::processAddRequest(
	char *inAddRequest, OutputStream *inOutputStream ) {

	char *strippedRequest = removeExplicitHex( inAddRequest );
	
	printf( "  Processing add request:  %s\n", strippedRequest );

	
	char *pointerToFirst = strstr( strippedRequest, "first=" );

	char *pointerToSecond = strstr( strippedRequest, "second=" );

	char *firstURLCopy = NULL;
	char *secondURLCopy = NULL;
			
	if( pointerToFirst != NULL && pointerToSecond != NULL ) {

		// process first URL first
		char *pointerToURL = strstr( pointerToFirst, "http://" );
		
		if( pointerToURL != NULL ) {

			char *pointerToDelim =
				strchr( pointerToURL, '&' );

			if( pointerToDelim != NULL ) {
				// delimiter present
				// take chars up to delimiter
				pointerToDelim[0] = '\0';
				}
			// else take rest of string
			
			firstURLCopy = new char[ strlen( pointerToURL ) + 1 ];
			strcpy( firstURLCopy, pointerToURL );
			}

		// now second URL
		pointerToURL = strstr( pointerToSecond, "http://" );

		if( pointerToURL != NULL ) {

			char *pointerToDelim =
				strchr( pointerToURL, '&' );

			if( pointerToDelim != NULL ) {
				// delimiter present
				// take chars up to delimiter
				pointerToDelim[0] = '\0';
				}
			// else take rest of string
					
			secondURLCopy = new char[ strlen( pointerToURL ) + 1 ];
			strcpy( secondURLCopy, pointerToURL );
			}
				
		}

	if( firstURLCopy != NULL && secondURLCopy != NULL ) {
		mRepository->addExamplePair( firstURLCopy, secondURLCopy );

		char *username = parseArgument( inAddRequest, "username" );
		char *password = parseArgument( inAddRequest, "password" );

		if( !verifyUser( username, password, inOutputStream ) ) {
		if( username != NULL ) {
			delete [] username;
			}
		if( password != NULL ) {
			delete [] password;
			}
		// error page already generated
		delete []strippedRequest;
		return;
		}
		
		generatePageHead( "Addition Successful", username, password,
						  inOutputStream );

		if( mAddResultString != NULL ) {
		
			char *firstURLReplaced = replaceAllTags( mAddResultString,
													 "<!--#FIRSTURL-->",
													 firstURLCopy );

			char *bothURLsReplaced = replaceAllTags( firstURLReplaced,
													 "<!--#SECONDURL-->",
													 secondURLCopy );

			delete [] firstURLReplaced;

			inOutputStream->write( (unsigned char *)bothURLsReplaced,
								   strlen( bothURLsReplaced ) );

			delete [] bothURLsReplaced;
			}		
				
		generatePageFoot( username, password, inOutputStream );

		if( username != NULL ) {
			delete [] username;
			}
		if( password != NULL ) {
			delete [] password;
			}
		}
	else {
		if( firstURLCopy != NULL ) {
			delete [] firstURLCopy;
			}
		if( secondURLCopy != NULL ) {
			delete [] secondURLCopy;
			}

		printf( "  Bad add request\n" );
		generateMalformedRequestPage( inOutputStream );
		}

	delete [] strippedRequest;
	}



inline void TemplatePageGenerator::generatePageHead(
	char *inTitle,
	char *inUsername, char *inPassword,
	OutputStream *inOutputStream ) {

	if( inUsername == NULL || inPassword == NULL ) {
		printf( "Header:  warning: user name and/or password NULL\n" );
		return;
		}

	
	if( mHeaderString != NULL ) {
				
		char *processedHeader = insertStringForTag(
			mHeaderString, "<!--#TITLE-->", inTitle );


		char *temp = processedHeader;

		
		// use unique serial numbers in all links
		// to force page reloads

		unsigned long pageSerialNumber =
			getPageSerialNumber();
		
		char *serialNumberString = new char[ 40 ];
		sprintf( serialNumberString, "%d", pageSerialNumber );

		processedHeader = replaceAllTags(
			processedHeader, "<!--#SERIALNUMBER-->", serialNumberString  );

		delete [] temp;
		delete [] serialNumberString;
		
		// insert user name
		temp = processedHeader;
		processedHeader = replaceAllTags(
			processedHeader, "<!--#USERNAME-->", inUsername  );
		delete [] temp;

		
		// insert password
		temp = processedHeader;
		processedHeader = replaceAllTags(
			processedHeader, "<!--#PASSWORD-->", inPassword  );
		delete [] temp;


		if( strcmp( inUsername, "" ) != 0 ) {
			// display user stats

			char *statsString = new char[ 200 + strlen( inUsername ) ];

			int rank =  mUserDatabase->getUserRank( inUsername );
			int numUsers = mUserDatabase->getNumUsers();
			
			sprintf( statsString,
				"Logged in as: %s | Rank: %d/%d | <A HREF=\"/\">Logout</A>",
					 inUsername,
					 rank, numUsers );

			temp = processedHeader;
			processedHeader = replaceAllTags(
				processedHeader, "<!--#USER_STATS-->", statsString );
			delete [] temp;
			
			delete [] statsString;
			}
			
		
		inOutputStream->write( (unsigned char*)processedHeader,
							   strlen( processedHeader ) );

		delete [] processedHeader;
		}
	}



inline void TemplatePageGenerator::generatePageFoot(
	char *inUsername, char *inPassword,
	OutputStream *inOutputStream ) {

	if( mFooterString != NULL ) {		
		
		// insert user name
		char *processedFooter = replaceAllTags(
			mFooterString, "<!--#USERNAME-->", inUsername  );
		
		
		// insert password
		char *temp = processedFooter;
		processedFooter = replaceAllTags(
			processedFooter, "<!--#PASSWORD-->", inPassword  );
		delete [] temp;


		// use unique serial numbers in all links
		// to force page reloads

		unsigned long pageSerialNumber =
			getPageSerialNumber();
		
		char *serialNumberString = new char[ 40 ];
		sprintf( serialNumberString, "%d", pageSerialNumber );

		temp = processedFooter;
		processedFooter = replaceAllTags(
			processedFooter, "<!--#SERIALNUMBER-->", serialNumberString  );

		delete [] temp;
		delete [] serialNumberString;
				
		
		inOutputStream->write( (unsigned char*)processedFooter,
							   strlen( processedFooter ) );

		delete [] processedFooter;
		}
	}



inline void TemplatePageGenerator::generateMalformedRequestPage(
	OutputStream *inOutputStream ) {

	generatePageHead( "Malformed Request", "", "", inOutputStream );
	
	char *buffer = new char[200];

	sprintf( buffer,
			 "<FONT COLOR=#000000>" );
	inOutputStream->write( (unsigned char *)buffer,
						   strlen( buffer ) );
	
	sprintf( buffer, "Your browser issued a malformed request." );
	inOutputStream->write( (unsigned char *)buffer, strlen( buffer ) );

	sprintf( buffer,
			 "</FONT>" );
	inOutputStream->write( (unsigned char *)buffer,
						   strlen( buffer ) );
	
	delete [] buffer;

	generatePageFoot( "", "", inOutputStream );
	}



inline char *TemplatePageGenerator::parseArgument(
	char *inString, char *inArgument ) {

	int stringLength = strlen( inString );
	
	char *stringCopy = new char[ stringLength + 1 ];
	strcpy( stringCopy, inString );
	
	
	// argString = inArgument + '='
	int argStringLength = strlen( inArgument ) + 1;
	
	char *argString = new char[ argStringLength + 1 ];

	strcpy( argString, inArgument );

	argString[ argStringLength - 1 ] = '=';
	argString[ argStringLength ] = '\0';

	
	char *pointerToFirst = strstr( stringCopy, argString );
	delete [] argString;

	
	if( pointerToFirst != NULL ) {
		char *pointerToArgValue = &( pointerToFirst[ argStringLength ] );

		char *pointerToArgValueEnd = strchr( pointerToArgValue, '&' );

		if( pointerToArgValueEnd == NULL ) {
			pointerToArgValueEnd = strchr( pointerToArgValue, '\0' );
			}

		pointerToArgValueEnd[0] = '\0';

		int argValueLength = strlen( pointerToArgValue );

		char *argValue = new char[ argValueLength + 1 ];

		strcpy( argValue, pointerToArgValue );

		delete [] stringCopy;
		
		return argValue;
		}
	else {
		delete [] stringCopy;
		return NULL;
		}
	
	}



inline char *TemplatePageGenerator::removeExplicitHex( char *inString ) {

	int stringLength = strlen( inString );

	char *returnString = new char[ stringLength + 1 ];

	int stringIndex = 0;
	int returnStringIndex = 0;

	while( stringIndex < stringLength + 1 ) {

		if( inString[ stringIndex ] != '%' ) {
			// not a hex representation
			returnString[ returnStringIndex ] = inString[ stringIndex ];
			stringIndex++;
			returnStringIndex++;
			}
		else {
			// the start of hex
			char twoChars[2];
			twoChars[0] = inString[ stringIndex + 1 ];
			twoChars[1]= inString[ stringIndex + 2 ];

			char summedChar = 0;

			for( int i=0; i<2; i++ ) {
				int shiftAmount = 4 * ( 1 - i );
				switch( twoChars[i] ) {
					case '0':
						summedChar += 0x0 << shiftAmount;
						break;
					case '1':
						summedChar += 0x1 << shiftAmount;
						break;
					case '2':
						summedChar += 0x2 << shiftAmount;
						break;
					case '3':
						summedChar += 0x3 << shiftAmount;
						break;
					case '4':
						summedChar += 0x4 << shiftAmount;
						break;
					case '5':
						summedChar += 0x5 << shiftAmount;
						break;
					case '6':
						summedChar += 0x6 << shiftAmount;
						break;
					case '7':
						summedChar += 0x7 << shiftAmount;
						break;
					case '8':
						summedChar += 0x8 << shiftAmount;
						break;
					case '9':
						summedChar += 0x9 << shiftAmount;
						break;
					case 'A':
						summedChar += 0xA << shiftAmount;
						break;
					case 'B':
						summedChar += 0xB << shiftAmount;
						break;
					case 'C':
						summedChar += 0xC << shiftAmount;
						break;
					case 'D':
						summedChar += 0xD << shiftAmount;
						break;
					case 'E':
						summedChar += 0xE << shiftAmount;
						break;
					case 'F':
						summedChar += 0xF << shiftAmount;
						break;
					default:
						break;
					}

				}

			returnString[ returnStringIndex ] = summedChar;
			stringIndex += 3;
			returnStringIndex++;			
			}
		}

	return returnString;
	}



inline char *TemplatePageGenerator::readFileToString( File *inFile ) {
		
	if( inFile->exists() ) {
		FileInputStream *inStream = new FileInputStream( inFile );
		
		int length = inFile->getLength();

		char *returnString =
			new char[ length + 1 ];

		inStream->read( (unsigned char*)returnString, length ); 


		// terminate string
		returnString[ length ] = '\0';

		
		delete inStream;
		
		return returnString;
		}
	else {
		printf( "File does not exist:  %s\n",
				inFile->getFullFileName() );
		return NULL;
		}
	}



inline char *TemplatePageGenerator::insertStringForTag(
	char *inField,
	char *inTag,
	char *inStringToInsert ) {

	
	char *returnString = new char[
		strlen( inField )
		- strlen( inTag )
		+ strlen( inStringToInsert ) + 1 ];
	

	char *fieldTagPointer = strstr( inField, inTag );

	if( fieldTagPointer != NULL ) {

		// prematurely terminate field string at start of tag occurence
		char tagStartChar = fieldTagPointer[0];
		fieldTagPointer[0] = '\0';

		// pointer to first char after tag occurrence
		char *fieldPostTagPointer = &( fieldTagPointer[ strlen( inTag ) ] );
		
		sprintf( returnString, "%s%s%s",
				 inField,
				 inStringToInsert,
				 fieldPostTagPointer );

		// restore field string
		fieldTagPointer[0] = tagStartChar;
		}
	else {
		// tag not found
		
		// simply return a copy of inField
		delete [] returnString;
		returnString = new char[ strlen( inField ) + 1 ];
		strcpy( returnString, inField );
		}

	return returnString;
	}



inline char *TemplatePageGenerator::replaceAllTags( char *inField,
													char *inTag,
													char *inStringToInsert ) {

	int numTags = countTags( inField, inTag );

	// replace first tag (this does the right thing whether
	// or not there is a first tag
	char *fieldWithTagsReplaced = insertStringForTag(
		inField, inTag, inStringToInsert  );

	// now replace rest
	for( int i=1; i<numTags; i++ ) {
		char *oldString = fieldWithTagsReplaced;
			
		fieldWithTagsReplaced = insertStringForTag(
				fieldWithTagsReplaced,
				inTag, inStringToInsert  );
		
		delete [] oldString;
		}

	return fieldWithTagsReplaced;
	}



inline int TemplatePageGenerator::countTags(
	char *inField,
	char *inTag ) {

	int tagCount = 0;

	char *fieldTagPointer = strstr( inField, inTag );

	while( fieldTagPointer != NULL ) {
		tagCount++;

		// skip first character of last tag found and search again
		fieldTagPointer = strstr( &( fieldTagPointer[1] ), inTag );
		}

	return tagCount;	
	}



inline void TemplatePageGenerator::addPendingExample( Example *inExample ) {
	mVectorLock->lock();

	mPendingExampleVector->push_back( inExample );
	
	mVectorLock->unlock();
	}



inline Example *TemplatePageGenerator::removePendingExample(
	unsigned long inExampleSerialNumber ) {
	mVectorLock->lock();

	int numPendingExamples = mPendingExampleVector->size();
	for( int i=0; i<numPendingExamples; i++ ) {
		Example *example =
			*( mPendingExampleVector->getElement( i ) );

		if( example->getSerialNumber() == inExampleSerialNumber ) {

			mPendingExampleVector->deleteElement( i );

			mVectorLock->unlock();
			return example;
			}
		
	
		}

	mVectorLock->unlock();

	return NULL;
	}



inline unsigned long TemplatePageGenerator::getPageSerialNumber() {

	mPageSerialNumberLock->lock();

	unsigned long serialNumber = mNextAvailableSerialNumber;

	mNextAvailableSerialNumber++;

	mPageSerialNumberLock->unlock();

	return serialNumber;
	}



inline int TemplatePageGenerator::parseNumChoices( char *inField ) {
	const char *numChoicesTagString = "<!--#NUMCHOICES=";

	int numChoicesTagLength = strlen( numChoicesTagString );
	
	char *startNumChoicesTag = strstr( inField,
									   numChoicesTagString );
	
	if( startNumChoicesTag != NULL ) {
		
		char *startNumChoicesNumber =
			&( startNumChoicesTag[ numChoicesTagLength ] );

		char *startNumChoicesTagClose =
			strstr( startNumChoicesNumber, "-->" );

		if( startNumChoicesTagClose != NULL ) {

			// temporarily terminate string right after
			// number so that we can parse it
			char tempChar = startNumChoicesTagClose[0];
			startNumChoicesTagClose[0] = '\0';
			
			
			int numChoices;

			int numRead = sscanf( startNumChoicesNumber,
								  "%d", &numChoices );

			startNumChoicesTagClose[0] = tempChar;
		
			if( numRead == 1 ) {

				return numChoices;
				}
			else {
				return 0;
				}
	
			}
		else {
			return 0;
			}
		}
	else {
		return 0;
		}
	}



inline void TemplatePageGenerator::readTemplateFiles() {
	deleteStrings();
	
	printf( "Reading template files\n" );

	mMainPageString = readFileToString( mMainPageTemplateFile );
	mHeaderString = readFileToString( mHeaderTemplateFile );
	mFooterString = readFileToString( mFooterTemplateFile );
	
	mPlayRoundString = readFileToString( mPlayRoundTemplateFile );
	mRoundChoiceString = readFileToString( mRoundChoiceTemplateFile );

	mPlayResultString = readFileToString( mPlayResultTemplateFile );
	mPlayResultChoiceString = readFileToString( mResultChoiceTemplateFile );
	mPlayResultCorrectChoiceString =
		readFileToString( mResultCorrectChoiceTemplateFile );

	mAddFormString = readFileToString( mAddFormTemplateFile );
	mAddResultString = readFileToString( mAddResultTemplateFile );

	mCreateUserFormString = readFileToString( mCreateUserFormTemplateFile );
	mLoginFormString = readFileToString( mLoginFormTemplateFile );
	}



inline char *TemplatePageGenerator::copyString( char *inString ) {
	int length = strlen( inString );
	char *copy = new char[ length + 1 ];
	strcpy( copy, inString );

	return copy;
	}



#endif
