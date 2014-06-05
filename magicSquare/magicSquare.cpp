

#include <stdio.h>

/*
int square[6][6] = {
    { 6, 32, 3, 34, 35, 1 },
    { 7, 11, 27, 28, 8, 30 },
    { 19, 14, 16, 15, 23, 24 },
    { 18, 20, 22, 21, 17, 13 },
    { 25, 29, 10, 9, 26, 12 },
    { 36, 5, 33, 4, 2, 31 }
    };
*/

int square[36] = {
    6, 32, 3, 34, 35, 1,
    7, 11, 27, 28, 8, 30,
    19, 14, 16, 15, 23, 24,
    18, 20, 22, 21, 17, 13,
    25, 29, 10, 9, 26, 12,
    36, 5, 33, 4, 2, 31
    };

/*
int square[6][6] = {
    { 28, 21, 1, 14, 17, 30 },
    { 22, 3, 26, 33, 12, 15 },
    { 6, 35, 10, 5, 24, 31 },
    { 29, 4, 25, 34, 11, 8 },
    { 19, 32, 13, 2, 27, 18 },
    { 7, 16, 36, 23, 20, 9 }
    };
*/  


char printSquare( int *inArray, int inD ) {
    for( int y=0; y<inD; y++ ) {
        for( int x=0; x<inD; x++ ) {
            printf( "%3d ", inArray[y * inD + x] );
            }
        printf("\n" );
        }
    }



char checkMagic( int *inArray, int inD ) {
    int magicSum = ( inD * ( inD * inD + 1 ) ) / 2;

    for( int i=0; i<inD; i++ ) {
        int colSum = 0;
        int rowSum = 0;
        
        for( int j=0; j<inD; j++ ) {
            colSum += inArray[j * inD + i];
            rowSum += inArray[i * inD + j];
            }
        
        if( colSum != magicSum || rowSum != magicSum ) {
            return false;
            }
        }
    
    int diagASum = 0;
    int diagBSum = 0;
    
    for( int i=0; i<inD; i++ ) {
        diagASum += inArray[i * inD + i];
        diagBSum += inArray[(inD - i - 1) * inD + i];
        }
    if( diagASum != magicSum || diagBSum != magicSum ) {
        return false;
        }

    return true;
    }



char checkMagicRow( int *inArray, int inD, int inRowNumber ) {
    int magicSum = ( inD * ( inD * inD + 1 ) ) / 2;

    int rowSum = 0;
        
    for( int j=0; j<inD; j++ ) {
        rowSum += inArray[inRowNumber * inD + j];
        }

    return ( rowSum == magicSum );
    }


char checkMagicColumnUnder( int *inArray, int inD, int inColumnNumber ) {
    int magicSum = ( inD * ( inD * inD + 1 ) ) / 2;

    int colSum = 0;
        
    for( int j=0; j<6; j++ ) {
        colSum += inArray[j * inD + inColumnNumber];
        }

    return ( colSum <= 111 );
    }





int stepCount = 0;

