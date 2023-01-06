/*
 * Modification History
 *
 * 2002-January-11   Jason Rohrer
 * Created.
 */


#include <stdio.h>


int getNumBitsOn( unsigned char inChar );


int main() {

	int numChords = 256;

	unsigned char *chords = new unsigned char[ numChords ];

	
	int i;
	for( i=0; i<numChords; i++ ) {
		chords[i] = (unsigned char)i;
		}

	// bubble sort
	for( int a=0; a<numChords; a++ ) {
		for( int b=0; b<numChords-1; b++ ) {

			int numThis = getNumBitsOn( chords[ b ] );
			int numNext = getNumBitsOn( chords[ b+1 ] );

			if( numThis < numNext ) {
				unsigned char temp = chords[ b ];
				chords[ b ] = chords[ b+1 ];
				chords[ b+1 ] = temp;
				}
			}
		}

	// now print chart
	for( i=0; i<256; i++ ) {
		int j;
		for( j=0; j<4; j++ ) {
			printf( "%d ", ( chords[i] >> j ) & 1 );
			}
		printf( " " );
		for( j=4; j<8; j++ ) {
			printf( "%d ", ( chords[i] >> j ) & 1 );
			}
		printf( "\n" );
		}

	
	delete [] chords;
	
	return 0;
	}



int getNumBitsOn( unsigned char inChar ) {
	int numBits = 0;
	for( int i=0; i<8; i++ ) {
		if( 1 == ( ( inChar >> i ) & 1 ) ) {
			numBits++;
			}
		}

	return numBits;
	}

