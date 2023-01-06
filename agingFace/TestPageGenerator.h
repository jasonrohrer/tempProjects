/*
 * Modification History
 *
 * 2001-November-14   Jason Rohrer
 * Created.
 * Changed to test dynamic content ideas.
 */



#ifndef TEST_PAGE_GENERATOR_INCLUDED
#define TEST_PAGE_GENERATOR_INCLUDED



#include "minorGems/io/OutputStream.h"
#include "PageGenerator.h"
#include "ExampleRepository.h"

#include <stdio.h>
#include <string.h>

/**
 * A test implementation of the page generator interface.
 *
 * @author Jason Rohrer
 */
class TestPageGenerator : public PageGenerator {

	public:
		

		/**
		 * Constructs a page generator.
		 *
		 * @param inRepository the repository to use.
		 *   Must be destroyed by caller after this class is destroyed.
		 */
		TestPageGenerator( ExampleRepository *inRepository );
		
		
		
		// implements the PageGenerator interface
		virtual void generatePage( char *inGetRequestPath,
								   OutputStream *inOutputStream );
		

		
	protected:
		ExampleRepository *mRepository;


		
		/**
		 * Generates the head of a page, up to the opening <BODY> tag.
		 *
		 * @param inTitle the title of the page.
		 *   Must be destroyed by caller if non-const.
		 *
		 * @param inOutputStream the stream to write the generated head to.
		 *   Must be destroyed by caller.
		 */
		virtual void generatePageHead(
			char *inTitle, OutputStream *inOutputStream );


		
		/**
		 * Generates the foot of a page, starting with the closing </BODY> tag.
		 *
		 * @param inOutputStream the stream to write the generated head to.
		 *   Must be destroyed by caller.
		 */
		virtual void generatePageFoot(
			OutputStream *inOutputStream );

		

		/**
		 * Generates a page stating that a request was malformed.
		 *
		 * @param inOutputStream the stream to write the generated page to.
		 *   Must be destroyed by caller.
		 */
		virtual void generateMalformedRequestPage(
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
		 * Generates the play page.
		 *
		 * @param inOutputStream the stream to write the generated page to.
		 *   Must be destroyed by caller.
		 */
		virtual void generatePlayPage(
			OutputStream *inOutputStream );



		/**
		 * Generates the add form page.
		 *
		 * @param inOutputStream the stream to write the generated page to.
		 *   Must be destroyed by caller.
		 */
		virtual void generateAddForm(
			OutputStream *inOutputStream );



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

		
		
	};



inline TestPageGenerator::TestPageGenerator(
	ExampleRepository *inRepository )
	: mRepository( inRepository ) {

	}
		
		
		
inline void TestPageGenerator::generatePage(
	char *inGetRequestPath, OutputStream *inOutputStream ) {

	/*
	  Add request of the form:
	  /submitadd?young=http://young.url&old=http://old.url
	*/
	
	// look for a "/submitadd" string
	char *pointerToAddRequest = strstr( inGetRequestPath, "/submitadd" );

	// look for a "/formadd" string
	char *pointerToAddForm = strstr( inGetRequestPath, "/formadd" );

	
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
			generateAddForm( inOutputStream );
			}
		else {
			printf( "  Bad request\n" );
			generateMalformedRequestPage( inOutputStream );
			}
		}
	else {
		generatePlayPage( inOutputStream );
		}
	
	}



