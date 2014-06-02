

#include <stdio.h>


int square[6][6] = {
    { 6, 32, 3, 34, 35, 1 },
    { 7, 11, 27, 28, 8, 30 },
    { 19, 14, 16, 15, 23, 24 },
    { 18, 20, 22, 21, 17, 13 },
    { 25, 29, 10, 9, 26, 12 },
    { 36, 5, 33, 4, 2, 31 }
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


char printSquare( int inArray[6][6] ) {
    for( int y=0; y<6; y++ ) {
        for( int x=0; x<6; x++ ) {
            printf( "%3d ", inArray[y][x] );
            }
        printf("\n" );
        }
    }



char checkMagic( int inArray[6][6] ) {
    int magicSum = 111;

    for( int i=0; i<6; i++ ) {
        int colSum = 0;
        int rowSum = 0;
        
        for( int j=0; j<6; j++ ) {
            colSum += inArray[j][i];
            rowSum += inArray[i][j];
            }
        
        if( colSum != magicSum || rowSum != magicSum ) {
            return false;
            }
        }
    
    int diagASum = 0;
    int diagBSum = 0;
    
    for( int i=0; i<6; i++ ) {
        diagASum += inArray[i][i];
        diagBSum += inArray[6 - i - 1][i];
        }
    if( diagASum != magicSum || diagBSum != magicSum ) {
        return false;
        }

    return true;
    }



char fillMagic( int inArray[6][6], int inNextPosToFill ) {
    
    if( inNextPosToFill == 36 ) {
        return checkMagic( inArray );
        }
    
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
    
    for( int i=0; i<remainingCount; i++ ) {
        inArray[inNextPosToFill/6][inNextPosToFill%6] = remainingNumbers[i];
        
        if( fillMagic( inArray, inNextPosToFill+1 ) ) {
            return true;
            }
        }
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
        int turnScore = square[ m->indices[i] ]
                              [ inValues[i] ];
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




int main() {

    printf( "Test\n" );

    // too slow.... hmmm...
    //char result = fillMagic( square, 25 );

    printf( "Result of fillMagic = %d\n", result );

    printSquare( square );
    
    
    char magic = checkMagic( square );
    
    printf( "Magic test:  %d\n", magic );

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
