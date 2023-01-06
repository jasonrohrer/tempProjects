/*
 * Modification History
 *
 * 2001-November-14   Jason Rohrer
 * Created.
 * Added a function for getting the number of pairs in the repository.
 */



#ifndef EXAMPLE_REPOSITORY_INCLUDED
#define EXAMPLE_REPOSITORY_INCLUDED



#include "Example.h"



/**
 * Interface for a set of aging face examples.
 *
 * @author Jason Rohrer
 */
class ExampleRepository {

	public:



		/**
		 * Virtual destructor to ensure proper destruction of derived classes.
		 */
		virtual ~ExampleRepository();


		
		/**
		 * Gets the number of example pairs in the repository.
		 *
		 * @return the number of pairs in the repository.
		 */
		virtual int getNumPairs() = 0;

		
		
		/**
		 * Adds an example pair to the repository.
		 *
		 * @param inYoungFaceLink a c-string URL for the young face.
		 *   Will be destroyed when this class is destroyed.
		 * @param inOldFaceURL a c-string URL for the old face.
		 *   Will be destroyed when this class is destroyed.
		 */
		virtual void addExamplePair( char *inYoungFaceURL,
									 char *inOldFaceURL ) = 0;

		

		/**
		 * Gets an example from the repository.
		 *
		 * @param inNumDummyChoices the number of dummy
		 *   choices to include along with the target response choice.
		 *
		 * @return an example.
		 *   Must be destroyed by caller.
		 */
		virtual Example *getExample( int inNumDummyChoices ) = 0;



		/**
		 * Post the opponent's response to a given example.
		 *
		 * @param inExample the example the opponent has responded to.
		 *   Must be destroyed by caller.
		 * @param inOpponentChoiceIndex the response index chosen
		 *   by the opponent.
		 */
		virtual void postResponse( Example *inExample,
								   int inOpponentChoiceIndex ) = 0;
		
		
		
	};



inline ExampleRepository::~ExampleRepository() {
	// does nothing
	}



#endif