char fillMagic( int *inArray, int inD, int inNextPosToFill, 
                char *inUnusedNumberMap ) {
    /*
    if( stepCount >= 10000000 ) {
        return false;
        }
    stepCount ++;
    */
    /*
      printSquare( inArray );
      printf( "\n\n" );
    */
    
    int numCells = inD * inD;

    if( inNextPosToFill == numCells ) {
        //printSquare( inArray );
        //printf( "\n\n" );

        return checkMagic( inArray, inD );
        }
    /*
    // walk through remaining available numbers and try each one
    // in next pos
    int remainingNumbers[36];
    
    int remainingCount = 0;
    for( int i=1; i<=36; i++ ) {
        
        char alreadyUsed = false;
        for( int j=0; j<inNextPosToFill; j++ ) {
            if( inArray[j/6][j%6] == i ) {
                alreadyUsed = true;
                break;
                }
            }
        
        if( !alreadyUsed ) {
            remainingNumbers[ remainingCount ] = i;
            remainingCount++;
            }
        }
    */

    int posToFillY = inNextPosToFill / inD;
    int posToFillX = inNextPosToFill % inD;

    /*
    printf( "Unused map:\n" );
    for( int i=0; i<36; i++ ) {
        printf( "%d ", inUnusedNumberMap[i] );
        }
    printf( "\n\n" );
    */
    for( int i=0; i<numCells; i++ ) {
        
        if( inUnusedNumberMap[i] ) {
            inUnusedNumberMap[i] = false;
            
            inArray[inNextPosToFill] = i+1;
            
            /*
            printf( "%d: Attempting to fill pos %d,%d with %d\n",
                    inNextPosToFill, posToFillX, posToFillY, i+1 );
            */
            
            char shouldRecurse = true;

            if( posToFillX == 5 ) {
                // completing a row
                // don't bother recursing if this row is not a valid
                // magic row
                if( ! checkMagicRow( inArray, inD, posToFillY ) ) {
                    shouldRecurse = false;
                    }
                }
            
            /*
            if( shouldRecurse && 
                // at least three cells full, otherwise, never have
                // sum overflow
                posToFillY > 3 &&
                ! checkMagicColumnUnder( inArray, inD, posToFillX ) ) {
                // column overflow
                shouldRecurse = false;
                }
            */

                
            if( shouldRecurse ) {    
                if( fillMagic( inArray, inD, 
                               inNextPosToFill+1, inUnusedNumberMap ) ) {
                    return true;
                    }            
                }
            
            inArray[inNextPosToFill] = 0;

            // using this number in this position didn't work
            inUnusedNumberMap[i] = true;
            }
        }
    }



#include "minorGems/util/random/CustomRandomSource.h"
#include "minorGems/util/SimpleVector.h"


CustomRandomSource randSource( time( NULL ) );


char fillMagicRandom( int *inArray, int inD ) {
    int maxNumber = inD * inD;
        
    for( int i=0; i<maxNumber; i++ ) {
        inArray[i] = i + 1;
        }
    
    // Knuth/Durstenfeld/Fisher/Yates shuffle
    
    for( int i=0; i<maxNumber; i++ ) {
        
        int swapPos = randSource.getRandomBoundedInt( i, maxNumber - 1 );
        
        int temp = inArray[i];
        inArray[i] = inArray[swapPos];
        inArray[swapPos] = temp;
        }
    }



int measureMagicDeviation( int *inArray, int inD ) {
    int magicSum = ( inD * ( inD * inD + 1 ) ) / 2;
    
    int totalDeviation = 0;
    
    for( int i=0; i<inD; i++ ) {
        int colSum = 0;
        int rowSum = 0;
        
        for( int j=0; j<inD; j++ ) {
            colSum += inArray[j * inD + i];
            rowSum += inArray[i * inD + j];
            }
        
        if( colSum > magicSum ) {
            totalDeviation += ( colSum - magicSum );
            }
        else {
            totalDeviation += ( magicSum - colSum );
            }

        if( rowSum > magicSum ) {
            totalDeviation += ( rowSum - magicSum );
            }
        else {
            totalDeviation += ( magicSum - rowSum );
            }
        }
    
    int diagASum = 0;
    int diagBSum = 0;
    
    for( int i=0; i<inD; i++ ) {
        diagASum += inArray[i * inD + i];
        diagBSum += inArray[(inD - i - 1) * inD + i];
        }

    
    if( diagASum > magicSum ) {
        totalDeviation += ( diagASum - magicSum );
        }
    else {
        totalDeviation += ( magicSum - diagASum );
        }
    
    if( diagBSum > magicSum ) {
        totalDeviation += ( diagBSum - magicSum );
        }
    else {
        totalDeviation += ( magicSum - diagBSum );
        }
    
    return totalDeviation;
    }

