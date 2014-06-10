#include "MagicSquareGameState.h"




MagicSquareGameState::MagicSquareGameState() {
    }




MagicSquareGameState::MagicSquareGameState( int *inStartingState ) {
    
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
    
    }



int MagicSquareGameState::getScore( int inPlayerNumber ) {
    int playerScore = 0;
    
    // players score every other turn
    for( int t=inPlayerNumber; t<6; t+=2 ) {
        
        int m1 = mPlayerMoves[0][t];
        int m2 = mPlayerMoves[1][t];
        
        
        if( m1 != -1 && m2 != -1 ) {

            playerScore += mSquare[m1][m2];
            }    
        }
    return playerScore;
    }




int MagicSquareGameState::getScore( char inDebug ) {
    
    // p1 pushes score up by scoring
    // p2 pushes score down by scoring
    return getScore(0) - getScore(1);
    }

        

char MagicSquareGameState::getGameOver() {
    for( int t=0; t<6; t++ ) {
        
        int m1 = mPlayerMoves[0][t];
        int m2 = mPlayerMoves[1][t];
        
        
        if( m1 == -1 || m2 == -1 ) {
            return false;
            }
        }
    return true;
    }



SimpleVector<GameState *> MagicSquareGameState::getPossibleMoves() {
    
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
                
                MagicSquareGameState *newState = (MagicSquareGameState*)copy();
                    
                newState->mPlayerMoves[nextPlayer][nextTurn] = b;
                
                result.push_back( newState );
                }
            }
        }
    
    return result;
    }

        

GameState *MagicSquareGameState::copy() {
    MagicSquareGameState *newState = new MagicSquareGameState();
    
    newState->copyFrom( this );

    return newState;
    }


        
void MagicSquareGameState::copyFrom( GameState *inOther ) {
    MagicSquareGameState *other = (MagicSquareGameState*)inOther;
    
    for( int y=0; y<6; y++ ) {
        memcpy( mSquare[y], other->mSquare[y], 6 * sizeof( int ) );
        }
    
    for( int p=0; p<2; p++ ) {
        memcpy( mPlayerMoves[p], other->mPlayerMoves[p], 6 * sizeof( int ) );
        }
    }



void MagicSquareGameState::printState() {
    
    printf(     "Player moves:  1    2\n" );
    
    for( int t=0; t<6; t++ ) {
        printf( "              %2d   %2d\n", 
                mPlayerMoves[0][t],
                mPlayerMoves[1][t] );
        }
    
    }

