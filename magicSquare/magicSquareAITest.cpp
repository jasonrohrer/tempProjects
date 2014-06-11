

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
        
        virtual char needsFlipForP2ToPlayAsP1() = 0;

        virtual GameState *pickMove( 
            MagicSquareGameState *inState ) = 0;

        int pickMoveColumnOrRow( MagicSquareGameState *inState ) {
            
            GameState *move = pickMove( inState );
            MagicSquareGameState *magicMove = (MagicSquareGameState*)move;
            
            int columnOrRow = -1;
            
            for( int t=0; t<6; t++ ) {
                for( int p=0; p<2; p++ ) {
                    
                    if( inState->mPlayerMoves[p][t] == -1 &&
                        magicMove->mPlayerMoves[p][t] != -1 ) {
                        
                        // a move made that wasn't present in old state
                        columnOrRow = magicMove->mPlayerMoves[p][t];
                        break;
                        }
                    }
                }
            
            
            delete move;
            return columnOrRow;
            }
        
    };


class MinMaxSquarePlayer : public SquarePlayer {
    public:
        MinMaxSquarePlayer( MinOrMax inPlayerSide ) 
                : mPlayerSide( inPlayerSide ) {}
        
        virtual GameState *pickMove( 
            MagicSquareGameState *inState ) {
            
            return minMaxPickMove( inState, mPlayerSide, -1 );
            }
        
        virtual char needsFlipForP2ToPlayAsP1() {
            return true;
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

        
        virtual char needsFlipForP2ToPlayAsP1() {
            return false;
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
                        if( magicSate->mPlayerMoves[mPlayerNumber][t] 
                            == pick ) {
                            
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



class GreedySquarePlayer : public SquarePlayer {
    public:
        
        GreedySquarePlayer( int inPlayerNumber ) 
                : mPlayerNumber( inPlayerNumber ) {}
        
        virtual char needsFlipForP2ToPlayAsP1() {
            return true;
            }

        virtual GameState *pickMove( 
            MagicSquareGameState *inState ) {

            int numMovesMade = inState->getNumMovesMade();
            
            if( mPlayerNumber == 0 && numMovesMade % 2 == 0 ) {
                // we're p1, and it's our move, moving first
                
                // no score change until they move after us!

                // need to test all possible two-branches 

                GameState *bestMove = NULL;
                int bestScore = INT_MIN;
                
                SimpleVector<GameState *> moves =
                    inState->getPossibleMoves();
                for( int i=0; i<moves.size(); i++ ) {
                    GameState *move = *( moves.getElement( i ) );
                    
                    int worstScore = INT_MAX;
                    
                    // look for worst score p2 can give us
                    // from here
                    SimpleVector<GameState *> p2Moves =
                        move->getPossibleMoves();
                    for( int j=0; j<p2Moves.size(); j++ ) {
                        GameState *p2Move = *( p2Moves.getElement( j ) );
                        
                        int score = p2Move->getScore();
                        
                        if( score < worstScore ) {
                            worstScore = score;
                            }
                        delete p2Move;
                        }
                    
                    if( worstScore > bestScore ) {
                        bestScore = worstScore;
                        if( bestMove != NULL ) {
                            delete bestMove;
                            }
                        bestMove = move;
                        }
                    else {
                        delete move;
                        }
                    }
                return bestMove;
                }
            else if( mPlayerNumber == 1 && numMovesMade % 2 == 1 ) {
                // we're p2, and it's our move, moving after
                // they already picked
                
                // our move will result in a score change
                
                GameState *bestMove = NULL;
                int worstScore = INT_MAX;
                
                SimpleVector<GameState *> moves =
                    inState->getPossibleMoves();
                for( int i=0; i<moves.size(); i++ ) {
                    GameState *move = *( moves.getElement( i ) );
                    
                    int score = move->getScore();
                    
                    if( score < worstScore ) {
                        worstScore = score;
                        if( bestMove != NULL ) {
                            delete bestMove;
                            }
                        bestMove = move;
                        }
                    else {
                        delete move;
                        }
                    }
                
                return bestMove;
                }
            else {
                // asking for our move, but it's not our turn?
                printf( "ERROR:  Greedy player called for move "
                        "but it's not our turn\n" );
                return NULL;
                }
            }
        
    protected:
        int mPlayerNumber;
    };




#include "minorGems/util/random/JenkinsRandomSource.h"

// plays randomly
class RandomSquarePlayer : public SquarePlayer {
    public:
        RandomSquarePlayer( int inSeed ) 
                : mRandSource( inSeed ) {
            }
        
        virtual char needsFlipForP2ToPlayAsP1() {
            return false;
            }

        virtual GameState *pickMove( 
            MagicSquareGameState *inState ) {
            
            SimpleVector<GameState *> moves =
                    inState->getPossibleMoves();

            int movePick = 
                mRandSource.getRandomBoundedInt( 0, moves.size() - 1 );
            

            for( int i=0; i<moves.size(); i++ ) {
                if( i != movePick ) {
                    delete *( moves.getElement(i) );
                    }
                }
            
            return *( moves.getElement(movePick) );
            }
        
    protected:
        JenkinsRandomSource mRandSource;
    };



int main() {

    printf( "Generating square\n" );
    
    int *squareA = generateMagicSquare6( 10 );
    //int *squareA = generateMagicSquare6( 16 );
    
    printf( "SquareA:\n" );
    printSquare( squareA, 6 );
    

    MagicSquareGameState startState( squareA );

    startState.printState();
    
    //GameState *flipped = startState.flipGame();
    

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
    MinMaxSquarePlayer player2( max );

    //HumanPlayer player1( 0 );
    //HumanPlayer player2( 1 );

    //GreedySquarePlayer player1( 0 );
    //GreedySquarePlayer player2( 0 );
    
    //RandomSquarePlayer player1;
    //RandomSquarePlayer player2( 231345 );

    players[0] = &player1;
    players[1] = &player2;


    //int nextPlayer = 0;
    
    GameState *nextState = startState.copy();
    

    while( ! nextState->getGameOver() ) {

        MagicSquareGameState *nextStateMagic = 
            (MagicSquareGameState*)nextState;

        int p1Move = 
            players[0]->pickMoveColumnOrRow( nextStateMagic );
        
        int p2Move;
        if( players[1]->needsFlipForP2ToPlayAsP1() ) {
            
            // flip game so that p2 can play from a blind, p1 perspective
            MagicSquareGameState *nextStateFlipped =
                (MagicSquareGameState*)( nextStateMagic->flipGame() );
        
            p2Move = players[1]->pickMoveColumnOrRow( nextStateFlipped );
            
            delete nextStateFlipped;
            }
        else {
            p2Move = players[1]->pickMoveColumnOrRow( nextStateMagic );
            }
        

        GameState *temp = nextState;

        MagicSquareGameState *temp2 = 
            (MagicSquareGameState*)nextStateMagic->makeMove( 0, p1Move );
        nextState = temp2->makeMove( 1, p2Move );
        

        delete temp;
        delete temp2;

        printf( "Player moves:\n" );
        nextState->printState();
        }
        
    
    printf( "Final minmax score = %d\n", nextState->getScore() );
    
    MagicSquareGameState *magicState = (MagicSquareGameState*)nextState;
    printf( "Final score = P1: %d   P2: %d\n", 
            magicState->getScore( 0 ), magicState->getScore( 1 ) );
    

    
    delete nextState;

    delete [] squareA;
    
    
    return 1;
    }