// pick two cells and swap
// returns true, because swap always done
char swapRandom( int *inArray, int inNumCells ) {
    int swapPosA = randSource.getRandomBoundedInt( 0, inNumCells - 1 );
    
    int swapPosB = randSource.getRandomBoundedInt( 0, inNumCells - 1 );
    
    int temp = inArray[ swapPosA ];
    
    inArray[ swapPosA ] = inArray[ swapPosB ];
    inArray[ swapPosB ] = temp;
    
    return true;
    }



// tries random swap, undoes it if no improvement
// returns true if swap happened
char swapRandomOnlyImprove( int *inArray, int inD, int inNumCells ) {
    int oldDeviation = measureMagicDeviation( inArray, inD );

    int swapPosA = randSource.getRandomBoundedInt( 0, inNumCells - 1 );
    
    int swapPosB = randSource.getRandomBoundedInt( 0, inNumCells - 1 );
    
    int temp = inArray[ swapPosA ];
    
    inArray[ swapPosA ] = inArray[ swapPosB ];
    inArray[ swapPosB ] = temp;
    

    int newDeviation = measureMagicDeviation( inArray, inD );
    
    if( newDeviation < oldDeviation ) {
        return true;
        }
    
    // else swap back

    temp = inArray[ swapPosA ];
    
    inArray[ swapPosA ] = inArray[ swapPosB ];
    inArray[ swapPosB ] = temp;

    return false;
    }



// find worst pair of rows or columns and swap their elements
void swapSmart( int *inArray, int inD ) {
    int overMax = inD * inD * inD;
    
    int highestColumnIndex = -1;
    int lowestColumnIndex = -1;
    int highestColumnSum = 0;
    int lowestColumnSum = overMax;
    
    int highestRowIndex = -1;
    int lowestRowIndex = -1;
    int highestRowSum = 0;
    int lowestRowSum = overMax;
    
    for( int i=0; i<inD; i++ ) {
        int colSum = 0;
        int rowSum = 0;
        
        for( int j=0; j<inD; j++ ) {
            colSum += inArray[j * inD + i];
            rowSum += inArray[i * inD + j];
            }
        
        if( colSum > highestColumnSum ) {
            highestColumnSum = colSum;
            highestColumnIndex = i;
            }
        if( colSum < lowestColumnSum ) {
            lowestColumnSum = colSum;
            lowestColumnIndex = i;
            }

        if( rowSum > highestRowSum ) {
            highestRowSum = rowSum;
            highestRowIndex = i;
            }
        if( rowSum < lowestRowSum ) {
            lowestRowSum = rowSum;
            lowestRowIndex = i;
            }
        }
    
    if( highestRowSum > highestColumnSum ) {
        // improve these rows
        
        int bestDiffAfterSwap = overMax;
        int bestColumnToSwap = -1;
        
        int rH = highestRowIndex;
        int rL = lowestRowIndex;

        for( int i=0; i<inD; i++ ) {
            int colHighValue = inArray[ rH * inD + i ];
            int colLowValue = inArray[ rL * inD + i ];
            
            int diffIfSwap = 
                ( ( highestRowSum - colHighValue + colLowValue ) - 
                  ( lowestRowSum + colHighValue - colLowValue ) );
            
            if( diffIfSwap < 0 ) {
                diffIfSwap = -diffIfSwap;
                }
            if( diffIfSwap < bestDiffAfterSwap ) {
                bestDiffAfterSwap = diffIfSwap;
                bestColumnToSwap = i;
                }
            }
        
        int temp = inArray[ rH * inD + bestColumnToSwap ];
        inArray[ rH * inD + bestColumnToSwap ] = 
            inArray[ rL * inD + bestColumnToSwap ];
        inArray[ rL * inD + bestColumnToSwap ] = temp;
        }
    else {
        // improve these columns
        
        int bestDiffAfterSwap = overMax;
        int bestRowToSwap = -1;
        
        int cH = highestColumnIndex;
        int cL = lowestColumnIndex;

        for( int i=0; i<inD; i++ ) {
            int rowHighValue = inArray[ i * inD + cH ];
            int rowLowValue = inArray[ i * inD + cL ];
            
            int diffIfSwap = 
                ( ( highestColumnSum - rowHighValue + rowLowValue ) - 
                  ( lowestColumnSum + rowHighValue - rowLowValue ) );
            
            if( diffIfSwap < 0 ) {
                diffIfSwap = -diffIfSwap;
                }
            if( diffIfSwap < bestDiffAfterSwap ) {
                bestDiffAfterSwap = diffIfSwap;
                bestRowToSwap = i;
                }
            }
        
        int temp = inArray[ bestRowToSwap * inD + cH ];
        inArray[ bestRowToSwap * inD + cH ] = 
            inArray[ bestRowToSwap * inD + cL ];
        inArray[ bestRowToSwap * inD + cL ] = temp;
        }
    
    }



