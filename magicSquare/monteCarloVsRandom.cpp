#include "magicSquare6.h"
#include "MagicSquareGameState.h"


#include "minorGems/ai/gameTree/minMax.h"


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



typedef struct PickOrder {
        int picks[6];
    } PickOrder;


PickOrder allPickOrders[720];

static void fillAllPickOrders() {
    
    int o = 0;
    for( int i=0; i<6; i++ ) {
        for( int j=0; j<6; j++ ) {
            for( int k=0; k<6; k++ ) {
                for( int l=0; l<6; l++ ) {
                    for( int m=0; m<6; m++ ) {
                        for( int n=0; n<6; n++ ) {
                            if( i != j
                                &&
                                i != k
                                &&
                                i != l
                                &&
                                i != m
                                &&
                                i != n
                                &&
                                j != k
                                &&
                                j != l
                                &&
                                j != m
                                &&
                                j != n
                                &&
                                k != l
                                &&
                                k != m
                                &&
                                k != n
                                &&
                                l != m
                                &&
                                l != n
                                &&
                                m != n ) {
                                
                                allPickOrders[o].picks[0] = i;
                                allPickOrders[o].picks[1] = j;
                                allPickOrders[o].picks[2] = k;
                                allPickOrders[o].picks[3] = l;
                                allPickOrders[o].picks[4] = m;
                                allPickOrders[o].picks[5] = n;
                                o++;
                                }
                            }
                        }
                    }
                }
            }
        }
    /*
    printf( "After setting, o = %d\n", o );

    for( int i=0; i<o; i++ ) {
        printf( "Order %d = ", i );
        for( int p=0; p<6; p++ ) {
            printf( "%d ", allPickOrders[i].picks[p] );
            }
        printf( "\n" );
        }
    */
    }


#include "minorGems/util/random/JenkinsRandomSource.h"


static int runTest( int inSquareSeed ) {
    
    JenkinsRandomSource randSource( inSquareSeed );

    int *squareA = generateMagicSquare6( 10 + inSquareSeed );

    
    int winCount[720];

    for( int p=0; p<720; p++ ) {
        winCount[p] = 0;

        PickOrder ourPickOrder = allPickOrders[p];
        
        for( int r=0; r<720; r++ ) {
            MagicSquareGameState state( squareA );

            PickOrder theirPickOrder = 
                allPickOrders[ r ];
            

            for( int m=0; m<6; m++ ) {
                state.makeMoveInternal( 0, ourPickOrder.picks[m] );
                state.makeMoveInternal( 1, theirPickOrder.picks[m] );
                }
            if( state.getScore( 0 ) > state.getScore( 1 ) ) {
                winCount[p]++;
                }
                
            }
        
        //printf( "Win count for %d = %d\n", p, winCount[p] );
        }
    
    int bestP;
    int bestWinCount = 0;
    
    for( int p=0; p<720; p++ ) {
        if( winCount[p] > bestWinCount ) {
            bestWinCount = winCount[p];
            bestP = p;
            }
        }
    /*
    printf( "r = %d, Best p = %d, win count = %d (%f)\n", 
            inSquareSeed, bestP, bestWinCount, (float)bestWinCount / 720.0 );
    */
            /*
    PickOrder ourPickOrder = allPickOrders[bestP];
    
    int testWinCount = 0;
    for( int r=0; r<720; r++ ) {
         MagicSquareGameState state( squareA );
         
         PickOrder theirPickOrder = 
             allPickOrders[ r ];
         
         
         for( int m=0; m<6; m++ ) {
             state.makeMoveInternal( 0, ourPickOrder.picks[m] );
             state.makeMoveInternal( 1, theirPickOrder.picks[m] );
             }
         if( state.getScore( 0 ) > state.getScore( 1 ) ) {
             testWinCount++;
             }       
        }

    printf( "Subsequent test win count = %d\n", testWinCount );
    */

    delete [] squareA;
    return bestWinCount;
    }


int main() {
    
    fillAllPickOrders();
    
    
    int bestWinCount = 0;
    
    for( int r=30; r<600; r++ ) {

        int winCount = runTest( r );
        printf( "r=%d\n", r );
        
        if( winCount <= 360 ) {
            printf( "Win count = %d for seed = ", winCount, r );
            }
        }
    }