inline void TestPageGenerator::generatePlayPage(
	OutputStream *inOutputStream ) {

	// general play request

	generatePageHead( "Play Round", inOutputStream );	
	
	char *buffer = new char[200];

	int numDummies = 3;
	
	if( mRepository->getNumPairs() < numDummies + 1 ) {
		sprintf( buffer, "Not enough pairs in database to play.<BR>" );
		inOutputStream->write( (unsigned char *)buffer, strlen( buffer ) );
		}
	else {
		
		// get an example from the repository with 3 dummies
		Example *example = mRepository->getExample( 3 );
		
		sprintf( buffer, "<CENTER><TABLE BORDER=0><TR>" );
		inOutputStream->write( (unsigned char *)buffer, strlen( buffer ) );

		int numChoices = example->getNumChoices();
		char **choices = example->getChoiceURLs();

		for( int i=0; i<numChoices; i++ ) {
			char *buffer2 = new char[ 200 + 2 * strlen( choices[i] ) ];
		
			sprintf( buffer2, "<TD><A HREF=\"%s\">%s</A></TD>",
					 choices[i], choices[i] );
			inOutputStream->write( (unsigned char *)buffer2,
								   strlen( buffer2 ) );
			delete [] buffer2;
			}

		sprintf( buffer, "</TR></TABLE></CENTER><BR>" );
		inOutputStream->write( (unsigned char *)buffer, strlen( buffer ) );

	
		sprintf( buffer, "<CENTER><TABLE BORDER=0><TR>" );
		inOutputStream->write( (unsigned char *)buffer, strlen( buffer ) );

		char *stimulus = example->getStimulusURL();
	
		char *buffer2 = new char[ 200 + 2 * strlen( stimulus ) ];
		
		sprintf( buffer2, "<TD><A HREF=\"%s\">%s</A></TD>",
				 stimulus, stimulus );
		inOutputStream->write( (unsigned char *)buffer2,
							   strlen( buffer2 ) );
		delete [] buffer2;

		sprintf( buffer, "</TR></TABLE></CENTER><BR>" );
		inOutputStream->write( (unsigned char *)buffer, strlen( buffer ) );
		}

	
	delete [] buffer;

	generatePageFoot( inOutputStream );

	
	}



inline void TestPageGenerator::generateAddForm(
	OutputStream *inOutputStream ) {

	generatePageHead( "Add an Image Pair", inOutputStream );

	/*
	  Sample google form
	  
	<form action="/search" name=f><table cellspacing=0 cellpadding=0><tr><td width=75>&nbsp;</td><td align=center><input maxLength=256 size=55 name=q value=""><br><input type=submit value="Google Search" name=btnG><input type=submit value="I'm Feeling Lucky" name=btnI></td><td valign=top nowrap><font size=-2>&nbsp;&#149;&nbsp;<a href=/advanced_search>Advanced&nbsp;Search</a><br>&nbsp;&#149;&nbsp;<a href=/preferences>Preferences</a><br>&nbsp;&#149;&nbsp;<a href=/language_tools>Language Tools</a></font></td></tr></table></form>
	*/

	char *buffer = new char[200];

	sprintf( buffer, "<FORM ACTION=/submitadd>" );
	inOutputStream->write( (unsigned char *)buffer,
						   strlen( buffer ) );

	sprintf( buffer, "URL of <B>young</B> image:  " );
	inOutputStream->write( (unsigned char *)buffer,
						   strlen( buffer ) );
	sprintf(
		buffer,
		"<INPUT MAXLENGTH=256 SIZE=55 NAME=young VALUE=\"http://\"><BR>" );
	inOutputStream->write( (unsigned char *)buffer,
						   strlen( buffer ) );

	sprintf( buffer, "URL of <B>old</B> image:  " );
	inOutputStream->write( (unsigned char *)buffer,
						   strlen( buffer ) );
	sprintf(
		buffer,
		"<INPUT MAXLENGTH=256 SIZE=55 NAME=old VALUE=\"http://\"><BR>" );
	inOutputStream->write( (unsigned char *)buffer,
						   strlen( buffer ) );

	sprintf(
		buffer, "<INPUT TYPE=submit VALUE=\"Submit\">" );
	inOutputStream->write( (unsigned char *)buffer,
						   strlen( buffer ) );
	
	sprintf( buffer, "</FORM>" );
	inOutputStream->write( (unsigned char *)buffer,
						   strlen( buffer ) );

	
	delete [] buffer;
	
	generatePageFoot( inOutputStream );
	}



