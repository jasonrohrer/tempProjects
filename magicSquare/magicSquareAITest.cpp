

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



int main() {

    printf( "Generating square\n" );
    
    int *squareA = generateMagicSquare6( 10 );
    
    printf( "SquareA:\n" );
    printSquare( squareA, 6 );
    

    MagicSquareGameState startState( squareA );

    startState.printState();
    

    /*
    printf( "\n\nTest:\n\n" );
    
    SimpleVector<GameState *> possMoves = startState.getPossibleMoves();
    
    for( int i=0; i<possMoves.size(); i++ ) {
        GameState *move = *( possMoves.getElement( i ) );

        int score = minMax( move, min );
        
        printf( "Possible move with score %d:\n", score );
        
        move->printState();
        delete move;
        }
    
    printf( "\n\n" );
    */
    


    MinOrMax nextPlayer = max;
    
    GameState *nextState = minMaxPickMove( &startState, nextPlayer, -1 );

    while( ! nextState->getGameOver() ) {
        switch( nextPlayer ) {
            case max:
                nextPlayer = min;
                printf( "P1 move:\n" );
                break;
            case min:
                nextPlayer = max;
                printf( "P2 move:\n" );
                break;
            }

        nextState->printState();
        

        GameState * temp = nextState;
        
        if( nextPlayer == min ) {
            int pick = -1;
            while( pick < 0 ) {
                printf( "Pick a column (0 - 5):\n> " );
                scanf( "%d", &pick );

                if( pick > 5 || pick < 0 ) {
                    pick = -1;
                    }
                else {
                    MagicSquareGameState *magicSate = 
                        (MagicSquareGameState*)nextState;
                    
                    for( int t=0; t<6; t++ ) {
                        if( magicSate->mPlayerMoves[1][t] == pick ) {
                            printf( "Column %d already picked\n", pick );
                            pick = -1;
                            break;
                            }
                        }
                    }
                }
            // good pick
            nextState = nextState->copy();
            MagicSquareGameState *magicSate = 
                (MagicSquareGameState*)nextState;
            for( int t=0; t<6; t++ ) {
                if( magicSate->mPlayerMoves[1][t] == -1 ) {
                    magicSate->mPlayerMoves[1][t] = pick;
                    break;
                    }
                }
            }
        else {
            nextState = minMaxPickMove( nextState, nextPlayer, -1 );
            }
        
        delete temp;
        }
 

    switch( nextPlayer ) {
        case max:
            nextPlayer = min;
            printf( "P1 move:\n" );
            break;
        case min:
            nextPlayer = max;
            printf( "P2 move:\n" );
            break;
        }
    
    nextState->printState();
    
    printf( "Final score = %d\n", nextState->getScore() );
    

    
    delete nextState;

    delete [] squareA;
    
    
    return 1;
    }
