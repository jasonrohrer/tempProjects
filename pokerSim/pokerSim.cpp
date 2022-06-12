
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "minorGems/crypto/cryptoRandom.h"


typedef enum{ hearts=0, diamonds, clubs, spades, endSuit } suit;

// ace = 14
typedef enum{ jack=11, queen, king, ace, endRank } highRank;
 


typedef struct card {
        suit s;
        int r;
    } card;


char equal( card inA, card inB ) {
    return inA.s == inB.s && inA.r == inB.r;
    }



// unshuffled deck
card *buildFreshDeck() {
    card *deck = new card[52];
    

    int i = 0;

    for( int s = hearts; s < endSuit; s++ ) {
        
        for( int r = 2; r<endRank; r++ ) {
            deck[i].s = (suit)s;
            deck[i].r = r;
            
            i++;
            }
        }
    return deck;
    }



char suitToChar( suit inSuit ) {
    switch( inSuit ) {
        case hearts:
            return 'H';
        case diamonds:
            return 'D';
        case clubs:
            return 'C';
        case spades:
            return 'S';
        default:
            return '_';
        }
    }


char rankToChar( int inRank ) {
    switch( inRank ) {
        case 10:
            return 'T';
        case jack:
            return 'J';
        case queen:
            return 'Q';
        case king:
            return 'K';
        case ace:
            return 'A';
        default:
            return '0' + inRank;
        }
    }




void printDeck( card *inDeck ) {
    for( int i=0; i<52; i++ ) {
        card c = inDeck[i];
        
        printf( "%c%c ", rankToChar( c.r ), suitToChar( c.s ) );
        }
    printf( "\n\n" );
    }




// interface
class BitStream {
    public:
        virtual ~BitStream() {};
        
        // bits as an array of 1 and 0 values, one bit per array index
        virtual void getBits( char *outBitBuffer, int inNumBits ) = 0;
    };



class StdRandBitStream : public BitStream {
    public:
        StdRandBitStream() {
            srand( (unsigned)time(NULL) );
            }
        virtual void getBits( char *outBitBuffer, int inNumBits );
    
    };


void StdRandBitStream::getBits(  char *outBitBuffer, int inNumBits ) {
    for( int i=0; i<inNumBits; i++ ) {
        if( rand() < RAND_MAX / 2 ) {
            outBitBuffer[i] = 0;
            }
        else {
            outBitBuffer[i] = 1;
            }
        }
    }




class CryptoRandBitStream : public BitStream {
    public:
        CryptoRandBitStream() 
                : mBytePoolSize( 3000 ), mNextPoolByte( 0 ),
                  mNextPoolBitPosition( 0 ) {
                    
                    mBytePool = new unsigned char[mBytePoolSize];
                    
                    refreshBytePool();
                    }
        

        ~CryptoRandBitStream() {
            delete [] mBytePool;
            }


        virtual void getBits( char *outBitBuffer, int inNumBits );

    protected:
        unsigned char *mBytePool;
        int mBytePoolSize;
        int mNextPoolByte;
        // position of next unused bit inside the next byte
        int mNextPoolBitPosition;

        void refreshBytePool() {
            char gotBytes = getCryptoRandomBytes( mBytePool, 
                                                  mBytePoolSize );
            
            if( !gotBytes ) {
                printf( "Failed to get crypto random bytes\n" );
                exit( 1 );
                }
            
            mNextPoolByte = 0;
            mNextPoolBitPosition = 0;
            }
      
  
    };



void CryptoRandBitStream::getBits(  char *outBitBuffer, int inNumBits ) {    
    
    for( int i=0; i<inNumBits; i++ ) {
        
        outBitBuffer[i] = 
            ( mBytePool[mNextPoolByte] >> mNextPoolBitPosition ) & 0x01;

        mNextPoolBitPosition ++;
        if( mNextPoolBitPosition > 7 ) {
            mNextPoolBitPosition = 0;
            mNextPoolByte ++;
            if( mNextPoolByte >= mBytePoolSize ) {
                refreshBytePool();
                }
            }
        }
    }





// get random between start and end, inclusive
int getRangedRandom( BitStream *inStream, int inStart, int inEnd ) {
    int max = inEnd - inStart;
    
    int numBits = 1;

    // one beyond was possible witn numBits
    int possibleMaxBound = 2;
    
    while( possibleMaxBound <= max ) {
        numBits ++;
        possibleMaxBound *= 2;
        }

    char bitBuffer[100];
    
    int value = 0;
    char inRange = false;
    
    while( ! inRange ) {
        inStream->getBits( bitBuffer, numBits );
    
        // convert them to an int
        
        value = 0;
        
        for( int i=0; i<numBits; i++ ) {
            value = value | bitBuffer[i] << i;
            }
        if( value <= max ) {
            inRange = true;
            }
        }
    
    
    return value + inStart;
    }



