#include "streamFileTracker.h"

#include "minorGems/util/SimpleVector.h"
#include "minorGems/util/stringUtils.h"


SimpleVector<char *> streamFileNames;

int currentFileIndex = -1;

// track the first file in our current playback channel
// we should loop back to this file if we exhaust all files in the stream
int startOfCurrentChannel = -1;

int uniqueFileNumber = 0;

char streamingFromLocalFileSet = false;



void clearTracker() {
    int numFiles = streamFileNames.size();

    for( int i=0; i<numFiles; i++ ) {
        delete [] *( streamFileNames.getElement( i ) );
        }
    streamFileNames.deleteAll();

    currentFileIndex = -1;
    uniqueFileNumber = 0;
    }



void setLocalFileSetStreaming( char inLocal ) {
    streamingFromLocalFileSet = inLocal;
    }



char getLocalFileSetStreaming() {
    return streamingFromLocalFileSet;
    }



char *getUniqueFileName() {

    char *tryName = autoSprintf( "%d.data", uniqueFileNumber );

    FILE *tryFile = fopen( tryName, "rb" );

    while( tryFile != NULL ) {
        // file exists

        delete [] tryName;
        fclose( tryFile );

        // skip to next number
        uniqueFileNumber++;
        
        tryName = autoSprintf( "%d.data", uniqueFileNumber );

        tryFile = fopen( tryName, "rb" );
        }

    // found name that does not exist

    return tryName;
    }



void addFileName( char *inFileName ) {
    streamFileNames.push_back( stringDuplicate( inFileName ) );

    if( currentFileIndex == -1 ) {
        // our first file
        currentFileIndex = 0;
        startOfCurrentChannel = 0;
        }
    }



int getCurrentFileIndex() {
    return currentFileIndex;
    }



void setStartOfCurrentChannel( int inIndex ) {
    startOfCurrentChannel = inIndex;
    }



int getStartOfCurrentChannel() {
    return startOfCurrentChannel;
    }



void stepCurrentIndexToNextFile() {
    if( streamFileNames.size() > currentFileIndex + 1 ) {
        currentFileIndex ++;
        }
    else {
        // wrap back to first file from our current channel
        currentFileIndex = startOfCurrentChannel;
        }
    }



void jumpCurrentIndexToEnd() {
    currentFileIndex = streamFileNames.size() - 1;
    }



int getMaxFileIndex() {
    return streamFileNames.size() - 1;
    }



char *getFileName( int inIndex ) {
    return stringDuplicate( *( streamFileNames.getElement( inIndex ) ) );
    }


