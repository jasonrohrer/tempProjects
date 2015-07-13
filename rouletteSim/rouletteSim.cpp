#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "minorGems/crypto/cryptoRandom.h"



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




// returns 0 (red), 1 (black), or 2 (green)
int spinWheel( BitStream *inStream ) {
    int spin = getRangedRandom( inStream, 1, 38 );
    
    if( spin > 36 ) {
        return 2;
        }
    
    if( spin % 2 == 0 ) {
        return 0;
        }
    return 1;
    }



int start = 1;
int limit = 100000;

int startBankroll = 1000000;

int startBet = 2;
double increaseFactor = 2;

int main() {
    
    //CryptoRandBitStream bitStream;
    StdRandBitStream bitStream;

    for( int a=0; a<100; a++ ) {
        int bankroll = startBankroll;
        int current = startBet;
        

        for( int i=0; i<1000 || current > 1; i++ ) {
            int spin = spinWheel( &bitStream );
            

            if( spin == 0 ) {
                //printf( "  Won %d\n", current );
                bankroll += current;
                current = start;
                }
            else {
                //printf( "  lost %d\n", current );
                bankroll -= current;
                current *= increaseFactor;
                if( current > limit ) {
                    // hold at max
                    current = limit;
                    }

                if( bankroll < 0 ) {
                    printf( "busted\n" );
                    break;
                    }
                }
            }
        
        printf( "Ending bankroll = %d\n", bankroll );
        }
    
    return 0;
    }

