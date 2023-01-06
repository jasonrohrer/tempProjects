#include <stdio.h>

#include "DataSource.h"



/**
 * Implementation of DataSource that reads from a series of files.
 */
class FileSeriesDataSource : public DataSource {

        
    public:


        
        /**
         * Constructs a DataSource.
         *
         * @param inFiles an array of file handles.
         *   The array and the handles will be destroyed when this class
         *   is destroyed.
         * @param inNumFiles the length of the inFiles array.
         */
        FileSeriesDataSource( FILE **inFiles, int inNumFiles );


        
        ~FileSeriesDataSource();


        
        // implements the DataSource interface
        virtual int readData( unsigned char *inBuffer,
                              int inDesiredByteCount );


    private:
        
        int mNumFiles;
        FILE **mFiles;

        int mCurrentFile;
    };

