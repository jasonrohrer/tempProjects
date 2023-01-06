/*
 * Modification History
 *
 * 2001-November-14   Jason Rohrer
 * Created.
 * Added a missing comment.
 */



#ifndef PAGE_GENERATOR_INCLUDED
#define PAGE_GENERATOR_INCLUDED


#include "minorGems/io/OutputStream.h"



/**
 * An interface for an HTML page generator.
 *
 * @author Jason Rohrer
 */
class PageGenerator {

	public:
		

		
		/**
		 * Virtual destructor to ensure proper destruction of derived classes.
		 */
		virtual ~PageGenerator();



		/**
		 * Generates a page according to a get request path.
		 *
		 * @param inGetRequestPath the path specified
		 *   by the get request.
		 *   Must be destroyed by caller if non-const.
		 * @param inOutputStream the stream to write the generated page to.
		 *   Must be destroyed by caller.
		 */
		virtual void generatePage( char *inGetRequestPath,
								   OutputStream *inOutputStream ) = 0;
		

		
	};



inline PageGenerator::~PageGenerator() {
	// does nothing
	}



#endif