// shuffles deck using a random bit stream
// this is reverse-order modern-method Fisher-Yates
// https://en.wikipedia.org/wiki/Fisher%E2%80%93Yates_shuffle#Modern_method
//
// Note that we only go up to i = 50 (second to last card), because
// the last card card would only be swapped with itself
void shuffleDeck( card *inDeck, BitStream *inStream ) {
    for( int i=0; i<51; i++ ) {
        card temp = inDeck[i];
        
        int swapPos = getRangedRandom( inStream, i, 51 );

        inDeck[i] = inDeck[swapPos];
        
        inDeck[swapPos] = temp;
        }
    }





void flopHittingTest() {

    card *deck = buildFreshDeck();
    
    printDeck( deck );

    
    CryptoRandBitStream bitStream;
    //StdRandBitStream bitStream;


    shuffleDeck( deck, &bitStream );
    
    printDeck( deck );
    

    delete [] deck;


    int total = 5000;

    int numHitFlopOnlyA = 0;
    int numHitFlopOnlyB = 0;
    int numHitFlopBoth = 0;
    int numHitFlopNeither = 0;
    

    for( int t=0; t<total; t++ ) {
        
        card *deck;
        
        deck = buildFreshDeck();
        
        shuffleDeck( deck, &bitStream );


        char bothHavePlayable = false;
        
        while( !bothHavePlayable ) {
            delete [] deck;
            deck = buildFreshDeck();
            
            shuffleDeck( deck, &bitStream );

            if( ( deck[0].r > 10 || deck[1].r > 10 )
                &&
                ( deck[2].r > 10 || deck[3].r > 10 ) ) {
                
                bothHavePlayable = true;
                }
            }
        
        // both have playable hands, look at flop

        int aHit = false;
        int bHit = false;
        
        for( int i=4; i<=6; i++ ) {
            
            if( deck[0].r == deck[i].r ||
                deck[1].r == deck[i].r ) {
                aHit = true;
                }
            if( deck[2].r == deck[i].r ||
                deck[3].r == deck[i].r ) {
                bHit = true;
                }
            }
        
        if( aHit && bHit ) {
            numHitFlopBoth ++;
            }
        else if( aHit ) {
            numHitFlopOnlyA ++;
            }
        else if( bHit ) {
            numHitFlopOnlyB ++;
            }
        else {
            numHitFlopNeither ++;
            }
        
        //total++;

        delete [] deck;
        }
    

    printf( "Total both playable: %d\n"
            "Flop hit neither: %d (%d%%)\n"
            "Flop hit both: %d (%d%%)\n"
            "Flop hit A: %d (%d%%)\n"
            "Flop hit B: %d (%d%%)\n\n"
            "Chance that B hit if A sees flop hit: %d%%\n\n",
            total,
            numHitFlopNeither, (numHitFlopNeither * 100 ) / total,
            numHitFlopBoth, (numHitFlopBoth * 100 ) / total,
            numHitFlopOnlyA, (numHitFlopOnlyA * 100 ) / total,
            numHitFlopOnlyB, (numHitFlopOnlyB * 100 ) / total,
            (numHitFlopBoth * 100 ) / (numHitFlopBoth + numHitFlopOnlyA ) );
            
    

    /*    

    // test code for shuffling cards

    for( int pos=0; pos<52; pos++ ) {
        printf( "Testing pos %d\n", pos );

        card *freshDeck = buildFreshDeck();

        int counts[52];
        for( int i=0; i<52; i++ ) {
            counts[i] = 0;
            }
        
        int trials = 10000;
        
        
        for( int t=0; t<trials; t++ ) {
        
            card *deck = buildFreshDeck();
        
            shuffleDeck( deck, &bitStream );
        
            for( int i=0; i<52; i++ ) {
                if( equal( deck[pos], freshDeck[i] ) ) {
                    counts[i]++;
                    break;
                    }
                }

            delete [] deck;
            }

        for( int i=0; i<52; i++ ) {
            float fraction = (float)counts[i] / (float)trials;
            
            if( fraction < 0.015 || fraction > 0.025 ) {
                
                printf( "Anomaly for card %d at position %d:  %f\n",
                        i, pos, fraction );
                }
            }
        }
    */



    /*
      // test code for random bit streams
    char *buffer = new char[20];
    
    bitStream.getBits( buffer, 20 );
    
    for( int i=0; i<20; i++ ) {
        printf( "%d ", buffer[i] );
        }
    printf( "\n\n" );
    delete [] buffer;
    
    
    for( int max=0; max<20; max++ ) {
        
        for( int i=0; i<20; i++ ) {
            printf( "%d ", getRangedRandom( &bitStream, 0, max ) );
            }
        printf( "\n" );
        for( int i=0; i<20; i++ ) {
            printf( "%d ", getRangedRandom( &bitStream, max/2, max ) );
            }
        printf( "\n\n" );
        }
    
    #define BINS 5

    int counts[BINS];
    
    for( int i=0; i<BINS; i++ ) {
        counts[i] = 0;
        }
    
    for( int i=0; i<10000000; i++ ) {
        counts[ getRangedRandom( &bitStream, 0, BINS-1 ) ] ++;
        }
    
    printf( "Hit counts:\n" );
    for( int i=0; i<BINS; i++ ) {
        printf( "%d ", counts[i] );
        }
    printf( "\n\n" );
    */



    }


