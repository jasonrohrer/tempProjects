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
    };