typedef struct swap {
        int indexA;
        int indexB;
    } swap;


// make swap that makes the biggest improvement to deviation score
// steepest descent
// returns true if improving swap possible
char swapBest( int *inArray, int inD, int inNumCells ) {
    
    swap bestSwap = { -1, -1 };
    int bestSwapDeviation = measureMagicDeviation( inArray, inD );
    
    for( int i=0; i<inNumCells; i++ ) {
        for( int j=i+1; j<inNumCells; j++ ) {
            
            int temp = inArray[i];
            inArray[i] = inArray[j];
            inArray[j] = temp;
            
            int deviation = measureMagicDeviation( inArray, inD );
            if( deviation < bestSwapDeviation ) {
                bestSwapDeviation = deviation;
                bestSwap.indexA = i;
                bestSwap.indexB = j;
                }

            // swap back
            temp = inArray[i];
            inArray[i] = inArray[j];
            inArray[j] = temp;
            }
        }
    

    if( bestSwap.indexA != -1 ) {
        // found best improving swap
        
        // swap it
        int i = bestSwap.indexA;
        int j = bestSwap.indexB;

        int temp = inArray[i];
        inArray[i] = inArray[j];
        inArray[j] = temp;
        return true;
        }
    else {
        return false;
        }
    }



// uses a series of best swaps from this point until bottom hit
void findBottomSteepest( int *inArray, int inD ) {
    int numCells = inD * inD;
    
    while( ! checkMagic( inArray, inD ) ) {

        char swapMade = swapBest( inArray, inD, numCells );
        
        if( !swapMade ) {
            // hit bottom
            break;
            }
        }
    }



// uses a series of random improving moves until stuck, then
// steepest descent from there
void findBottomAnyImprovementThenSteepest( int *inArray, int inD ) {
    int numCells = inD * inD;
    

    int numTries = 0;
    
    while( numTries < 10000 ) {
        
        char found = swapRandomOnlyImprove( inArray, inD, numCells );
        numTries++;
        
        if( found ) {
            // start over, looking for another improving move
            numTries = 0;
            }
        }

    // finish up with steepest descent from give-up point
    findBottomSteepest( inArray, inD );
    }



// try limit:  how many hit-bottom bounces before we give
// up and start from a new random starting state
void findMagicSquareSteepestBounce( int *inArray, int inD, int inNumBounces,
                                    int inTryLimit = -1 ) {
    int numCells = inD * inD;
    
    int numBounces = 0;
    while( ! checkMagic( inArray, inD ) ) {
        
        if( inTryLimit > 0 && numBounces > inTryLimit ) {
            fillMagicRandom( inArray, inD );
            numBounces = 0;
            }

        numBounces++;        

        for( int i=0; i<inNumBounces; i++ ) {
            swapRandom( inArray, numCells );
            }
        
        findBottomSteepest( inArray, inD );
        }
    }

    
    


