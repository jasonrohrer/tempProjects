#include "MagicSquareGameStateTwoPick.h"




MagicSquareGameStateTwoPick::MagicSquareGameStateTwoPick() {
    }




MagicSquareGameStateTwoPick::MagicSquareGameStateTwoPick( int *inStartingState ) {
    
    for( int y=0; y<6; y++ ) {
        for( int x=0; x<6; x++ ) {
            mSquare[y][x] = inStartingState[ y * 6 + x ];
            }
        }
    
    for( int p=0; p<2; p++ ) {
        for( int m=0; m<6; m++ ) {
            mPlayerMoves[p][m] = -1;
            }
        }

    mScoringFlipped = false;    
    }



int MagicSquareGameStateTwoPick::getScore( int inPlayerNumber ) {
    int playerScore = 0;
    
    int startScoringTurn = inPlayerNumber;
    
    if( mScoringFlipped ) {
        startScoringTurn = ( startScoringTurn + 1 ) % 2;
        }

    // players score every other turn
    for( int t=startScoringTurn; t<6; t+=2 ) {
        
        int m1 = mPlayerMoves[0][t];
        int m2 = mPlayerMoves[1][t];
        
        
        if( m1 != -1 && m2 != -1 ) {

            playerScore += mSquare[m1][m2];
            }    
        }
    return playerScore;
    }



int MagicSquareGameStateTwoPick::getNumMovesMade() {
    int numMovesMade = 0;
    
    // players score every other turn
    for( int t=0; t<6; t++ ) {
        
        if( mPlayerMoves[0][t] != -1 ) {
            numMovesMade++;
            
            if( mPlayerMoves[1][t] != -1 ) {
                numMovesMade++;
                }
            }    
        }
    return numMovesMade;
    }




GameState *MagicSquareGameStateTwoPick::makeMove( int inPlayerNumber, 
                                           int inColumnOrRow ) {
    MagicSquareGameStateTwoPick *newState = (MagicSquareGameStateTwoPick*)copy();
    
    for( int t=0; t<6; t++ ) {
        if( newState->mPlayerMoves[inPlayerNumber][t] == -1 ) {
            newState->mPlayerMoves[inPlayerNumber][t] = inColumnOrRow;
            break;
            }
        }
    return newState;
    }


GameState *MagicSquareGameStateTwoPick::flipGame() {
    
    MagicSquareGameStateTwoPick *newState = (MagicSquareGameStateTwoPick*)copy();
    
    for( int y=0; y<6; y++ ) {
        for( int x=0; x<6; x++ ) {
            newState->mSquare[y][x] = mSquare[x][y];
            }
        }
    
    for( int p=0; p<2; p++ ) {
        for( int m=0; m<6; m++ ) {
            newState->mPlayerMoves[(p+1) % 2][m] = mPlayerMoves[p][m];
            }
        }

    newState->mScoringFlipped = !mScoringFlipped;
    return newState;
    }





int MagicSquareGameStateTwoPick::getScore( char inDebug ) {
    
    // p1 pushes score up by scoring
    // p2 pushes score down by scoring
    return getScore(0) - getScore(1);
    }

        

char MagicSquareGameStateTwoPick::getGameOver() {
    for( int t=0; t<6; t++ ) {
        
        int m1 = mPlayerMoves[0][t];
        int m2 = mPlayerMoves[1][t];
        
        
        if( m1 == -1 || m2 == -1 ) {
            return false;
            }
        }
    return true;
    }



SimpleVector<GameState *> MagicSquareGameStateTwoPick::getPossibleMoves() {
    
    char rowsLeft[6];
    char colsLeft[6];
    
    for( int b=0; b<6; b++ ) {
        rowsLeft[b] = true;
        colsLeft[b] = true;
        }
    

    int nextPlayer = -1;
    int nextTurn = 0;

    for( int t=0; t<6; t++ ) {
        
        int m1 = mPlayerMoves[0][t];
        int m2 = mPlayerMoves[1][t];
        
        
        if( m1 != -1 ) {
            rowsLeft[m1] = false;
            if( m2 != -1 ) {
                colsLeft[m2] = false;
                }
            else {
                nextPlayer = 1;
                break;
                }
            }
        else {
            nextPlayer = 0;
            break;
            }

        nextTurn++;
        }
    
    SimpleVector<GameState *> result;
    
    if( nextPlayer != -1 ) {
        
        for( int b=0; b<6; b++ ) {
            if( nextPlayer == 0 && rowsLeft[b] 
                ||
                nextPlayer == 1 && colsLeft[b] ) {
                
                for( int bb=0; bb<6; bb++ ) {
                    if( b != bb ) {
                        if( nextPlayer == 0 && rowsLeft[bb] 
                            ||
                            nextPlayer == 1 && colsLeft[bb] ) {
                            MagicSquareGameStateTwoPick *newState = 
                                (MagicSquareGameStateTwoPick*)copy();
                            
                            newState->mPlayerMoves[nextPlayer][nextTurn] = b;
                            newState->mPlayerMoves[nextPlayer][nextTurn+1] = bb;
                            
                            result.push_back( newState );
                            }
                        }
                    }                    
                }
            }
        }
    
    return result;
    }

        

GameState *MagicSquareGameStateTwoPick::copy() {
    MagicSquareGameStateTwoPick *newState = new MagicSquareGameStateTwoPick();
    
    newState->copyFrom( this );

    return newState;
    }


        
void MagicSquareGameStateTwoPick::copyFrom( GameState *inOther ) {
    MagicSquareGameStateTwoPick *other = (MagicSquareGameStateTwoPick*)inOther;
    
    for( int y=0; y<6; y++ ) {
        memcpy( mSquare[y], other->mSquare[y], 6 * sizeof( int ) );
        }
    
    for( int p=0; p<2; p++ ) {
        memcpy( mPlayerMoves[p], other->mPlayerMoves[p], 6 * sizeof( int ) );
        }
    
    mScoringFlipped = other->mScoringFlipped;
    }



void MagicSquareGameStateTwoPick::printState() {
    
    printf(     "Player moves:  1    2\n" );
    printf(     "               ------\n" );
    
    for( int t=0; t<6; t++ ) {
        printf( "              %2d   %2d\n", 
                mPlayerMoves[0][t],
                mPlayerMoves[1][t] );
        }
    
    }

