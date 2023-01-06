
#include <math.h>
#include <stdio.h>


enum cell{ Empty=0, Planted=1 }; 


int columnSize = 5;

int numStates;
cell **allStates;

int *allCosts;

// array indicating when a center column of cells is covered by
// the columns on either side of it.
// indexed as isCovered[c1][c2][c3]   true if c1 and c3 cover c2,
// or false otherwise
char ***isCoveredArray;


char isCovered( int inC1, int inC2, int inC3 ) {
    cell *c1 = allStates[ inC1 ];
    cell *c2 = allStates[ inC2 ];
    cell *c3 = allStates[ inC3 ];

    for( int i=0; i<columnSize; i++ ) {
        // test for a cell that is not covered and return false
        if( c2[i] == Planted &&
            c1[i] == Planted &&
            c3[i] == Planted ) {

            if( i <=0 || c2[i-1] == Planted ) {
                if( i >=columnSize-1 || c2[i+1] == Planted ) {

                    return false;
                    }
                }
            }
        }

    // else all covered
    return true;
    }


void printState( int inStateIndex ) {
    cell *state = allStates[ inStateIndex ];

    for( int i=0; i<columnSize; i++ ) {
        if( state[i] == Empty ) {
            printf( "O" );
            }
        else {
            printf( "X" );
            }
        }
    printf( "\n" );
    }


struct costStatePair {
        int cost;
        // the index in the allStates array
        int stateIndex;
    };


// table for memoizing optLayout
struct costStatePair ***optLayoutTable;



// place where optLayout results are returned
struct costStatePair optLayoutResult;

/**
 * Computes the optimum layout for a given number of columns given
 * states for the first two columns.
 *
 * @param inNumColumns the number of columns.  Must be at least 3.
 * @param inFirstColumnStateIndex the index of the state (in allStates)
 *   of the first column.
 * @param inSecondColumnStateIndex the index of the state (in allStates)
 *   of the second column.
 *
 * @return result is set in the optLayoutResult structure and returned.
 *   Thus, this call is NOT threadsafe.   
 */
struct costStatePair optLayout( int inNumColumns, int inFirstColumnStateIndex,
                                int inSecondColumnStateIndex ) {

    int c = inNumColumns-1;
    if( optLayoutTable[c]
                      [inFirstColumnStateIndex]
                      [inSecondColumnStateIndex].cost != -1 ) {
        // already have this result
        return optLayoutTable[c]
                             [inFirstColumnStateIndex]
                             [inSecondColumnStateIndex];
        }
    
    if( inNumColumns < 3 ) {
        optLayoutResult.cost =
            allCosts[ inFirstColumnStateIndex ] +
            allCosts[ inSecondColumnStateIndex ];
        optLayoutResult.stateIndex = -1;


        // save in table
        optLayoutTable[c]
            [inFirstColumnStateIndex]
            [inSecondColumnStateIndex].cost = optLayoutResult.cost;
        optLayoutTable[c]
            [inFirstColumnStateIndex]
            [inSecondColumnStateIndex].stateIndex = optLayoutResult.stateIndex;
        
        
        return optLayoutResult;
        }
            
    
    
    int bestCost = inNumColumns * columnSize + 1;
    int bestStateIndex = -1;

    // examin all possible states for the third column
    for( int i=0; i<numStates; i++ ) {

        // if i does its part to cover our second column
        if( isCovered( inFirstColumnStateIndex,
                       inSecondColumnStateIndex, i ) ) {

            char iCovered = false;
            
            int costOfRest = 0;
            if( inNumColumns > 3 ) {
                struct costStatePair result =
                    optLayout( inNumColumns - 1,
                               inSecondColumnStateIndex,
                               i );
                costOfRest = result.cost -
                    allCosts[ inSecondColumnStateIndex ];

                // optLayout will only pick a 3rd column that helps to
                // cover i
                iCovered = true;
                }
            else {
                // just the cost of i by itself
                costOfRest = allCosts[i];

                // make sure that i is covered by inSecondColumnStateIndex
                // put the all-planted column (0) on the other side of i
                iCovered = isCovered( inSecondColumnStateIndex, i, 0 );
                }

            if( iCovered && costOfRest < bestCost ) {
                bestCost = costOfRest;
                bestStateIndex = i;
                }
            }
        }
    
    optLayoutResult.cost = bestCost +
        allCosts[ inFirstColumnStateIndex ] +
        allCosts[ inSecondColumnStateIndex ];
    
    optLayoutResult.stateIndex = bestStateIndex;


    // save in table
    optLayoutTable[c]
        [inFirstColumnStateIndex]
        [inSecondColumnStateIndex].cost = optLayoutResult.cost;
    optLayoutTable[c]
        [inFirstColumnStateIndex]
        [inSecondColumnStateIndex].stateIndex = optLayoutResult.stateIndex;

    
    return optLayoutResult;
    }




