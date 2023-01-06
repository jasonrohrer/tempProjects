/*
 * Modification History
 *
 * 2001-November-14   Jason Rohrer
 * Created.
 * Added a function for getting the number of pairs in the repository.
 *
 * 2001-November-15   Jason Rohrer
 * Added support for unique example serial numbers.
 * Made thread-safe.
 * Added support for a seed file.
 * Fixed a bug in target response url selection.
 *
 * 2001-November-19   Jason Rohrer
 * Added support for statistics.
 */



#ifndef UNIFORM_EXAMPLE_REPOSITORY_INCLUDED
#define UNIFORM_EXAMPLE_REPOSITORY_INCLUDED



#include "Example.h"
#include "ExampleRepository.h"

#include "minorGems/util/random/RandomSource.h"
#include "minorGems/util/SimpleVector.h"

#include "minorGems/system/MutexLock.h"

#include "minorGems/io/file/File.h"
#include "minorGems/io/file/FileInputStream.h"



/**
 * Repository that uses a uniform distribution over all examples.
 *
 * @author Jason Rohrer
 */
class UniformExampleRepository : public ExampleRepository {

	public:



		/**
		 * Constructs a repository.
		 *
		 * @param inRandSource the random source to use
		 *   when picking examples according to the distribution.
		 *   Must be destroyed by caller after this class is destroyed.
		 * @param inSeedFile the file of seed URL pairs to use,
		 *   or NULL if no seed file should be used.
		 *   Must be destroyed by caller if non-NULL.
		 *   Defaults to NULL.
		 */
		UniformExampleRepository( RandomSource *inRandSource,
								  File *inSeedFile = NULL );


		
		virtual ~UniformExampleRepository();



		// implements the ExampleRepository interface

		virtual int getNumPairs();
		
		virtual void addExamplePair( char *inYoungFaceURL,
									 char *inOldFaceURL );

		virtual Example *getExample( int inNumDummyChoices );

		virtual void postResponse( Example *inExample,
								   int inOpponentChoiceIndex );
		


	protected:
		RandomSource *mRandSource;

		SimpleVector<char *> *mYoungFaceURLs;
		SimpleVector<char *> *mOldFaceURLs;

		MutexLock *mLock;

		unsigned long mNextAvailableExampleSerialNumber;
	};



inline UniformExampleRepository::UniformExampleRepository(
	RandomSource *inRandSource, File *inSeedFile )
	: mRandSource( inRandSource ),
	  mYoungFaceURLs( new SimpleVector<char*>() ),
	  mOldFaceURLs( new SimpleVector<char*>() ),
	  mLock( new MutexLock() ),
	  mNextAvailableExampleSerialNumber( 0 ) {

	if( inSeedFile != NULL ) {

		if( inSeedFile->exists() ) {

			int fileSize = inSeedFile->getLength();

			FILE *file = fopen( inSeedFile->getFullFileName(), "r" );
			
			int numRead = 1;
			while( numRead == 1 ) {
				char *firstURLBuffer = new char[ fileSize + 1];
				numRead = fscanf( file, "%s", firstURLBuffer );

				if( numRead == 1 ) {
					char *secondURLBuffer = new char[ fileSize + 1];
					numRead = fscanf( file, "%s", secondURLBuffer );

					if( numRead == 1 ) {
						// shrink the buffers to save memory
						char *firstURL =
							new char[ strlen( firstURLBuffer ) + 1 ];
						strcpy( firstURL, firstURLBuffer );
						char *secondURL =
							new char[ strlen( secondURLBuffer ) + 1 ];
						strcpy( secondURL, secondURLBuffer );

						addExamplePair( firstURL, secondURL );
						}
					
					delete [] secondURLBuffer;
					}
				
				delete [] firstURLBuffer;
				}
			
			fclose( file );
			
			}
		else {
			printf( "Seed file does not exist:  %s\n",
					inSeedFile->getFullFileName() );
			}
		
		}
	
	}


		
