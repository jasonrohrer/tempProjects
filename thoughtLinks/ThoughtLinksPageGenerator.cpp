/*
 * Modification History
 *
 * 2002-May-2   Jason Rohrer
 * Created.
 *
 *
 * 2002-May-5   Jason Rohrer
 * Added support for the request and entry form url format.
 */



#include "ThoughtLinksPageGenerator.h"
#include "WebClient.h"

#include "minorGems/io/file/File.h"
#include "minorGems/io/file/FileInputStream.h"
#include "minorGems/io/file/FileOutputStream.h"

#include "minorGems/util/log/AppLog.h"
#include "minorGems/util/stringUtils.h"


#include <string.h>
#include <time.h>
#include <stdio.h>





ThoughtLinksPageGenerator::ThoughtLinksPageGenerator() {

    }



ThoughtLinksPageGenerator::~ThoughtLinksPageGenerator() { ;
    }



void ThoughtLinksPageGenerator::generatePage( char *inGetRequestPath,
                                              OutputStream *inOutputStream ) {

    
    if( strstr( inGetRequestPath, "request" ) ) {

        generateRequestPage( inGetRequestPath, inOutputStream );
        
        }
    else {

        generateEntryForm( inGetRequestPath, inOutputStream );
        
        }

    }



char *ThoughtLinksPageGenerator::getMimeType( char *inGetRequestPath ) {

    // default... return text/html
    return stringDuplicate( "text/html" );
    
    }



void ThoughtLinksPageGenerator::generatePageHead(
    OutputStream *inOutputStream ) {

    char *headerTemplate = loadStringFromTemplateFile( "header.html" );
    if( headerTemplate == NULL ) {
        headerTemplate = stringDuplicate( "" );
        }    
    
    inOutputStream->writeString( headerTemplate );
                    
    delete [] headerTemplate;
    
    }



void ThoughtLinksPageGenerator::generateEntryForm(
    char *inGetRequestPath,
    OutputStream *inOutputStream ) {

    char *entryFormTemplate = loadStringFromTemplateFile( "entryForm.html" );
    if( entryFormTemplate == NULL ) {
        entryFormTemplate = stringDuplicate( "" );
        }    
    
    inOutputStream->writeString( entryFormTemplate );
                    
    delete [] entryFormTemplate;
    }



void ThoughtLinksPageGenerator::generateRequestPage(
    char *inGetRequestPath,
    OutputStream *inOutputStream ) {

    
    //  http://<ADDRESS>/request?from_proxy=<ADDRESS>&
    //  link_from_page_title=<NAME>&link_from_page_url=<URL>&
    //  link_to_page_url=<URL>

    char *toPageURL = extractArgumentRemoveHex( inGetRequestPath,
                                                "link_to_page_url" );

    if( toPageURL != NULL ) {

        if( strstr( toPageURL, "http://" ) == NULL ) {
            
            }
        
        char *pageContents = WebClient::getWebPage( toPageURL );

        
        if( pageContents != NULL ) {

            char *filteredContents =
                fixAllRelativeLinks( pageContents,
                                     toPageURL );
            delete [] pageContents;

            generatePageHead( inOutputStream );
            
            inOutputStream->writeString( filteredContents );

            delete [] filteredContents;
            }
        
        }
    else {
        
        }
    
    }



char *ThoughtLinksPageGenerator::fixAllRelativeLinks(
    char *inPageString,
    char *inPageURL ) {

    char *srcFixed = makeRelativeLinksAbsolute(
        "src=\"",
        "SRC=\"",
        inPageString,
        inPageURL );

    char *backgroundFixed = makeRelativeLinksAbsolute(
        "background=\"",
        "BACKGROUND=\"",
        srcFixed,
        inPageURL );

    delete [] srcFixed;

    char *hrefFixed = makeRelativeLinksAbsolute(
        "href=\"",
        "HREF=\"",
        backgroundFixed,
        inPageURL );

    delete [] backgroundFixed;
    
    return hrefFixed;    
    }



