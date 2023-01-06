#ifndef DATA_SOURCE_INCLUDED
#define DATA_SOURCE_INCLUDED



/**
 * Abstract interface for a source of data.
 */
class DataSource {

        
    public:


        
        virtual ~DataSource() {
            };


        
        /**
         * Reads data into a buffer.
         *
         * @param inBuffer the buffer to put the read data into.
         * @param inDesiredByteCount the desired number of bytes to read.
         *
         * @return the number of bytes actually read, or -1 on an error.
         *   Returns -2 if a format change will start with the next call
         *   to readData
         */
        virtual int readData( unsigned char *inBuffer,
                              int inDesiredByteCount ) = 0;

        
        
    };



#endif