inline UniformExampleRepository::~UniformExampleRepository() {
	int numPairs = mYoungFaceURLs->size();

	for( int i=0; i<numPairs; i++ ) {
		char *youngFaceURL = *( mYoungFaceURLs->getElement( i ) );
		delete [] youngFaceURL;
		char *oldFaceURL = *( mOldFaceURLs->getElement( i ) );
		delete [] oldFaceURL;
		}

	delete mYoungFaceURLs;
	delete mOldFaceURLs;

	delete mLock;
	}



inline int UniformExampleRepository::getNumPairs() {
	mLock->lock();

	int numPairs = mYoungFaceURLs->size();

	mLock->unlock();
	

	return numPairs;
	}



inline void UniformExampleRepository::addExamplePair(
	char *inYoungFaceURL,
	char *inOldFaceURL ) {

	mLock->lock();
	
	mYoungFaceURLs->push_back( inYoungFaceURL );
	mOldFaceURLs->push_back( inOldFaceURL );	

	mLock->unlock();
	}



inline Example *UniformExampleRepository::getExample(
	int inNumDummyChoices ) {

	mLock->lock();
	
	int numPairs = mYoungFaceURLs->size();

	// first, pick the stimulus
	int targetIndex = mRandSource->getRandomBoundedInt( 0, numPairs - 1 );
	
	char *stimulusURL = *( mYoungFaceURLs->getElement( targetIndex ) );
	
	// now pick dummy choices
	int *dummyIndices = new int[ inNumDummyChoices ];
	int dummyCount = 0;

	double weight = 1.0 / numPairs;
	
	// fill in dummy indices with indices that differ from targetIndex
	while( dummyCount < inNumDummyChoices ) {
		int dummyIndex =
			mRandSource->getRandomBoundedInt( 0, numPairs - 1 );

		if( dummyIndex != targetIndex ) {
			dummyIndices[ dummyCount ] = dummyIndex;
			dummyCount++;
			}
		}

	char **choiceURLs = new char*[ inNumDummyChoices + 1 ];

	// pick a spot for the target choice
	int targetChoiceSpot = 
		mRandSource->getRandomBoundedInt( 0, inNumDummyChoices );

	choiceURLs[ targetChoiceSpot ] =
		*( mOldFaceURLs->getElement( targetIndex ) );

	
	// now fill in the dummies
	int dummyIndex = 0;
	int i;
	for( i=0; i<inNumDummyChoices + 1; i++ ) {
		if( i!= targetChoiceSpot ) {
			choiceURLs[ i ] =
				*( mOldFaceURLs->getElement( dummyIndices[ dummyIndex ] ) );
			dummyIndex++;
			}
		}

	// now we have the stimulus, the choices, and the target index
	// we need to copy them to make our example

	char *stimulusURLCopy = new char[ strlen( stimulusURL ) + 1 ];
	stimulusURLCopy = strcpy( stimulusURLCopy, stimulusURL );

	char **choiceURLsCopy = new char*[ inNumDummyChoices + 1 ];
	for( i=0; i<inNumDummyChoices + 1; i++ ) {
		choiceURLsCopy[i] = new char[ strlen( choiceURLs[i] ) + 1 ];
		choiceURLsCopy[i] = strcpy( choiceURLsCopy[i], choiceURLs[i] );
		}
	
	delete [] dummyIndices;
	delete [] choiceURLs;


	double *choiceWeights = new double[ inNumDummyChoices + 1 ];

	for( i=0; i<inNumDummyChoices + 1; i++ ) {
		if( i!=targetChoiceSpot ) {
			choiceWeights[i] = weight;
			}
		else {
			choiceWeights[i] = 0;
			}
		}
	
	Example *returnExample =
		new Example( stimulusURLCopy, inNumDummyChoices + 1,
					 choiceURLsCopy, targetChoiceSpot,
					 mNextAvailableExampleSerialNumber,
					 weight, choiceWeights );

	mNextAvailableExampleSerialNumber++;
	
	mLock->unlock();

	return returnExample;
	}



inline void UniformExampleRepository::postResponse(
	Example *inExample, int inOpponentChoiceIndex ) {

	// do nothing
	}



#endif




