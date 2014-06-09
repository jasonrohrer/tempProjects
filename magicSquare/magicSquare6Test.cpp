

#include "magicSquare6.h"

#include <stdio.h>


char printSquare( int *inArray, int inD ) {
    for( int y=0; y<inD; y++ ) {
        for( int x=0; x<inD; x++ ) {
            printf( "%3d ", inArray[y * inD + x] );
            }
        printf("\n" );
        }
    }


int main() {
    
    int *squareA = generateMagicSquare6( 10 );
    
    printf( "SquareA:\n" );
    printSquare( squareA, 6 );
    delete [] squareA;
    
    int setSize = 10;
    int **squareSet = generateMagicSquare6( 10, setSize );
    
    for( int i=0; i<setSize; i++ ) {
        printf( "Square %d:\n", i );
        
        printSquare( squareSet[i], 6 );
        
        delete [] squareSet[i];
        }
    delete [] squareSet;
    
    return 1;
    }
