#include "magicSquare6.h"
#include "MagicSquareGameStateTwoPick.h"


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



static MinOrMax switchSide( MinOrMax inSide ) {
    if( inSide == min ) {
        return max;
        }
    return min;
    }







static void runMinMaxVsPeekingGreedy( int inSquareSeed ) {
    

    int *squareA = generateMagicSquare6( 10 + inSquareSeed );

    //printSquare( squareA, 6 );

    MagicSquareGameStateTwoPick state( squareA );

    //int score = minMax( &state, max );
    
    //printf( "Best worst-case score = %d\n", score );
    

    GameState *currentState = state.copy();
    
    MinOrMax next = max;
    
    while( !currentState->getGameOver() ) {
        
        GameState *nextState;
        if( next == max ) {
            nextState = minMaxPickMove( currentState, next );
            }
        else {
            // p2 peeks at p1's move and makes greedy pick

            SimpleVector<GameState *> possibleMoves = 
                currentState->getPossibleMoves();
            
            int minScore = 200;
            
            GameState *minScoreMove = NULL;
            
            for( int i=0; i<possibleMoves.size(); i++ ) {
                GameState *move = possibleMoves.getElementDirect( i );
                
                int score = move->getScore();
                
                if( score < minScore ) {
                    minScore = score;
                    
                    if( minScoreMove != NULL ) {
                        delete minScoreMove;
                        }
                    minScoreMove = move->copy();    
                    }
                delete move;
                }
            
            //printf( "Greedy settled on move with score %d\n", 
            //        minScoreMove->getScore() );
            
            
            nextState = minScoreMove;
            }
        

        delete currentState;
        currentState = nextState;
        next = switchSide( next );
        }
    
    //printf( "Final state:\n" );
    //currentState->printState();
    
    
    printf( "Final score = %d\n", currentState->getScore() );


    delete currentState;
        

    delete [] squareA;
    }



// max player (p1) searches all possible moves
// min player (p2) considers only single-turn greedy move
// returns highest seen score
static int vsGreedyTreeSearch( GameState *inState, MinOrMax inNext ) {
    if( inState->getGameOver() ) {
        if( inState->getScore() > 0 ) {
            printf( "Found forced positive score vs. greedy: %d\n",
                    inState->getScore() );
            inState->printState();
            }
        return inState->getScore();
        }
    
    if( inNext == max ) {
        SimpleVector<GameState *> possibleMoves = 
            inState->getPossibleMoves();

        int highScore = -200;
        
        for( int i=0; i<possibleMoves.size(); i++ ) {
            GameState *move = possibleMoves.getElementDirect( i );
            
            int score = vsGreedyTreeSearch( move, min );
            
            if( score > highScore ) {
                highScore = score;
                }
            delete move;
            }
        return highScore;
        }
    else {
        int minScore = 200;
        
        GameState *minScoreMove = NULL;
        
        SimpleVector<GameState *> possibleMoves = 
            inState->getPossibleMoves();

        for( int i=0; i<possibleMoves.size(); i++ ) {
            GameState *move = possibleMoves.getElementDirect( i );
            
            int score = move->getScore();
            
            if( score < minScore ) {
                minScore = score;
                
                if( minScoreMove != NULL ) {
                    delete minScoreMove;
                    }
                minScoreMove = move->copy();    
                }
            delete move;
            }
        int score = vsGreedyTreeSearch( minScoreMove, max );
        
        delete minScoreMove;
        
        return score;
        }
    }


static void runVsGreedyTreeSearch( int inSquareSeed ) {
    

    int *squareA = generateMagicSquare6( 10 + inSquareSeed );

    //printSquare( squareA, 6 );

    MagicSquareGameStateTwoPick state( squareA );

    int highScore = vsGreedyTreeSearch( &state, max );
    printf( "Highest score = %d\n", highScore );
    
    if( highScore > 0 ) {
        printSquare( squareA, 6 );
        
        int score = minMax( &state, max );
    
        printf( "Best worst-case score (minmax vs minmax) = %d\n", score );

        }
    
    delete [] squareA;
    }




static void runMinMaxSearch( int inSquareSeed ) {
    

    int *squareA = generateMagicSquare6( 10 + inSquareSeed );

    //printSquare( squareA, 6 );

    MagicSquareGameStateTwoPick state( squareA );


    int score = minMax( &state, max );
    
    printf( "Best worst-case score (minmax vs minmax) = %d\n", score );
    
    delete [] squareA;
    }


int main() {

    
    for( int r=0; r<100; r++ ) {

        //runMinMaxVsPeekingGreedy( r );
        runVsGreedyTreeSearch( r );
        //runMinMaxSearch( r );
        }
    }
