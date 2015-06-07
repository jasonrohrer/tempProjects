#include "magicSquare6.h"

#include <stdio.h>

#include "minorGems/util/random/JenkinsRandomSource.h"



void printSquare( int *inArray, int inD ) {
    
    printf( "    " );

    for( int x=0; x<inD; x++ ) {
        printf( "%3d ", x );
        }
    printf( "\n" );
    
    for( int x=0; x<inD * 5; x++ ) {
        printf( "%c", '-' );
        }
    printf( "\n" );

    
    for( int y=0; y<inD; y++ ) {
        printf( "%d | ", y );
        
        for( int x=0; x<inD; x++ ) {
            printf( "%3d ", inArray[y * inD + x] );
            }

        printf( "\n" );
        for( int x=0; x<inD * 5; x++ ) {
            printf( "%c", '-' );
            }
        printf( "\n" );
        //printf("\n" );
        }
    }



int main() {

    for( int r=0; r<1; r++ ) {
        printf( "r=%d\n", r );
        
        JenkinsRandomSource randSource( r );

        int *squareA = generateMagicSquare6( 10 + r );
        
        printSquare( squareA, 6 );


        // our summed score over all possible opponent picks if
        // we pick [i][j]
        int ourScore[6][6];
        
        for( int i=0; i<6; i++ ) {
            ourScore[i][i] = -99999;
            }
        
        // i is what we pick for us
        // j is what we pick for them
        for( int ui = 0; ui<6; ui++ ) {
            for( int uj = 0; uj<6; uj++ ) {
                
                if( ui != uj ) {
                    
                    ourScore[ui][uj] = 0;
                    
                    // their picks
                    // i is what they pick for them
                    // j is what they pick for us
                    for( int ti = 0; ti<6; ti++ ) {
                        for( int tj = 0; tj<6; tj++ ) {
                            
                            if( ti == tj ) {
                                continue;
                                }
                            
                            //ourScore[ui][uj]
                            int scoreDiff =
                                // plus our score
                                squareA[ tj * 6 + ui ]
                                // minus their score
                                - squareA[ ti * 6 + uj ];

                            ourScore[ui][uj] += scoreDiff;
                            if( false && ui == 4 && uj==5 ) {
                                printf( 
                                    "We get %d, they get %d, "
                                    "score diff = %d, running total = %d\n", 
                                    squareA[ tj * 6 + ui ],
                                    squareA[ ti * 6 + uj ],
                                    scoreDiff,
                                    ourScore[ui][uj] );
                                }
                            
                            }
                        }

                    printf( "Net score for colUs=%d, colThem=%d = %d\n",
                            ui, uj, ourScore[ui][uj] );
                    }
                }
            }
        
        delete [] squareA;
        }
    

    return 0;
    }
