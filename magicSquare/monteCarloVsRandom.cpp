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


PickOrder getEmptyPickOrder() {
    PickOrder p;
    for( int m=0; m<6; m++ ) {
        p.picks[m] = -1;
        }
    return p;
    }


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


static void printPickOrder( PickOrder inPickOrder ) {
    printf( "[ " );
    for( int p=0; p<6; p++ ) {
        printf( "%d ", inPickOrder.picks[p] );
        }
    printf( "]\n" );
    }


#include "minorGems/util/random/JenkinsRandomSource.h"

static char arePicksCompatible( PickOrder inPartialPick, 
                                PickOrder inFullPick ) {
    for( int m=0; m<6; m++ ) {
        if( inFullPick.picks[m] != inPartialPick.picks[m]
            &&
            inPartialPick.picks[m] != -1 ) {
            return false;
            }
        }
    return true;
    }




// make two picks
// the so far picks and the return picks contain -1 to mark unknown (or as of
//  yet unset) picks
static PickOrder findBestMove( int *inSquare,
                               PickOrder inOurPicksSoFar,
                               PickOrder inTheirPicksSoFar ) {

    MagicSquareGameState state( inSquare );

    int winCount[720];

    int bestP;
    int bestWinCount = 0;

    char theirPicksCompatible[720];
    
    
    for( int r=0; r<720; r++ ) {
        theirPicksCompatible[r] = false;
        
        PickOrder theirPickOrder = allPickOrders[ r ];
        
        if( arePicksCompatible( inTheirPicksSoFar, theirPickOrder ) ) {
            theirPicksCompatible[r] = true;
            }
        }
    

    for( int p=0; p<720; p++ ) {
        winCount[p] = 0;
        
        PickOrder ourPickOrder = allPickOrders[p];
        
        if( arePicksCompatible( inOurPicksSoFar, ourPickOrder ) ) {
            // pick p is a way to fill in and continue ourPicksSoFar

            for( int r=0; r<720; r++ ) {
                
                
                if( theirPicksCompatible[r] ) {
                    PickOrder theirPickOrder = allPickOrders[ r ];
                    
                    state.setMoves( 0, ourPickOrder.picks );
                    state.setMoves( 1, theirPickOrder.picks );
                    
                    if( state.getScore( 0 ) > state.getScore( 1 ) ) {
                        winCount[p]++;
                        }
                    }
                }
            
            }
        
        if( winCount[p] > bestWinCount ) {
            bestWinCount = winCount[p];
            bestP = p;
            }
        }

    // what moves in our move list are waiting to be filled?
    int nextMovePlaces[2];
    int setCount = 0;

    for( int m=0; m<6 && setCount < 2; m++ ) {
        if( inOurPicksSoFar.picks[m] == -1 ) {
            nextMovePlaces[setCount] = m;
            setCount++;
            }
        }

    // win counts for each possible pair of moves that can fill the next
    // two places
    int nextMovePairWinCounts[6][6];
    for( int m=0; m<6; m++ ) {
        for( int n=0; n<6; n++ ) {
            nextMovePairWinCounts[m][n] = 0;
            }
        }
    
    for( int p=0; p<720; p++ ) {
        
        PickOrder ourPickOrder = allPickOrders[p];
        
        if( arePicksCompatible( inOurPicksSoFar, ourPickOrder ) ) {

            nextMovePairWinCounts
                [ ourPickOrder.picks[ nextMovePlaces[0] ] ]
                [ ourPickOrder.picks[ nextMovePlaces[1] ] ]
                += winCount[p];  
            }
        }
             
    int bestM, bestN;
    bestWinCount = 0;
    
    for( int m=0; m<6; m++ ) {
        for( int n=0; n<6; n++ ) {
            if( nextMovePairWinCounts[m][n] > bestWinCount ) {
                bestWinCount = nextMovePairWinCounts[m][n];
                bestM = m;
                bestN = n;
                }
            }
        }
    
    /*     
    PickOrder bestPickOrder = allPickOrders[ bestP ];

    // augment ourPicksSoFar until we've set next two moves
    setCount = 0;
    for( int m=0; m<6 && setCount < 2; m++ ) {
        
        if( inOurPicksSoFar.picks[m] == -1 ) {
            inOurPicksSoFar.picks[m] = bestPickOrder.picks[m];
            setCount++;
            }
        }
    */
    inOurPicksSoFar.picks[ nextMovePlaces[0] ] = bestM;
    inOurPicksSoFar.picks[ nextMovePlaces[1] ] = bestN;
    

    return inOurPicksSoFar;
    }


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




// returns 1 if we won
static int playGameVsRandom( JenkinsRandomSource *inRandSource,
                             int *inSquare, char inFirstPickRandom ) {

    PickOrder ourPicks;
    
    if( inFirstPickRandom ) {
        ourPicks = 
            allPickOrders[ inRandSource->getRandomBoundedInt( 0, 719 ) ];
    
        for( int m=2; m<6; m++ ) {
            ourPicks.picks[m] = -1;
            }
        }
    else {
        ourPicks = getEmptyPickOrder();
        }
            

    PickOrder theirPicks = 
        allPickOrders[ inRandSource->getRandomBoundedInt( 0, 719 ) ];
        
    int mStart=0;
    
    if( inFirstPickRandom ) {
        mStart = 1;
        }
    
    for( int m=mStart; m<3; m++ ) {
            
        PickOrder theirPartialPicks = getEmptyPickOrder();
            
        for( int pp=0; pp < m; pp ++ ) {
            theirPartialPicks.picks[ pp * 2 + 1 ] = 
                theirPicks.picks[ pp * 2 + 1 ];
            }
        //printf( "Their picks = " );
        //printPickOrder( theirPartialPicks );
            
        ourPicks = findBestMove( inSquare, ourPicks, theirPartialPicks );
        }
        
    MagicSquareGameState state( inSquare );

    for( int m=0; m<6; m++ ) {
        state.makeMoveInternal( 0, ourPicks.picks[m] );
        state.makeMoveInternal( 1, theirPicks.picks[m] );
        }
    if( state.getScore( 0 ) > state.getScore( 1 ) ) {
        return 1;
        }
    else {
        return 0;
        }
    }



static int runTestB( int inSquareSeed ) {
    JenkinsRandomSource randSource( inSquareSeed );

    int *squareA = generateMagicSquare6( 10 + inSquareSeed );

    int winCount = 0;
    int numTries = 40;
    for( int t=0; t<numTries; t++ ) {
        winCount += playGameVsRandom( &randSource, squareA, false );
        }
    
    printf( "Win rate = %f\n", (float)winCount / (float)numTries );

    /*
    winCount = 0;
    for( int t=0; t<numTries; t++ ) {
        winCount += playGameVsRandom( &randSource, squareA, true );
        }
    
    printf( "Win rate (first move random) = %f\n", 
            (float)winCount / (float)numTries );
    */
    delete [] squareA;

    return winCount;
    }


int main() {
    
    fillAllPickOrders();
    
    
    int bestWinCount = 0;
    
    for( int r=0; r<10; r++ ) {
        printf( "r=%d\n", r );

        int winCount = runTestB( r );
        }
    }
