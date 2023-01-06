/*
 * Modification History
 *
 * 2001-November-17   Jason Rohrer
 * Created.
 *
 * 2001-November-18   Jason Rohrer
 * Added support for saving the repository to file periodically.
 * Finished implementing the postResponse function.
 *
 * 2001-November-19   Jason Rohrer
 * Added support for statistics.
 *
 * 2001-November-20   Jason Rohrer
 * Added a more verbose error message.
 */



#ifndef DISTRIBUTION_EXAMPLE_REPOSITORY_INCLUDED
#define DISTRIBUTION_EXAMPLE_REPOSITORY_INCLUDED



#include "Example.h"
#include "ExampleRepository.h"

#include "minorGems/math/probability/ProbabilityMassFunction.h"

#include "minorGems/util/random/RandomSource.h"
#include "minorGems/util/SimpleVector.h"

#include "minorGems/system/MutexLock.h"

#include "minorGems/io/file/File.h"
#include "minorGems/io/file/FileInputStream.h"
#include "minorGems/io/file/FileOutputStream.h"


#include <time.h>
#include <math.h>


/**
 * Repository that uses an arbitrary distribution over the examples.
 *
 * @author Jason Rohrer
 */
class DistributionExampleRepository : public ExampleRepository {

	public:



		/**
		 * Constructs a repository.
		 *
		 * @param inRandSource the random source to use
		 *   when picking examples according to the distribution.
		 *   Must be destroyed by caller after this class is destroyed.
		 * @param inSecondsBetweenSaves the number of seconds
		 *   between disk saves of the repository.
		 * @param inRepositoryStoreFile the file to restore the
		 *   repository from and store the repository to periodically.
		 *   Will be destroyed when this class is destroyed.
		 * @param inExampleSeedFile the file of seed URL pairs to use,
		 *   or NULL if no seed file should be used.  Only used
		 *   if inExampleStoreFile does not exist.  All
		 *   examples from this file are given uniform probability masses.
		 *   Must be destroyed by caller if non-NULL.
		 *   Defaults to NULL.
		 */
		DistributionExampleRepository( RandomSource *inRandSource,
									   unsigned long inSecondsBetweenSaves,
									   File *inRepositoryStoreFile,
									   File *inExampleSeedFile = NULL );


		
		virtual ~DistributionExampleRepository();



		// implements the ExampleRepository interface

		virtual int getNumPairs();
		
		virtual void addExamplePair( char *inYoungFaceURL,
									 char *inOldFaceURL );

		virtual Example *getExample( int inNumDummyChoices );

		virtual void postResponse( Example *inExample,
								   int inOpponentChoiceIndex );
		


	protected:
		RandomSource *mRandSource;

		unsigned long mSecondsBetweenSaves;
		unsigned long mLastSaveTime;
		
		File *mRepositoryStoreFile;
		
		SimpleVector<char *> *mYoungFaceURLs;
		SimpleVector<char *> *mOldFaceURLs;

		
		ProbabilityMassFunction *mPairPMF;
		
		// for each pair, we have a distribution over
		// all other pairs as dummy choices
		SimpleVector< ProbabilityMassFunction* > *mChoiceDummyPMFs;

		
		MutexLock *mLock;

		unsigned long mNextAvailableExampleSerialNumber;



		/**
		 * Saves the repository to disk.
		 *
		 * Not thread safe, so should only be called
		 * by thread safe functions.
		 */
		virtual void saveRepository();



		/**
		 * Checks if enough time has passed to warrant a save,
		 * and saves the repository if necessary.
		 *
		 * Not thread safe, so should only be called
		 * by thread safe functions.
		 */
		virtual void checkAndSaveRepository();

		
		
	};



