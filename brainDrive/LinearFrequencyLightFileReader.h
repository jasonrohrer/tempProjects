/*
 * Modification History
 *
 * 2001-November-8   Jason Rohrer
 * Created.
 */


#ifndef LINEAR_FREQUENCY_LIGHT_FILE_READER_INCLUDED
#define LINEAR_FREQUENCY_LIGHT_FILE_READER_INCLUDED


#include "LinearFrequencyLight.h"

#include <stdio.h>


/**
 * Class that can read a set of linear frequency light specs from
 * a .lit file.
 *
 * @author Jason Rohrer
 */
class LinearFrequencyLightFileReader {

	public:

		/**
		 * Reads a set of linear frequency light specs from file.
		 *
		 * @param inFileName the name of the file to read from.
		 *   Must be destroyed by caller if non-const.
		 * @param outNumLightsRead pointer to where the number
		 *   of read lights will be returned.
		 *
		 * @return an array of lights read, or NULL
		 *   in the case of an error.
		 */
		static LinearFrequencyLight **readFile( char *inFileName,
												int *outNumLightsRead );

	protected:



		/**
		 * Prints a general error message about bad file format.
		 *
		 * @param inFileName the name of the file with the bad format.
		 */
		static void printBadFormatError( char *inFileName );



		/**
		 * Destroys all non-null elements in an array of lights,
		 * and then destroys the array.
		 *
		 * @param inArray the array to destroy.
		 * @param inNumLights the size of the array.
		 */
		static void cleanUpLightArray(
			LinearFrequencyLight **inArray, int inNumLights );

			

	};



inline LinearFrequencyLight **LinearFrequencyLightFileReader::readFile(
	char *inFileName,
	int *outNumLightsRead ) {

	// open the file as a text file
	FILE *file = fopen( inFileName, "r" );

	if( file == NULL ) {
		printf( "File open failed: %s\n", inFileName );
		return NULL;
		}

	int numLights;
	
	int numRead = fscanf( file, "%d", &numLights );

	if( numRead == 0 ) {
		printBadFormatError( inFileName );
		return NULL;
		}

	LinearFrequencyLight **lights = new LinearFrequencyLight*[ numLights ];

	// first, set them all to NULL so we can delete properly
	// in the case of an error
	int i;
	for( i=0; i<numLights; i++ ) {
		lights[i] = NULL;
		}
	   
	
	for( i=0; i<numLights; i++ ) {
		int inverted;
		numRead = fscanf( file, "%d", &inverted );

		if( numRead == 0 ) {
			printBadFormatError( inFileName );
			return NULL;
			}

		int numPoints;
		numRead = fscanf( file, "%d", &numPoints );

		unsigned long *seconds = new unsigned long[ numPoints ];
		unsigned long *milliseconds = new unsigned long[ numPoints ];
		double *freq = new double[ numPoints ];

		
		for( int j=0; j<numPoints; j++ ) {
			
			numRead = fscanf( file, "%d", &( seconds[j] ) );

			if( numRead == 0 ) {
				printBadFormatError( inFileName );

				delete [] seconds;
				delete [] milliseconds;
				delete [] freq;

				cleanUpLightArray( lights, numLights );
				return NULL;
				}

			numRead = fscanf( file, "%d", &( milliseconds[j] ) );
			if( numRead == 0 ) {
				printBadFormatError( inFileName );
				
				delete [] seconds;
				delete [] milliseconds;
				delete [] freq;

				cleanUpLightArray( lights, numLights );
				return NULL;
				}
			
			numRead = fscanf( file, "%lf", &( freq[j] ) );
			if( numRead == 0 ) {
				printBadFormatError( inFileName );

				
				delete [] seconds;
				delete [] milliseconds;
				delete [] freq;

				cleanUpLightArray( lights, numLights );
				return NULL;
				}
			
			}

		lights[i] = new LinearFrequencyLight( numPoints, seconds,
											  milliseconds, freq,
											  (char)inverted );
		}

	
	*outNumLightsRead = numLights;
	
	return lights;
	}



inline void LinearFrequencyLightFileReader::printBadFormatError(
	char *inFileName ) {
	
	printf( ".lit file not in proper format: %s\n", inFileName );
	}



inline void LinearFrequencyLightFileReader::cleanUpLightArray(
	LinearFrequencyLight **inArray, int inNumLights ) {

	for( int i=0; i<inNumLights; i++ ) {
		if( inArray[i] != NULL ) {
			delete inArray[i];
			}
		}

	delete [] inArray;
	}



#endif



