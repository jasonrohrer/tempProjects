/*
 * Modification History
 *
 * 2001-November-6   Jason Rohrer
 * Created.
 *
 * 2001-November-7   Jason Rohrer
 * Added support for testing the Light interface and
 * the LinearFrequencyLight implementation.
 * Made test vectors more interesting.
 *
 * 2001-November-8   Jason Rohrer
 * Added support for reading in linear frequency light
 * specs from file.  Not yet complete.
 * Finished support for reading light specs from file.
 */


#include "ParallelPort.h"
#include "Light.h"
#include "LinearFrequencyLight.h"
#include "LinearFrequencyLightFileReader.h"

#include "minorGems/system/Time.h"

#include <stdio.h>


/**
 * Test programthat flashes all eight output pins once
 * every second.
 *
 * @author Jason Rohrer
 */
int main( char inNumArgs, char **inArgs ) {

	if( inNumArgs != 2 ) {
		printf( "Must input a .lit file name\n" );
		return -1;
		}
	
	int numLights;

	LinearFrequencyLight **lights =
		LinearFrequencyLightFileReader::readFile( inArgs[1], &numLights );
	
	if( lights == NULL ) {
		return -1;
		}

	printf( "Read %d lights from file %s\n", numLights, inArgs[1] );

	
	unsigned long currentSec;
	unsigned long currentMSec;

	
	Time::getCurrentTime( &currentSec, &currentMSec );

	int i;

	for( i=0; i<numLights; i++ ) {
		lights[i]->setStartOffset( currentSec, currentMSec );
		}

	int numUsedLights = numLights;

	// we only have 8 output pins
	if( numUsedLights > 8 ) {
		numUsedLights = 8;
		}

	
	while( true ) {
		Time::getCurrentTime( &currentSec, &currentMSec );

		char byte = 0x00;
		
		
		for( i=0; i<numUsedLights; i++ ) {
			if( lights[i]->isLightOn( currentSec, currentMSec ) ) {
				// turn bit for light on
				byte = byte | ( 1 << i );
				}
			// else leave the bit off
			}

		ParallelPort::setByte( byte );
		}

	return 1;
	}

	
	
