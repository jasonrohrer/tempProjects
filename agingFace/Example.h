/*
 * Modification History
 *
 * 2001-November-14   Jason Rohrer
 * Created.
 *
 * 2001-November-15   Jason Rohrer
 * Added serial numbers.
 *
 * 2001-November-19   Jason Rohrer
 * Added statistics.
 */



#ifndef EXAMPLE_INCLUDED
#define EXAMPLE_INCLUDED



/**
 * A container for an aging face matching example.
 *
 * @author Jason Rohrer
 */
class Example {

	public:


		
		/**
		 * Constructs an example.
		 *
		 * @param inStimulusURL a non-const c-string URL for the
		 *   stimulus face.
		 *   Will be destroyed when this class is destroyed.
		 * @param inNumChoices the number of response choices.
		 *   Must be at least 1.
		 * @param inChoiceURLs an array of non-const c-string URLs for
		 *   the response choice faces.
		 *   Will be destroyed when this class is destroyed.
		 * @param inCorrectResponseIndex the
		 *   index in inChoiceURLs of the correct response face.
		 * @param inSerialNumber a unique (system-wide) serial
		 *   number for this example.  Should be different
		 *   from the serial number of any other example in this
		 *   run of the system.
		 * @param inTargetPairWeight the weight of the target
		 *   pair in this example.
		 * @param inChoiceWeights the weight of each choice in this
		 *   example.  Will be destroyed when this class is destroyed.
		 */
		Example( char *inStimulusURL, int inNumChoices,
				 char **inChoiceURLs, int inCorrectResponseIndex,
				 unsigned long inSerialNumber,
				 double inTargetPairWeight,
				 double *inChoiceWeights );
		

		
		~Example();


		
		/**
		 * Gets the URL for the stimulus face.
		 *
		 * @return the URL for the stimulus face.
		 *   Will be destroyed when this class is destroyed.
		 */
		char *getStimulusURL();



		/**
		 * Gets the number of response choices.
		 *
		 * @return the number of response choices.
		 */
		int getNumChoices();


		
		/**
		 * Gets the URLs for the response choice faces.
		 *
		 * @return an array of URLs for the response choice faces..
		 *   Will be destroyed when this class is destroyed.
		 */
		char **getChoiceURLs();



		/**
		 * Gets the index of the correct response choice.
		 *
		 * @return the index of the correct response choice..
		 */
		int getCorrectResponseIndex();



		/**
		 * Gets the unique serial number of this example.
		 *
		 * @return this example's serial number, which
		 *   is different from the serial numbers of all
		 *   other examples from this run of the system.
		 */
		unsigned long getSerialNumber();


		
		/**
		 * Gets a weight rating for the target pair in this example
		 *
		 * @return the weight of the target pair, in [0,1].
		 */
		double getTargetPairWeight();



		/**
		 * Gets a weight rating for a choice from this example.
		 *
		 * @param inChoiceIndex the index of the choice to check.
		 *
		 * @return the weight of the choice, in [0,1].
		 */
		double getChoiceWeight( int inChoiceIndex );

		

	protected:
		
		char *mStimulusURL;

		int mNumChoices;

		char **mChoiceURLs;

		int mCorrectResponseIndex;

		unsigned long mSerialNumber;

		double mTargetPairWeight;
		double *mChoiceWeights;
	};



inline Example::Example( char *inStimulusURL, int inNumChoices,
						 char **inChoiceURLs, int inCorrectResponseIndex,
						 unsigned long inSerialNumber,
						 double inTargetPairWeight,
						 double *inChoiceWeights )
	: mStimulusURL( inStimulusURL ), mNumChoices( inNumChoices ),
	  mChoiceURLs( inChoiceURLs ),
	  mCorrectResponseIndex( inCorrectResponseIndex ),
	  mSerialNumber( inSerialNumber ),
	  mTargetPairWeight( inTargetPairWeight ),
	  mChoiceWeights( inChoiceWeights ) {

	}



inline Example::~Example() {
	delete [] mStimulusURL;

	for( int i=0; i<mNumChoices; i++ ) {
		delete [] mChoiceURLs[i];
		}
	delete [] mChoiceURLs;

	delete [] mChoiceWeights;
	}



inline char *Example::getStimulusURL() {
	return mStimulusURL;
	}



inline int Example::getNumChoices() {
	return mNumChoices;
	}



inline char **Example::getChoiceURLs() {
	return mChoiceURLs;
	}



inline int Example::getCorrectResponseIndex() {
	return mCorrectResponseIndex;
	}



inline unsigned long Example::getSerialNumber() {
	return mSerialNumber;
	}



inline double Example::getTargetPairWeight() {
	return mTargetPairWeight;
	}



inline double Example::getChoiceWeight( int inChoiceIndex ) {
	return mChoiceWeights[ inChoiceIndex ];
	}



#endif