char *ThoughtLinksPageGenerator::makeRelativeLinksAbsolute(
    char *inAnchorLowercase,
    char *inAnchorUppercase,
    char *inPageString,
    char *inPageURL ) {

    
    char *urlCopy;
    char *urlType = "http://";
    if( strstr( inPageURL,  urlType ) == NULL ) { 

        urlCopy = new char[ strlen( inPageURL ) + strlen( urlType ) + 1 ];
        sprintf( urlCopy, "%s%s", urlType, inPageURL ); 
        }
    else {
        urlCopy = stringDuplicate( inPageURL );
        }

    // terminate string at first slash (skipping url type)
    char *slashLocation = strstr( &( urlCopy[ strlen( urlType ) ] ), "/" );
    if( slashLocation != NULL ) {
        slashLocation[0] = '\0';    
        }

    // add slash to create root string
    char *rootString = new char[ strlen( urlCopy ) + 2 ];
    sprintf( rootString, "%s/", urlCopy );


    // repair slash location
    if( slashLocation != NULL ) {
        slashLocation[0] = '/';    
        }

    
    char *relativeRootString;
    if( slashLocation == NULL ) {
        // relative path is same as root path
        relativeRootString = stringDuplicate( rootString );
        }
    else {
        // find last slash in URL
        char *lastSlash = slashLocation;
        char *currentSlash = strstr( &( lastSlash[1] ), "/" );
        while( currentSlash != NULL ) {
            lastSlash = currentSlash;
            currentSlash = strstr( &( lastSlash[1] ), "/" );
            }
        
        // terminate at slash
        lastSlash[0] = '\0';
        // add slash to create relative root string
        relativeRootString = new char[ strlen( urlCopy ) + 2 ];
        sprintf( relativeRootString, "%s/", urlCopy );
        }

    delete [] urlCopy;
    
    
    char found;

    // first, replace all lowercase with uppercase to standardize
    char *srcFixed = replaceAll(
        inPageString, inAnchorLowercase,
        inAnchorUppercase,
        &found );

    char *absoluteAnchor = new char[ strlen( inAnchorUppercase ) + 10 ];
    sprintf( absoluteAnchor, "%s/", inAnchorUppercase );
    
    char *relativeAnchor = new char[ strlen( inAnchorUppercase ) + 10 ];
    sprintf( relativeAnchor, "%s", inAnchorUppercase );
    
    char *httpAnchor = new char[ strlen( inAnchorUppercase ) + 10 ];
    sprintf( httpAnchor, "%shttp://", inAnchorUppercase );


    
    // first, replace absolute sources with a marker
    char *absoluteMarker = "ABSOLUTE_SRC_MARKER";
    char *absolutesMarked = replaceAll(
        srcFixed, absoluteAnchor,
        absoluteMarker,
        &found );
    delete [] srcFixed;
    
    // next, replace the http:// sources with a marker
    char *httpMarker = "HTTP_SRC_MARKER";
    char *httpMarked = replaceAll(
        absolutesMarked, httpAnchor,
        httpMarker,
        &found );

    delete [] absolutesMarked;

    
    // next, replace the relative sources with a marker
    char *relativeMarker = "RELATIVE_SRC_MARKER";
    char *relativesMarked = replaceAll(
        httpMarked, relativeAnchor,
        relativeMarker,
        &found );

    delete [] httpMarked;


    // finally, we can replace the relatives safely
    char *relativeReplacementString =
        new char[ strlen( relativeAnchor ) +
                strlen( relativeRootString ) + 1 ];
    sprintf( relativeReplacementString, "%s%s",
             relativeAnchor, relativeRootString );
    
    char *relativesReplaced = replaceAll(
        relativesMarked, relativeMarker,
        relativeReplacementString,
        &found );
    delete [] relativeReplacementString;
    delete [] relativesMarked;

    // now replace the http:// source markers
    char *httpReplaced = replaceAll(
        relativesReplaced, httpMarker,
        httpAnchor,
        &found );
    delete [] relativesReplaced;

    
    // finally, replace the absolute source markers
    char *absoluteReplacementString =
        new char[ strlen( relativeAnchor ) + strlen( rootString ) + 1 ];
    // use relative anchor before root string
    sprintf( absoluteReplacementString, "%s%s",
             relativeAnchor, rootString );
    char *absolutesReplaced = replaceAll(
        httpReplaced, absoluteMarker,
        absoluteReplacementString,
        &found );
    delete [] absoluteReplacementString;
    delete [] httpReplaced;


    delete [] rootString;
    delete [] relativeRootString;
    

    delete [] absoluteAnchor;
    delete [] relativeAnchor;
    delete [] httpAnchor;

    
    return absolutesReplaced;
    
    }



char *ThoughtLinksPageGenerator::loadStringFromTemplateFile(
    char *inFileName ) {

    
    char *skinName = "default";
    
    char **pathSteps = new char*[2];
    pathSteps[0] = "skins";
    pathSteps[1] = skinName;

    Path *skinDirPath = new Path( pathSteps, 2, false );

    File *skinFile = new File( skinDirPath, inFileName );

    delete [] pathSteps;

    
    char *returnString;
    if( !skinFile->exists() ) {        
        returnString = NULL;
        }
    else {

        FileInputStream *inStream = new FileInputStream( skinFile );

        int length = skinFile->getLength();

        char *buffer = new char[ length + 1 ];

        int numRead = inStream->read( (unsigned char *)buffer, length );

        if( numRead != length ) {
            returnString = NULL;

            delete [] buffer;
            }
        else {
            buffer[length] = '\0';
            returnString = buffer;
            }

        delete inStream;        
        }
    
    delete skinFile;
    
    return returnString;
    }



