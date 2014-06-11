#include "minorGems/ai/gameTree/GameState.h"



// game played on 6x6 magic squares
// player 1 picks row, player 2 picks column
class MagicSquareGameState : public GameState {
    public:
        
        // blank, no init done
        MagicSquareGameState();
        
        // square copied internally
        // inits to no moves made yet
        MagicSquareGameState( int *inStartingState );


        // get separate, positive score for each player
        // (different than min-max score, where players push score positive
        // or negative)
        int getScore( int inPlayerNumber );
        
        // total number of moves made by both players
        int getNumMovesMade();
        
        
        GameState *makeMove( int inPlayerNumber, int inColumnOrRow );

        // flips square and player moves made so far, turning rows into columns
        // p1 becomes p2, etc.
        GameState *flipGame();
        


        // relative state score for purpose of minMax search
        // (player 1 pushes score positively, player 2 negatively)
        virtual int getScore( char inDebug=false );
        

        virtual char getGameOver();

        virtual SimpleVector<GameState *> getPossibleMoves();
        

        virtual GameState *copy();
        
        virtual void copyFrom( GameState *inOther );

        virtual void printState();
        



        int mSquare[6][6];
        
        // -1 if not taken yet, otherwise show row or column that player
        // picked on that move
        int mPlayerMoves[2][6];

        char mScoringFlipped;
    };
