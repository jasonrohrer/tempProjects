

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




class SquarePlayer {
        
    public:
        virtual ~SquarePlayer() {
            }
        
        virtual GameState *pickMove( 
            MagicSquareGameState *inState ) = 0;
    };


class MinMaxSquarePlayer : public SquarePlayer {
    public:
        MinMaxSquarePlayer( MinOrMax inPlayerSide ) 
                : mPlayerSide( inPlayerSide ) {}
        
        virtual GameState *pickMove( 
            MagicSquareGameState *inState ) {
            
            return minMaxPickMove( inState, mPlayerSide, -1 );
            }
        
    protected:
        MinOrMax mPlayerSide;
    };




class HumanPlayer : public SquarePlayer {
    public:
        HumanPlayer( int inPlayerNumber )
                : mPlayerNumber( inPlayerNumber ) {
            switch( inPlayerNumber ) {
                case 0:
                    mMoveName = "row";
                    break;
                default:
                    mMoveName = "column";
                    break;
                };
            }
        
        virtual GameState *pickMove( 
            MagicSquareGameState *inState ) {
            

            int pick = -1;
            while( pick < 0 ) {
                printf( "Pick a %s (0 - 5):\n> ", mMoveName );
                scanf( "%d", &pick );

                if( pick > 5 || pick < 0 ) {
                    pick = -1;
                    }
                else {
                    MagicSquareGameState *magicSate = 
                        (MagicSquareGameState*)inState;
                    
                    for( int t=0; t<6; t++ ) {
                        if( magicSate->mPlayerMoves[1][t] == pick ) {
                            printf( "%s %d already picked\n", 
                                    mMoveName, pick );
                            pick = -1;
                            break;
                            }
                        }
                    }
                }
            // good pick
            MagicSquareGameState *nextState
                = (MagicSquareGameState *)inState->copy();
            for( int t=0; t<6; t++ ) {
                if( nextState->mPlayerMoves[mPlayerNumber][t] == -1 ) {
                    nextState->mPlayerMoves[mPlayerNumber][t] = pick;
                    break;
                    }
                }
            return nextState;
            }
        
    protected:
        int mPlayerNumber;
        const char *mMoveName;
        
    };



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
    
    SquarePlayer *players[2];
    

    MinMaxSquarePlayer player1( max );
    MinMaxSquarePlayer player2( min );

    //HumanPlayer player1( 0 );
    //HumanPlayer player2( 1 );
    

    players[0] = &player1;
    players[1] = &player2;


    int nextPlayer = 0;
    
    GameState *nextState = 
        players[0]->pickMove( &startState );

    while( ! nextState->getGameOver() ) {
        switch( nextPlayer ) {
            case 0:
                nextPlayer = 1;
                printf( "P1 move:\n" );
                break;
            case 1:
                nextPlayer = 0;
                printf( "P2 move:\n" );
                break;
            }

        nextState->printState();
        

        GameState * temp = nextState;
        
        nextState = 
            players[nextPlayer]->pickMove( (MagicSquareGameState*)nextState );
                
        delete temp;
        }
 

    switch( nextPlayer ) {
        case 0:
            nextPlayer = 1;
            printf( "P1 move:\n" );
            break;
        case 1:
            nextPlayer = 0;
            printf( "P2 move:\n" );
            break;
        }
        
    nextState->printState();
    
    printf( "Final minmax score = %d\n", nextState->getScore() );
    
    MagicSquareGameState *magicState = (MagicSquareGameState*)nextState;
    printf( "Final score = P1: %d   P2: %d\n", 
            magicState->getScore( 0 ), magicState->getScore( 1 ) );
    

    
    delete nextState;

    delete [] squareA;
    
    
    return 1;
    }
