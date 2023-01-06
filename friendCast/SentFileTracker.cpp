
#include "SentFileTracker.h"

#include "minorGems/util/stringUtils.h"



SentFileTracker::~SentFileTracker() {
    clearTracker();
    }



char SentFileTracker::sentFileYet( char *inFileName ) {
    int numFiles = mFileNames.size();
    char found = false;
    for( int i=0; i<numFiles && !found; i++ ) {

        char *name = *( mFileNames.getElement( i ) );
        
        if( strcmp( inFileName, name ) == 0 ) {
            found = true;
            }
        }

    if( !found ) {
        mFileNames.push_back( stringDuplicate( inFileName ) );
        }

    return found;
    }



void SentFileTracker::clearTracker() {
    int numFiles = mFileNames.size();
    for( int i=0; i<numFiles; i++ ) {
        delete [] *( mFileNames.getElement( i ) );
        }
    mFileNames.deleteAll();
    }