inline DistributionExampleRepository::DistributionExampleRepository(
	RandomSource *inRandSource,
	unsigned long inSecondsBetweenSaves,
	File *inRepositoryStoreFile, File *inSeedFile )
	: mRandSource( inRandSource ),
	  mSecondsBetweenSaves( inSecondsBetweenSaves ),
	  mRepositoryStoreFile( inRepositoryStoreFile ),
	  mLock( new MutexLock() ),
	  mNextAvailableExampleSerialNumber( 0 ) {

	mLastSaveTime = time( NULL );
	
	if( mRepositoryStoreFile->exists() ) {
		
		FileInputStream *inStream = new FileInputStream(
			mRepositoryStoreFile );
		
		long numPairs;
		inStream->readLong( &numPairs );

		mYoungFaceURLs = new SimpleVector<char*>( numPairs );
		mOldFaceURLs = new SimpleVector<char*>( numPairs );
		
		mChoiceDummyPMFs =
			new SimpleVector< ProbabilityMassFunction* >( numPairs );
		
		int i;
		// read all of the pair URLS
		for( i=0; i<numPairs; i++ ) {
			long youngURLLength;
			long oldURLLength;

			inStream->readLong( &youngURLLength );

			char *youngURL = new char[ youngURLLength + 1 ];

			inStream->read( (unsigned char*)youngURL, youngURLLength );
			youngURL[ youngURLLength ] = '\0';

			mYoungFaceURLs->push_back( youngURL );

			
			inStream->readLong( &oldURLLength );

			char *oldURL = new char[ oldURLLength + 1 ];

			inStream->read( (unsigned char*)oldURL, oldURLLength );
			oldURL[ oldURLLength ] = '\0';

			mOldFaceURLs->push_back( oldURL );			
			}

		// now deserialize the pair mass function
		mPairPMF = new ProbabilityMassFunction( mRandSource );
		mPairPMF->deserialize( inStream );

		
		// now deserialize each of the dummy choice mass functions
		for( i=0; i<numPairs; i++ ) {
			ProbabilityMassFunction *pmf =
				new ProbabilityMassFunction( mRandSource );

			pmf->deserialize( inStream );

			mChoiceDummyPMFs->push_back( pmf );
			}

		// last, deserialize the next available serial number
		inStream->readLong( (long *)( &mNextAvailableExampleSerialNumber ) );

		delete inStream;
		}	
	else if( inSeedFile != NULL ) {
		mYoungFaceURLs = new SimpleVector<char*>();
		mOldFaceURLs = new SimpleVector<char*>();

		mPairPMF = new ProbabilityMassFunction( mRandSource );
		mChoiceDummyPMFs =
			new SimpleVector< ProbabilityMassFunction* >();

		
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


		
inline DistributionExampleRepository::~DistributionExampleRepository() {
	int numPairs = mYoungFaceURLs->size();

	for( int i=0; i<numPairs; i++ ) {
		char *youngFaceURL = *( mYoungFaceURLs->getElement( i ) );
		delete [] youngFaceURL;
		char *oldFaceURL = *( mOldFaceURLs->getElement( i ) );
		delete [] oldFaceURL;

		ProbabilityMassFunction *pmf =
			*( mChoiceDummyPMFs->getElement( i ) );
		delete pmf;
		}

	delete mYoungFaceURLs;
	delete mOldFaceURLs;

	delete mPairPMF;
	delete mChoiceDummyPMFs;

	delete mRepositoryStoreFile;
	
	delete mLock;
	}



inline int DistributionExampleRepository::getNumPairs() {
	mLock->lock();

	int numPairs = mYoungFaceURLs->size();

	mLock->unlock();
	

	return numPairs;
	}



inline void DistributionExampleRepository::addExamplePair(
	char *inYoungFaceURL,
	char *inOldFaceURL ) {

	mLock->lock();

	long previousNumPairs = mYoungFaceURLs->size();
	
	mYoungFaceURLs->push_back( inYoungFaceURL );
	mOldFaceURLs->push_back( inOldFaceURL );

	// this new pair has "uniform" probability of being picked	
	double uniformVal;
	if( previousNumPairs != 0 ) {
		uniformVal = 1.0 / previousNumPairs;
		}
	else {
		uniformVal = 1;
		}
	mPairPMF->addElement( uniformVal );


	// now we need to construct a PMF for the dummy choices for this pair	
	long newNumPairs = previousNumPairs + 1;

	double *newDummyPMFVals = new double[ newNumPairs ];
	int i;
	double newUniformVal = 1.0 / newNumPairs;
	
	for( i=0; i<newNumPairs; i++ ) {
		newDummyPMFVals[i] = newUniformVal;
		}
	
	ProbabilityMassFunction *pmf =
		new ProbabilityMassFunction( mRandSource,
									 newNumPairs,
									 newDummyPMFVals );

	mChoiceDummyPMFs->push_back( pmf );

	
	// now we need to add this pair to the choice dummy PMFs of
	// the existing pairs
	for( i=0; i<previousNumPairs; i++ ) {
		pmf = *( mChoiceDummyPMFs->getElement( i ) );

		// it has "uniform" probability of being picked as a dummy
		// by each other pair
		pmf->addElement( uniformVal );
		}
	
	
	mLock->unlock();
	}



inline Example *DistributionExampleRepository::getExample(
	int inNumDummyChoices ) {

	mLock->lock();

	// first, pick the target pair
	unsigned long targetIndex = mPairPMF->sampleElement();
	
	char *stimulusURL = *( mYoungFaceURLs->getElement( targetIndex ) );
	
	// now pick dummy choices
	int *dummyIndices = new int[ inNumDummyChoices ];
	int dummyCount = 0;

	ProbabilityMassFunction *targetDummyPMF
		= *( mChoiceDummyPMFs->getElement( targetIndex ) );

	
	// fill in dummy indices with indices that differ from targetIndex
	while( dummyCount < inNumDummyChoices ) {
		int dummyIndex = 
			targetDummyPMF->sampleElement();

		char pickAgain = false;

		// make sure dummy is not the same as target
		if( dummyIndex == targetIndex ) {
			pickAgain = true;
			}
		// make sure dummy is not the same as any other dummy
		for( int i=0; i<dummyCount; i++ ) {
			if( dummyIndex == dummyIndices[ i ] ) {
				pickAgain = true;
				}
			}
		
		if( !pickAgain ) {
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


	// collect stats for this example
	double targetWeight = mPairPMF->getProbability( targetIndex );
	double *choiceWeights = new double[ inNumDummyChoices + 1 ];
	dummyIndex = 0;
	for( i=0; i<inNumDummyChoices + 1; i++ ) {
		if( i!=targetChoiceSpot ) {
			choiceWeights[i] =
				targetDummyPMF->getProbability( dummyIndices[dummyIndex] );
			dummyIndex++;
			}
		else {
			choiceWeights[i] = 0;
			}
		}

	
	Example *returnExample =
		new Example( stimulusURLCopy, inNumDummyChoices + 1,
					 choiceURLsCopy, targetChoiceSpot,
					 mNextAvailableExampleSerialNumber,
					 targetWeight, choiceWeights );
	
	delete [] dummyIndices;
	delete [] choiceURLs;

	mNextAvailableExampleSerialNumber++;
	
	mLock->unlock();

	return returnExample;
	}



inline void DistributionExampleRepository::postResponse(
	Example *inExample, int inOpponentChoiceIndex ) {

	mLock->lock();

	// first, we need to find the index of URLs in the example
	char *stimulusURL = inExample->getStimulusURL();
	char **choiceURLs = inExample->getChoiceURLs();
	int numChoices = inExample->getNumChoices(); 
	
	int stimulusIndex = -1;
	int *choiceIndices = new int[ numChoices ];

	int i, j;
	int numPairs = mYoungFaceURLs->size();

	for( j=0; j<numChoices; j++ ) {
		choiceIndices[j] = -1;
		}

	for( i=0; i<numPairs; i++ ) {
		char *youngURL = *( mYoungFaceURLs->getElement( i ) );
		char *oldURL = *( mOldFaceURLs->getElement( i ) );

		if( strcmp( youngURL, stimulusURL ) == 0 ) {
			stimulusIndex = i;
			}

		for( j=0; j<numChoices; j++ ) {
			if( strcmp( oldURL, choiceURLs[j] ) == 0 ) {
				choiceIndices[j] = i;
				}
			}		
		}

	// make sure all URLs found in repository before proceeding
	char allURLsFound = true;

	if( stimulusIndex == -1 ) {
		allURLsFound = false;
		}
	for( j=0; j<numChoices; j++ ) {
		if( choiceIndices[j] == -1 ) {
			allURLsFound = false;
			}
		}

	if( allURLsFound ) {
		int correctResponseIndex = inExample->getCorrectResponseIndex();

		int responseFactor = 1;

		if( correctResponseIndex != inOpponentChoiceIndex ) {
			responseFactor = -1;
			}

		// first, update the PMF over all pairs

		// from the LazyBoosting paper

		double oldD = mPairPMF->getProbability( stimulusIndex );
		
		double rHat = oldD * responseFactor;
		double minProb = mPairPMF->getMinimumProbability();
		if( rHat < minProb ) {
			rHat = minProb;
			}

		// log = ln
		double alphaHat = 0.5 * log( ( 1 + rHat ) / ( 1 - rHat ) );

		double newD = oldD * exp( ( -alphaHat * responseFactor ) );

		mPairPMF->setProbability( stimulusIndex, newD );

		// now, update the PMF for the dummy choices given this pair
		ProbabilityMassFunction *choiceDummyPMF =
			*( mChoiceDummyPMFs->getElement( stimulusIndex ) );

		for( j=0; j<numChoices; j++ ) {
			// skip the mass of this pair with itself
			if( j != stimulusIndex ) {
				// use the same update for the choices
				// (we increase the weight of a choice
				// if it was part of an example set that stumped
				// the opponent)
				// NOTE:
				// Maybe we should only increase the weight
				// on the choice that lured the opponent
				// away from the correct choice, in otherwords,
				// the wrong choice clicked by the user.
				// The other choices may have nothing
				// to do with the luring choice.
				int choiceIndex = choiceIndices[j];

				oldD = choiceDummyPMF->getProbability( choiceIndex );
				
				rHat = oldD * responseFactor;
				minProb = choiceDummyPMF->getMinimumProbability();
				if( rHat < minProb ) {
					rHat = minProb;
					}
				
				// log = ln
				alphaHat = 0.5 * log( ( 1 + rHat ) / ( 1 - rHat ) );
				
				newD = oldD * exp( ( -alphaHat * responseFactor ) );
				
				choiceDummyPMF->setProbability( choiceIndex, newD );
				}
			}
		
		}
	else {
		printf( "All URLs not found while posting response to repository\n" ); 
		printf( "Printing example:\n" );

		int i;
		for( i=0; i<numChoices; i++ ) {
			printf( "choice %d: %s\n", i, choiceURLs[i] );
			}
		printf( "stimulus: %s\n", stimulusURL );

		printf( "Printing repository:\n" );

		for( i=0; i<numPairs; i++ ) {
			char *youngURL = *( mYoungFaceURLs->getElement( i ) );
			char *oldURL = *( mOldFaceURLs->getElement( i ) );

			printf( "young %d: %s\n", youngURL );
			printf( "old %d: %s\n", oldURL );
			}
		}

	
	delete [] choiceIndices;
	
	
	checkAndSaveRepository();
	

	mLock->unlock();
	// do nothing
	}



inline void DistributionExampleRepository::checkAndSaveRepository() {

	// check if the repository needs to be saved.
	unsigned long currentTime = time( NULL );
	
	if( currentTime - mLastSaveTime > mSecondsBetweenSaves ) {
		saveRepository();

		mLastSaveTime = currentTime;
		}

	}



inline void DistributionExampleRepository::saveRepository() {
	printf( "Saving repository...\n" );
	
	File *backupFile = NULL;
	
	if( mRepositoryStoreFile->exists() ) {
		// first, make a backup of our current store file
		
		backupFile = new File( NULL, "repositoryBackup.nul" );

		mRepositoryStoreFile->copy( backupFile );
		}

	
	// now we save the current repository state into the store file
			
	FileOutputStream *outStream = new FileOutputStream(
		mRepositoryStoreFile );
		
	long numPairs = mYoungFaceURLs->size();
	outStream->writeLong( numPairs );
		
	int i;
	// write all of the pair URLS
	for( i=0; i<numPairs; i++ ) {
		char *youngURL = *( mYoungFaceURLs->getElement( i ) );
		char *oldURL = *( mOldFaceURLs->getElement( i ) );
		
		long youngURLLength = strlen( youngURL );
		long oldURLLength = strlen( oldURL );
		
		outStream->writeLong( youngURLLength );
		outStream->write( (unsigned char*)youngURL, youngURLLength );
			
		outStream->writeLong( oldURLLength );
		outStream->write( (unsigned char*)oldURL, oldURLLength );
		}

	// now serialize the pair mass function
	mPairPMF->serialize( outStream );
	
	// now serialize each of the dummy choice mass functions
	for( i=0; i<numPairs; i++ ) {
		ProbabilityMassFunction *pmf =
			*( mChoiceDummyPMFs->getElement( i ) );

		pmf->serialize( outStream );
		}

	// last, seserialize the next available serial number
	outStream->writeLong( (long)mNextAvailableExampleSerialNumber );

	delete outStream;

	
	// we have successfully written a new file,
	// so delete our backup

	if( backupFile != NULL ) {
		backupFile->remove();
		delete backupFile;
		}

	printf( "...done\n" );
	}



#endif