char isPremium( card inCA, card inCB ) {
    
    if( inCA.r == inCB.r ) {
        // pocket pair
        return true;
        }
    
    if( inCA.r >= 11 && inCB.r >= 11 ) {
        
        if( inCA.r == 14 || inCB.r == 14 ) {
            // big ace
            return true;
            }
        
        if( ( inCA.r == 13 && inCB.r == 12 )
            ||
            ( inCA.r == 12 && inCB.r == 13 ) ) {
            
            // KQ
            return true;
            }
        }
    
    return false;
    }



void pocketPairTest() {
    
    int currentNoPairRun = 0;
    int longestNoPairRun = 0;

    int runCount = 0;
    int runSum = 0;
    
    int runCounts[100];
    
    for( int r=0; r<100; r++ ) {
        runCounts[r] = 0;
        }
    
    card *deck;
    
    deck = buildFreshDeck();

    CryptoRandBitStream bitStream;
    
    int numHands = 90;
        
    for( int t=0; t<numHands; t++ ) {
        shuffleDeck( deck, &bitStream );
        currentNoPairRun ++;

        if( isPremium( deck[0], deck[1] ) ) {
            runCount ++;
            runSum += currentNoPairRun;
            
            if( currentNoPairRun < 100 ) {
                runCounts[ currentNoPairRun ] ++;
                }
            
            currentNoPairRun = 0;
            }
        else {

            if( currentNoPairRun > longestNoPairRun ) {
                longestNoPairRun = currentNoPairRun;
                }
            }
        }
    
    delete [] deck;

    printf( "%d hands:\n"
            "%d pairs or premium, %f average run between pairs, "
            "%d longest run\n",
            numHands,
            runCount, runSum / (float)runCount, longestNoPairRun );

    /*
    printf( "Histogram:\n" );
    for( int r=0; r<100; r++ ) {
        printf( "%d: ", r );
        
        for( int t = 0; t<runCounts[r]; t++ ) {
            printf( "+" );
            }
        printf( "\n" );
        }
    */
    }




int countPremium( int numHands, CryptoRandBitStream *inBitStream, 
                  card *inDeck ) {
    
    int count = 0;
    
    for( int t=0; t<numHands; t++ ) {
        shuffleDeck( inDeck, inBitStream );
        
        if( isPremium( inDeck[0], inDeck[1] ) ) {
            count++;
            }
        }
    return count;
    }


void premiumsPerVisit() {
    
    int premiumCountBins[100];
    
    for( int r=0; r<100; r++ ) {
        premiumCountBins[r] = 0;
        }
    

    int numVisits = 2000;
    
    card *deck;
    
    deck = buildFreshDeck();

    CryptoRandBitStream bitStream;


    for( int v=0; v<numVisits; v++ ) {
        int numPremimus = countPremium( 90, &bitStream, deck );
    
        premiumCountBins[numPremimus] ++;
        }
    
    delete [] deck;

    printf( "%d visits (90 per visit):\n", numVisits );

    
    printf( "Histogram:\n" );
    for( int r=0; r<100; r++ ) {
        printf( "%2d: ", r );
        
        for( int t = 0; t<premiumCountBins[r]/5; t++ ) {
            printf( "+" );
            }
        printf( "  %d", premiumCountBins[r] );
        printf( "\n" );
        }
    
    }




int main() {
    //pocketPairTest();

    premiumsPerVisit();
    
    return 0;
    }