// returns new square
int *improveMutateSquare( int *inArray, int inD ) {
    int numCells = inD * inD;
    
    int *newSquare = new int[ numCells ];
    
    memcpy( newSquare, inArray, sizeof(int) * numCells );
    
    int oldDeviation = measureMagicDeviation( inArray, inD );
    int newDeviation = oldDeviation;

    int numTries = 0;
    
    while( numTries < 10000 &&
           newDeviation >= oldDeviation ) {
        

        swapRandom( newSquare, numCells );
        
        // this doesn't seem to work to find magic squares
        // even if only used for 1% of swaps.
        // random swaps are better
        //swapSmart( newSquare, inD );
        
        //swapBest( newSquare, inD, numCells );

        
        newDeviation = measureMagicDeviation( newSquare, inD );
        
        if( newDeviation > oldDeviation ) {
            // unswap
            memcpy( newSquare, inArray, sizeof(int) * numCells );
            }
        
        numTries ++;
        }
        

    return newSquare;
    }





int *anyMutateSquare( int *inArray, int inD ) {

    int numCells = inD * inD;
    
    int *newSquare = new int[ numCells ];
    
    memcpy( newSquare, inArray, sizeof(int) * numCells );
    
    swapRandom( newSquare, numCells );
    
    return newSquare;
    }




int count = 0;

void printValues( int *inValues, int inNumValues, void *inUnused ) {
    for( int i=0; i<inNumValues; i++ ) {
        printf( "%d ", inValues[i] );
        }
    printf( "\n" );
    count++;
    }



void callOnAllPerm( int *inValuesToPermute, int inNumValues, int inNumSkip,
                    void inProcessPermutationCallback( int *inValues,
                                                       int inNumValues,
                                                       void *inExtraParam ),
                    void *inExtraParam ) {

    if( inNumSkip >= inNumValues ) {
        inProcessPermutationCallback( inValuesToPermute, inNumValues,
                                      inExtraParam );
        return;
        }
    

    // pick value to stick in spot at inNumSkip
    
    for( int i=inNumSkip; i<inNumValues; i++ ) {
        
        int temp = inValuesToPermute[inNumSkip];
        inValuesToPermute[ inNumSkip ] = inValuesToPermute[i];
        inValuesToPermute[i] = temp;
        
        callOnAllPerm( inValuesToPermute, inNumValues, inNumSkip+1,
                       inProcessPermutationCallback, inExtraParam );
        // revert
        inValuesToPermute[i] = inValuesToPermute[ inNumSkip ];
        inValuesToPermute[ inNumSkip ] = temp;
        }
    }



typedef struct FixedMove {
        int *indices;
        int numIndices;
    } FixedMove;


int playerAWinCount = 0;
int playerBWinCount = 0;
int tieCount = 0;

void checkBothPlayerMovesForTie( int *inValues, int inNumValues,
                                 // pointer to a FixedMove for player A
                                 void *inPlayerAFixedMove ) {

    FixedMove *m = (FixedMove *)inPlayerAFixedMove;
    
    int playerAScore = 0;
    int playerBScore = 0;
    
    for( int i=0; i<inNumValues; i++ ) {
        int turnScore = square[ m->indices[i] *  inNumValues + inValues[i] ];
        if( i%2 == 0 ) {
            playerAScore += turnScore;
            }
        else {
            playerBScore += turnScore;
            }
        }
    
    if( playerAScore > playerBScore ) {
        playerAWinCount++;
        }
    else if( playerBScore > playerAScore ) {
        playerBWinCount++;
        }
    else {
        tieCount++;

        /*
        printf( "Tie:\n" );

        for( int i=0; i<inNumValues; i++ ) {
            int turnScore = square[ m->indices[i] ]
                                  [ inValues[i] ];
            
            printf( "A picks %d, B picks %d, ",
                    m->indices[i], inValues[i] );

            if( i%2 == 0 ) {
                printf( "A scores %d\n", turnScore );
                }
            else {
                printf( "B scores %d\n", turnScore );
                }
            }
        printf( "\n" );
        */
        }
    }




void checkFixedPlayerAMove( int *inValues, int inNumValues, void *inUnused ) {
    
    int indices[6] = { 0, 1, 2, 3, 4, 5 };
    
    FixedMove m;
    m.indices = inValues;
    m.numIndices = inNumValues;

    callOnAllPerm( indices, 6, 0, checkBothPlayerMovesForTie, &m );
    }