char *ThoughtLinksPageGenerator::replaceOnce( char *inHaystack, char *inTarget,
                                              char *inSubstitute,
                                              char *outFound ) {

    char *haystackCopy = stringDuplicate( inHaystack );
    
	char *fieldTargetPointer = strstr( haystackCopy, inTarget );


    if( fieldTargetPointer == NULL ) {
        // target not found
        *outFound = false;
        return haystackCopy;
        }
    else {
        // target found

		// prematurely terminate haystack copy string at
        // start of target occurence
        // (okay, since we're working with a copy of the haystack argument)
		fieldTargetPointer[0] = '\0';

		// pointer to first char after target occurrence
		char *fieldPostTargetPointer =
            &( fieldTargetPointer[ strlen( inTarget ) ] );

        char *returnString = new char[
            strlen( inHaystack )
            - strlen( inTarget )
            + strlen( inSubstitute ) + 1 ];
        
		sprintf( returnString, "%s%s%s",
				 haystackCopy,
				 inSubstitute,
				 fieldPostTargetPointer );

		delete [] haystackCopy;

        *outFound = true;
        return returnString;
		}
    
    }



char *ThoughtLinksPageGenerator::replaceAll( char *inHaystack, char *inTarget,
                                             char *inSubstitute,
                                             char *outFound ) {

    // repeatedly replace once until replacing fails
    
    char lastFound = true;
    char atLeastOneFound = false;
    char *returnString = stringDuplicate( inHaystack );

    while( lastFound ) {

        char *nextReturnString =
            replaceOnce( returnString, inTarget, inSubstitute, &lastFound );

        delete [] returnString;
        
        returnString = nextReturnString;

        if( lastFound ) {
            atLeastOneFound = true;
            }
        }

    *outFound = atLeastOneFound;
    
    return returnString;    
    }



char *ThoughtLinksPageGenerator::extractUpToTarget(
    char *inHaystack, char *inTarget, char *outFound ) {

    char *haystackCopy = stringDuplicate( inHaystack );

    char *fieldTargetPointer = strstr( haystackCopy, inTarget );
    
    if( fieldTargetPointer != NULL ) {
        // terminate string at start of target
        fieldTargetPointer[0] = '\0';

        *outFound = true;
        }
    else {
        *outFound = false;
        }

    // trim string
    char *returnString = stringDuplicate( haystackCopy );

    delete [] haystackCopy;

    
    return returnString;
    }



char *ThoughtLinksPageGenerator::extractAfterTarget(
    char *inHaystack, char *inTarget, char *outFound ) {

    char *haystackCopy = stringDuplicate( inHaystack );

    char *fieldTargetPointer = strstr( haystackCopy, inTarget );
    
    char *afterTargetPointer;
    
    if( fieldTargetPointer != NULL ) {

        afterTargetPointer = &( fieldTargetPointer[ strlen( inTarget ) ] );

        *outFound = true;
        }
    else {
        // end of string
        afterTargetPointer = &( haystackCopy[ strlen( haystackCopy ) ] );
        
        *outFound = false;
        }

    // trim string
    char *returnString = stringDuplicate( afterTargetPointer );

    delete [] haystackCopy;

    
    return returnString;
    }



char *ThoughtLinksPageGenerator::extractArgument( char *inHaystack,
                                                  char *inArgName ) {

    char *argNameWithEquals = new char[ strlen( inArgName ) + 2 ];

    sprintf( argNameWithEquals, "%s%s",
             inArgName, "=" );

    char *haystackCopy = stringDuplicate( inHaystack );
    
    char *pointerToArgStart = strstr( haystackCopy, argNameWithEquals );
    
    if( pointerToArgStart == NULL ) {
        delete [] haystackCopy;
        delete [] argNameWithEquals;
        return NULL;
        }
    else {
        char *pointerToArgEnd = strstr( pointerToArgStart, "&" );
        if( pointerToArgEnd != NULL ) {
            // terminate string at arg end
            pointerToArgEnd[0] = '\0';
            }
        // else entire remainder of string is argument

        char *pointerToArgValue =
            &( pointerToArgStart[ strlen( argNameWithEquals ) ] );


        // trim string
        char *returnString = stringDuplicate( pointerToArgValue );

        delete [] argNameWithEquals;
        delete [] haystackCopy;
        return returnString;
        }
    
    }



char *ThoughtLinksPageGenerator::extractArgumentRemoveHex( char *inHaystack,
                                                           char *inArgName ) {
    char *extractedArg = extractArgument( inHaystack, inArgName );

    if( extractedArg != NULL ) {

        char *convertedArg = removeExplicitHex( extractedArg );

        delete [] extractedArg;

        return convertedArg;
        }
    else {
        return NULL;
        }
    }



char *ThoughtLinksPageGenerator::removeExplicitHex( char *inString ) {

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

    // now run through string and replace any + characters with spaces
    char *plusLocation = strstr( returnString, "+" );
    while( plusLocation != NULL ) {
        plusLocation[0] = ' ';
        plusLocation = strstr( plusLocation, "+" );
        }
    
	return returnString;
	}



char *ThoughtLinksPageGenerator::hexEncode( char *inString ) {

    // for now, just replace spaces

    char found;
    
    return replaceAll( inString, " ", "+", &found ); 
    
    }