inline void TestPageGenerator::processAddRequest(
	char *inAddRequest, OutputStream *inOutputStream ) {

	char *strippedRequest = removeExplicitHex( inAddRequest );
	
	printf( "  Processing add request:  %s\n", strippedRequest );

	
	char *pointerToYoung = strstr( strippedRequest, "young=" );

	char *pointerToOld = strstr( strippedRequest, "old=" );

	char *youngURLCopy = NULL;
	char *oldURLCopy = NULL;
			
	if( pointerToYoung != NULL && pointerToOld != NULL ) {

		// process young URL first
		char *pointerToURL = strstr( pointerToYoung, "http://" );
		
		if( pointerToURL != NULL ) {

			char *pointerToDelim =
				strchr( pointerToURL, '&' );

			if( pointerToDelim != NULL ) {
				// delimiter present
				// take chars up to delimiter
				pointerToDelim[0] = '\0';
				}
			// else take rest of string
					
			youngURLCopy = new char[ strlen( pointerToURL ) + 1 ];
			strcpy( youngURLCopy, pointerToURL );
			}

		// now old URL
		pointerToURL = strstr( pointerToOld, "http://" );

		if( pointerToURL != NULL ) {

			char *pointerToDelim =
				strchr( pointerToURL, '&' );

			if( pointerToDelim != NULL ) {
				// delimiter present
				// take chars up to delimiter
				pointerToDelim[0] = '\0';
				}
			// else take rest of string
					
			oldURLCopy = new char[ strlen( pointerToURL ) + 1 ];
			strcpy( oldURLCopy, pointerToURL );
			}
				
		}

	if( youngURLCopy != NULL && oldURLCopy != NULL ) {
		mRepository->addExamplePair( youngURLCopy, oldURLCopy );

		generatePageHead( "Addition Successful", inOutputStream );

		// make buffer large enough
		char *buffer = new char[ 200
							   + 2 * strlen( youngURLCopy )
							   + 2 * strlen( oldURLCopy ) ];
		
		sprintf( buffer, "The following pair has been added:<BR>" );
		inOutputStream->write( (unsigned char *)buffer,
							   strlen( buffer ) );

		sprintf( buffer,
				 "<B>Young image</B>:  <A HREF=%s>%s</A><BR>",
				 youngURLCopy, youngURLCopy );
		inOutputStream->write( (unsigned char *)buffer,
							   strlen( buffer ) );

		sprintf( buffer,
				 "<B>Old image</B>:  <A HREF=%s>%s</A><BR>",
				 oldURLCopy, oldURLCopy );
		inOutputStream->write( (unsigned char *)buffer,
							   strlen( buffer ) );

		int numPairs = mRepository->getNumPairs();

		sprintf( buffer,
				 "<B>Number of pairs in repository</B>:  %d",
				 numPairs );
		inOutputStream->write( (unsigned char *)buffer,
							   strlen( buffer ) );
				
		delete [] buffer;

				
		generatePageFoot( inOutputStream );
		}
	else {
		if( youngURLCopy != NULL ) {
			delete [] youngURLCopy;
			}
		if( oldURLCopy != NULL ) {
			delete [] oldURLCopy;
			}

		printf( "  Bad add request\n" );
		generateMalformedRequestPage( inOutputStream );
		}

	delete [] strippedRequest;
	}



inline void TestPageGenerator::generatePageHead(
	char *inTitle, OutputStream *inOutputStream ) {

	// make our buffer long enough for the title and our tags 
	char *buffer = new char[ 200 + strlen( inTitle ) ];

	sprintf( buffer, "<HTML>" );
	inOutputStream->write( (unsigned char *)buffer, strlen( buffer ) );

	sprintf( buffer, "<HEAD><TITLE>%s</TITLE></HEAD>", inTitle );
	inOutputStream->write( (unsigned char *)buffer, strlen( buffer ) );
		
	sprintf( buffer, "<BODY>" );
	inOutputStream->write( (unsigned char *)buffer, strlen( buffer ) );
	
	delete [] buffer;
	}



inline void TestPageGenerator::generatePageFoot(
	OutputStream *inOutputStream ) {
	
	char *buffer = new char[200];

	sprintf( buffer, "<HR>--<A HREF=\"/\">Play Round</A>--" );
	inOutputStream->write( (unsigned char *)buffer, strlen( buffer ) );

	sprintf( buffer, "<A HREF=\"/formadd\">Add an Image Pair </A>--" );
	inOutputStream->write( (unsigned char *)buffer, strlen( buffer ) );
	
	sprintf( buffer, "</BODY>" );
	inOutputStream->write( (unsigned char *)buffer, strlen( buffer ) );

	sprintf( buffer, "</HTML>" );
	inOutputStream->write( (unsigned char *)buffer, strlen( buffer ) );
	
	delete [] buffer;
	}



inline void TestPageGenerator::generateMalformedRequestPage(
	OutputStream *inOutputStream ) {

	generatePageHead( "Malformed Request", inOutputStream );
	
	char *buffer = new char[200];

	sprintf( buffer, "Your browser issued a malformed request." );
	inOutputStream->write( (unsigned char *)buffer, strlen( buffer ) );

	delete [] buffer;

	generatePageFoot( inOutputStream );
	}



inline char *TestPageGenerator::removeExplicitHex( char *inString ) {

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



#endif