#include "minorGems/system/Time.h"



int main() {

    printf( "Test\n" );

    // too slow.... hmmm...

    #define testD 6
    #define testNumCells 36

    char unusedMap[ testNumCells ];
    int testSquare[ testNumCells ];
    
    for( int i=0; i<testNumCells; i++ ) {
        unusedMap[i] = true;
        testSquare[i] = 0;
        }    
    


    #define MAX_LIMITS 8
    int limitValues[MAX_LIMITS];
    
    for( int i=0; i<MAX_LIMITS; i++ ) {
        limitValues[i] = 25 * ( i+1 );
        }

    double limitTimeTotal[MAX_LIMITS];
    double limitWorstTime[MAX_LIMITS];
    
    for( int i=0; i<MAX_LIMITS; i++ ) {
        limitValues[i] = 25 * ( i+1 );
        
        limitTimeTotal[i] = 0;
        limitWorstTime[i] = 0;
        }
    
    int numRuns = 3;
   
    for( int r=0; r<numRuns; r++ ) {
        printf( "Run %d\n", r );
        
        int seed = r + 103;
        
        randSource.reseed( seed );
        fillMagicRandom( testSquare, testD );        

        for( int b=0; b<MAX_LIMITS; b++ ) {
            printf( "   Limit %d\n", limitValues[b] );
            
            randSource.reseed( seed );
            fillMagicRandom( testSquare, testD );        

            double startTime = Time::getCurrentTime();
            findMagicSquareSteepestBounce( testSquare, 
                                           testD, 7, limitValues[b] );
            double netTime = Time::getCurrentTime() - startTime;
            limitTimeTotal[b] += netTime;

            if( netTime > limitWorstTime[b] ) {
                limitWorstTime[b] = netTime;
                }
            }
        }
    for( int b=0; b<MAX_LIMITS; b++ ) {
        printf( "%d limit average time = %f, worst time = %f\n",
                limitValues[b], limitTimeTotal[b] / numRuns, 
                limitWorstTime[b] );
        }

    return 0;

    
    
    


    //char result = fillMagic( testSquare, testD, 0, unusedMap );

    double timeDiffSum = 0;
    double timeASum = 0;
    double timeBSum = 0;

    numRuns = 3;
    int numTimingRuns = 10;
    
    #define MAX_BOUNCES 8
    int maxNumBounces = MAX_BOUNCES;
    int minNumBounces = 2;
    
    double bounceTimeTotal[MAX_BOUNCES];
    double bounceWorstTime[MAX_BOUNCES];
    
    for( int b=0; b<maxNumBounces; b++ ) {
        bounceTimeTotal[b] = 0;
        bounceWorstTime[b] = 0;
        }
    
    for( int r=0; r<numRuns; r++ ) {
        printf( "Run %d\n", r );
        
        int seed = r + 103;
        
        randSource.reseed( seed );
        fillMagicRandom( testSquare, testD );        

        for( int b=minNumBounces; b<maxNumBounces; b++ ) {
            printf( "   Bounce %d\n", b );
            
            randSource.reseed( seed );
            fillMagicRandom( testSquare, testD );        

            double startTime = Time::getCurrentTime();
            findMagicSquareSteepestBounce( testSquare, testD, b, 25 );
            double netTime = Time::getCurrentTime() - startTime;
            bounceTimeTotal[b] += netTime;

            if( netTime > bounceWorstTime[b] ) {
                bounceWorstTime[b] = netTime;
                }
            }
        }
    for( int b=minNumBounces; b<maxNumBounces; b++ ) {
        printf( "%d bounces average time = %f, worst time = %f\n",
                b, bounceTimeTotal[b] / numRuns, bounceWorstTime[b] );
        }
    
    
    return 0;

    int mutationCount = 0;
    int totalMutationCount = 0;

    int oldDeviation = measureMagicDeviation( testSquare, testD );
    

    int numRandomSwaps = 3;
    while( ! checkMagic( testSquare, testD ) ) {
        
        //swapRandom( testSquare, testNumCells );


        /*
        if( !swapBest( testSquare, testD, testNumCells ) ) {
            // no improving swap possible
    
            // do some random swaps
            for( int i=0; i<numRandomSwaps; i++ ) {
                swapRandom( testSquare, testNumCells );
                }
            // do more random swaps next time
            numRandomSwaps = 
                randSource.getRandomBoundedInt( 3, numRandomSwaps * 2 );
            printf( "numRandomSwaps = %d\n", numRandomSwaps );
            }
        */

        if( !swapBest( testSquare, testD, testNumCells ) ) {
            fillMagicRandom( testSquare, testD );
            }
        
                                
        int newDeviation = measureMagicDeviation( testSquare, testD );
        //printf( "Deviation old, new = %d, %d \n", oldDeviation, newDeviation );
        oldDeviation = measureMagicDeviation( testSquare, testD );
        mutationCount ++;
        totalMutationCount ++;

        if( mutationCount > 3000 ) {
            printf( "Too many mutations tried, starting over\n" );
            fillMagicRandom( testSquare, testD );
            mutationCount = 0;
            }
        }
    
    while( ! checkMagic( testSquare, testD ) ) {
        
        int *betterSquare = improveMutateSquare( testSquare, testD );

        mutationCount ++;
        totalMutationCount ++;
        
        int newDeviation = measureMagicDeviation( betterSquare, testD );
        int oldDeviation = measureMagicDeviation( testSquare, testD );
    
        //printf( "Deviation old, new = %d, %d \n", oldDeviation, newDeviation );
        
        if( newDeviation != 0 && newDeviation == oldDeviation ) {
            //printf( "Hit bottom, jumping out\n" );
            delete [] betterSquare;
            betterSquare = anyMutateSquare( testSquare, testD );
            memcpy( testSquare, betterSquare, 
                    testNumCells * sizeof( int ) );
            }
        memcpy( testSquare, betterSquare, testNumCells * sizeof( int ) );
        
        delete [] betterSquare;

        
        if( mutationCount > 400 ) {
            printf( "Too many mutations tried, starting over\n" );
            fillMagicRandom( testSquare, testD );
            mutationCount = 0;
            }
        }
    

    printf( "%d mutations done (%d total)\n", 
            mutationCount, totalMutationCount );
    
    //printf( "Result of fillMagic = %d\n", result );

    printSquare( testSquare, testD );

    char magic = checkMagic( testSquare, testD );
    
    printf( "Magic test:  %d\n", magic );

    
    //return 0;
    
    if( testNumCells == 36 ) {
        // replace main square for rest of tests below
        memcpy( square, testSquare, sizeof( int ) * testNumCells );
        }
    

    /*
    for( int x=0; x<6; x++ ) {
        int sum = 0;
        for( int y=0; y<6; y++ ) {
            sum += square[y][x];
            }
        printf( "Sum = %d\n", sum );
        }

    for( int y=0; y<6; y++ ) {
        int sum = 0;
        for( int x=0; x<6; x++ ) {
            sum += square[y][x];
            }
        printf( "Sum = %d\n", sum );
        }
    */

    int indices[6] = { 0, 1, 2, 3, 4, 5 };
    /*
    count = 0;
    callOnAllPerm( indices, 6, 0, printValues, NULL );

    printf( "Count = %d\n", count );
    */

    callOnAllPerm( indices, 6, 0, checkFixedPlayerAMove, NULL );

    int total = playerAWinCount + playerBWinCount + tieCount;
    
    printf( "PlayerA Wins %d (%f%%),"
            "\nPlayer B Wins %d (%f%%),\n"
            "Tied on %d (%f%%),\n"
            "Total %d\n\n",
            playerAWinCount, playerAWinCount * 100.0 / total,
            playerBWinCount, playerBWinCount * 100.0 / total, 
            tieCount, tieCount * 100.0 / total, total );
    
    return 1;
    }