void printOptLayout( int inNumColumns ) {
    // find best starting state for 10 columns by
    // running optLayout on 11 columns
    int bestFirstColumn = -1;
    int bestSecondColumn = -1;
    int bestCost = (inNumColumns+1) * columnSize + 1;

    
    
    for( int i=0; i<numStates; i++ ) {
        // state 0 is the all-planted state
        struct costStatePair result = optLayout( inNumColumns+1, 0, i );

        if( result.cost < bestCost ) {
            bestFirstColumn = i;
            bestSecondColumn = result.stateIndex;
            bestCost = result.cost;            
            }
        //printf( "best cost = %d\n", bestCost );
        }

    printf( "%d empty cells:\n", bestCost );
    printState( bestFirstColumn );
    printState( bestSecondColumn );
    
    for( int c=inNumColumns; c>=3; c-- ) {

        struct costStatePair restult =
            optLayout( c, bestFirstColumn, bestSecondColumn );
        
        printState( restult.stateIndex );

        bestFirstColumn = bestSecondColumn;
        bestSecondColumn = restult.stateIndex;
        }
    }






int main( int inNumArgs, char **inArgs ) {

    int numColumns = 10;

    if( inNumArgs > 1 ) {
        sscanf( inArgs[1], "%d", &columnSize );
        }
    if( inNumArgs > 2 ) {
        sscanf( inArgs[2], "%d", &numColumns );
        }
    
    
    numStates = (int)( pow( 2, columnSize ) );
    allStates = new cell*[ numStates ];
    allCosts = new int[ numStates ];


    
    
    
    int i, j, k;
    for( i=0; i<numStates; i++ ) {

        allCosts[i] = 0;

        allStates[i] = new cell[ columnSize ];
        for( int b=0; b<columnSize; b++ ) {

            if( ( ( i >> b ) & 0x1 ) == 1 ) {
                allStates[i][b] = Empty;
                allCosts[i] ++;
                }
            else {
                allStates[i][b] = Planted;
                }
            }
        }

    int numEntries = 0;
    optLayoutTable = new struct costStatePair **[numColumns +1];
    int c;
    for( c=0; c<numColumns+1; c++ ) {
        optLayoutTable[c] = new struct costStatePair *[numStates];
        for( int i=0; i<numStates; i++ ) {
            optLayoutTable[c][i] = new struct costStatePair[ numStates ];

            for( int j=0; j<numStates; j++ ) {
                optLayoutTable[c][i][j].cost = -1;
                optLayoutTable[c][i][j].stateIndex = -1;

                numEntries ++;
                }
            }            
        }
    printf( "Done constructing memoization table with %d entries\n",
            numEntries );
    
    /*
    isCoveredArray = new char**[ numStates ];
    for( i=0; i<numStates; i++ ) {
        isCoveredArray[i] = new char*[numStates];
        for( j=0; j<numStates; j++ ) {
            isCoveredArray[i][j] = new char[numStates];
            for( k=0; k<numStates; k++ ) {
                isCoveredArray[i][j][k] = isCovered( i, j, k );
                }
            }
        }
    */

    /*
    for( i=0; i<numStates; i++ ) {
        for( j=0; j<numStates; j++ ) {
            delete [] isCoveredArray[i][j];
            }
        delete [] isCoveredArray[i];
        }
    delete [] isCoveredArray;
    */



    for( c=2; c<=numColumns; c++ ) {
        printf( "\n%d by %d\n", columnSize, c );

        printOptLayout( c );
        }

    
    


    for( c=0; c<numColumns+1; c++ ) {
        
        for( int i=0; i<numStates; i++ ) {
            delete [] optLayoutTable[c][i];
            }
        delete [] optLayoutTable[c];    
        }
    delete [] optLayoutTable;
    
    for( i=0; i<numStates; i++ ) {
        delete [] allStates[i];
        }
    delete [] allStates;
    delete [] allCosts;
    
    return 0;
    }

