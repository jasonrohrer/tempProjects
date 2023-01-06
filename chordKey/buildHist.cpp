/*
 * Modification History
 *
 * 2002-January-9   Jason Rohrer
 * Created.
 *
 * 2002-January-11   Jason Rohrer
 * Fixed a bug in the number of characters.
 * Added sorting.
 */

#include <stdio.h>


int main() {

	int charGot = getchar();

	int numChars = 256;
	
	int *histogram = new int[ numChars ];
	char *characters = new char[ numChars ];
	
	int i;
	for( i=0; i<numChars; i++ ) {
		histogram[i] = 0;
		characters[i] = i;
		}
	
	while( charGot != EOF ) {
		histogram[charGot] ++;
		charGot = getchar();
		}

	// bubble sort
	for( int a=0; a<numChars; a++ ) {
		for( int b=0; b<numChars-1; b++ ) {

			if( histogram[ b ] > histogram[ b+1 ] ) {
				int temp = histogram[ b ];
				histogram[ b ] = histogram[ b+1 ];
				histogram[ b+1 ] = temp;

				temp = characters[ b ];
				characters[ b ] = characters[ b+1 ];
				characters[ b+1 ] = temp;
				}
			}
		}
	
	for( i=0; i<numChars; i++ ) {
		printf( "%d\t\t\"%c\"\t\t%d\n", histogram[i], characters[i],
				(int)( characters[i] ) ); 
		}

	delete [] histogram;
	
	return 0;
	}

