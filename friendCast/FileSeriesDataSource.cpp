#include "FileSeriesDataSource.h"



FileSeriesDataSource::FileSeriesDataSource( FILE **inFiles, int inNumFiles )
    : mNumFiles( inNumFiles ), mFiles( inFiles ),
      mCurrentFile( 0 ) {

    }


        
FileSeriesDataSource::~FileSeriesDataSource() {
    for( int i=0; i<mNumFiles; i++ ) {
        fclose( mFiles[i] );
        }
    delete [] mFiles;
    }



int FileSeriesDataSource::readData( unsigned char *inBuffer,
                                    int inDesiredByteCount ) {

    // read a new buffer of data from the file
    int size = fread( inBuffer, 1, inDesiredByteCount,
                      mFiles[ mCurrentFile ] );

    if( size == 0 ) {
        // end of file

        // go back to start
        fseek( mFiles[ mCurrentFile ], 0, SEEK_SET );
        
        // go on to next file
        mCurrentFile++;
        if( mCurrentFile >= mNumFiles ) {
            mCurrentFile = 0;
            }

        // tell caller about format change that will start with
        // next readData call
        return -2;
        }

    return size;
    }

