

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
        
        
        virtual const char *getName() = 0;
        

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
        
        virtual const char *getName() {
            return "MinMax";
            }

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
        
        virtual const char *getName() {
            return "Human";
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
        
        virtual const char *getName() {
            return "Greedy";
            }


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
        
        virtual const char *getName() {
            return "Random";
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



// result:  1, p1 win, 0 tie, -1 p2 win
int runGame( int *inSquare, 
             SquarePlayer *inPlayer1, SquarePlayer *inPlayer2 ) {
    
    MagicSquareGameState *nextState = new MagicSquareGameState( inSquare );

    while( ! nextState->getGameOver() ) {

        int p1Move = 
            inPlayer1->pickMoveColumnOrRow( nextState );
        
        int p2Move;
        if( inPlayer2->needsFlipForP2ToPlayAsP1() ) {
            
            // flip game so that p2 can play from a blind, p1 perspective
            MagicSquareGameState *nextStateFlipped =
                (MagicSquareGameState*)( nextState->flipGame() );
        
            p2Move = inPlayer1->pickMoveColumnOrRow( nextStateFlipped );
            
            delete nextStateFlipped;
            }
        else {
            p2Move = inPlayer2->pickMoveColumnOrRow( nextState );
            }
        

        MagicSquareGameState *temp = nextState;

        MagicSquareGameState *temp2 = 
            (MagicSquareGameState*)( nextState->makeMove( 0, p1Move ) );
        nextState = (MagicSquareGameState*)( temp2->makeMove( 1, p2Move ) );
        

        delete temp;
        delete temp2;

        //printf( "Player moves:\n" );
        //nextState->printState();
        }

    int minMaxScore = nextState->getScore();
    
    int result = 0;
    if( minMaxScore > 0 ) {
        result = 1;
        }
    else if( minMaxScore < 0 ) {
        result = -1;
        }
    // else tie

    delete nextState;
    return result;
    }





int main() {

    printf( "Generating square\n" );
    
    int *squareA = generateMagicSquare6( 10 );
    //int *squareA = generateMagicSquare6( 19 );
    
    //printf( "SquareA:\n" );
    //printSquare( squareA, 6 );
    

    MagicSquareGameState startState( squareA );

    //startState.printState();
    
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
    

    // we're going to flip the game state for p2 before asking for its moves
    // so make it a max player also
    MinMaxSquarePlayer minMaxPlayer1( max );
    MinMaxSquarePlayer minMaxPlayer2( max );

    // Human players don't need to be flipped before getting turn-blind moves
    // from them
    //HumanPlayer player1( 0 );
    //HumanPlayer player2( 1 );


    // we're going to flip the game state for p2 before asking for its moves
    // so make it a 0 player also
    GreedySquarePlayer greedyPlayer1( 0 );
    GreedySquarePlayer greedyPlayer2( 0 );
    
    RandomSquarePlayer randomPlayer1( 231345 );
    RandomSquarePlayer randomPlayer2( 231345 );



    //runGame( squareA, &minMaxPlayer1, &minMaxPlayer2 );
    

    players[0] = &minMaxPlayer1;
    players[1] = &minMaxPlayer2;
    

    SquarePlayer *player1[3] = 
        { &minMaxPlayer1, &greedyPlayer1, &randomPlayer1 };

    SquarePlayer *player2[3] = 
        { &minMaxPlayer2, &greedyPlayer2, &randomPlayer2 };
    

    // for all combinations of p1 and p2
    // third index is p1win, p2win, tie
    int winAndTieCounts[3][3][3];

    for( int p1=0; p1<3; p1++ ) {
        for( int p2=0; p2<3; p2++ ) {
            for( int w=0; w<3; w++ ) {
                winAndTieCounts[p1][p2][w] = 0;
                }
            }
        }
    

    int seed = 49;
    
    for( int r=0; r<2; r++ ) {
        printf( "Round %d\n", r );
        seed ++;
        
        int *square = generateMagicSquare6( seed );

        printf( "Square:\n" );
        printSquare( square, 6 );
    

        for( int p1=0; p1<3; p1++ ) {
            for( int p2=0; p2<3; p2++ ) {
                
                int result = runGame( square, player1[p1], player2[p2] );
                
                if( result == 1 ) {
                    winAndTieCounts[p1][p2][0]++;
                    }
                else if( result == -1 ) {
                    winAndTieCounts[p1][p2][1]++;
                    }
                else {
                    winAndTieCounts[p1][p2][2]++;
                    }
                }
            }
        delete [] square;
        }
    
    printf( "Results:\n" );
    
    for( int p1=0; p1<3; p1++ ) {
        for( int p2=0; p2<3; p2++ ) {
            printf( "p1:%s   p2:%s   %d p1 wins, %d p2 wins, %d ties\n",
                    player1[p1]->getName(),
                    player2[p2]->getName(),                    
                    winAndTieCounts[p1][p2][0],
                    winAndTieCounts[p1][p2][1],
                    winAndTieCounts[p1][p2][2] );
            }
        }
    return 0;
    


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
