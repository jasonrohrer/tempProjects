/*
 * Modification History
 *
 * 2001-November-6   Jason Rohrer
 * Created.
 *
 * 2001-November-7   Jason Rohrer
 * Added preprocessor protection against multiple includes.
 */



#ifndef PARALLEL_PORT_INCLUDED
#define PARALLEL_PORT_INCLUDED


/**
 * Class for outputting bytes to the parallel port.
 *
 * @author Jason Rohrer
 */
class ParallelPort {

	public:


		
		/**
		 * Sets the current output byte.
		 *
		 * @param inByte the byte to set.
		 */
		static void setByte( char inByte );


		
	};


#endif
