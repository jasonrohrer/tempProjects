#include <stdlib.h>
#include <time.h>
#include <stdio.h>


static float randRange( int inLow, int inHigh ) {
    // code found here:
    // https://c-faq.com/lib/randrange.html
    return (float)( inLow + rand() ) / 
        (float)(RAND_MAX / (inHigh - inLow + 1) + 1);
    }


#define NUM_PLAYERS 500


int main() {
    srand( time( NULL ) );
    
    int startBalance = 1500;

    int wager = 1500;

    int bankrolls[NUM_PLAYERS];
    
    int lowPoint[NUM_PLAYERS];
    int highPoint[NUM_PLAYERS];
    
    char busted[NUM_PLAYERS];

    float winFraction = 90;

    int winAmount = 2;

    int numHands = 100000;


    for( int p=0; p< NUM_PLAYERS; p++ ) {
        bankrolls[p] = startBalance;

        lowPoint[p] = bankrolls[p];
        highPoint[p] = bankrolls[p];
        busted[p] = false;
        }


    for( int i=0; i < numHands; i++ ) {
        
        for( int p=0; p< NUM_PLAYERS; p++ ) {
            
            float roll = randRange( 0, 100 );
            
            bankrolls[p] -= wager;
            
            if( roll < winFraction ) {
                bankrolls[p] += winAmount * wager;
                }
            
            if( bankrolls[p] > highPoint[p] ) {
                highPoint[p] = bankrolls[p];
                }
            if( bankrolls[p] < lowPoint[p] ) {
                lowPoint[p] = bankrolls[p];
                }
            if( bankrolls[p] <= 0 ) {
                busted[p] = true;
                }
            }
        }

    
    int numBusted = 0;
    
    for( int p=0; p< NUM_PLAYERS; p++ ) {
        if( busted[p] ) {
            numBusted ++;
            }
        }
    
    printf( "Starting bankroll $%d, wager $%d, "
            "win fraction %.2f%%, players in hand %d, percent busted %.1f%%\n",
            startBalance, wager, winFraction, winAmount,
            100 * numBusted / (float)NUM_PLAYERS );
    
            
    return 0;
    }
