/*
To compile:

git clone https://github.com/HenryRLee/PokerHandEvaluator.git
cd PokerHandEvaluator/cpp
make libphevalplo4.a libphevalplo5.a libphevalplo6.a

cp oneCard.cpp PokerHandEvaluator/cpp/examples
cd examples

g++ -g -I../include -o oneCard oneCard.cpp ../libpheval.a ../libphevalplo4.a ../libphevalplo5.a ../libphevalplo6.a
*/


#include <iostream>
#include <phevaluator/phevaluator.h>
#include <phevaluator/rank.h>
#include <cassert>
#include <string.h>



static const char allRanks[13] = { '2', '3', '4', '5', '6', '7', '8', '9',
                                   'T', 'J', 'Q', 'K', 'A' };


// const strings for all possible cards in hand
// used when reading CardSet from file to avoid allocating memory
static const char *allCards[52] = {
    "Ad","2d","3d","4d","5d","6d","7d","8d","9d","Td","Jd","Qd","Kd",
    "As","2s","3s","4s","5s","6s","7s","8s","9s","Ts","Js","Qs","Ks",
    "Ac","2c","3c","4c","5c","6c","7c","8c","9c","Tc","Jc","Qc","Kc",
    "Ah","2h","3h","4h","5h","6h","7h","8h","9h","Th","Jh","Qh","Kh" };




// a full or partial deck
typedef struct Deck {
        const char *cards[52];
        int numCards;
    } Deck;



// represents either a hand or a board
// for PLO, this is 6 cards max (hand can be 4, 5, or 6, 
// board can be 3, 4, or 5)
typedef struct CardSet {
        const char *cards[6];
        int numCards;
    } CardSet;
        



CardSet makeHand( const char *inA,
                  const char *inB,
                  const char *inC,
                  const char *inD ) {
    CardSet s;
    s.numCards = 4;
    s.cards[0] = inA;
    s.cards[1] = inB;
    s.cards[2] = inC;
    s.cards[3] = inD;

    return s;
    }


CardSet makeHand( const char *inA ) {
    CardSet s;
    s.numCards = 1;
    s.cards[0] = inA;

    return s;
    }



CardSet makeBoard( const char *inFlopA,
                   const char *inFlopB,
                   const char *inFlopC,
                   const char *inTurn = "",
                   const char *inRiver = "" ) {
    CardSet s;
    s.numCards = 3;
    s.cards[0] = inFlopA;
    s.cards[1] = inFlopB;
    s.cards[2] = inFlopC;

    if( strcmp( inTurn, "" ) != 0 ) {
        s.numCards = 4;
        s.cards[3] = inTurn;

        if( strcmp( inRiver, "" ) != 0 ) {
            s.numCards = 5;
            s.cards[4] = inRiver;
            }
        }
    
    return s;
    }



CardSet copy( CardSet *inSet ) {
    CardSet newSet;
    newSet.numCards = inSet->numCards;
    for( int i=0; i< inSet->numCards; i++ ) {
        newSet.cards[i] = inSet->cards[i];
        }
    return newSet;
    }



void boardAdd( CardSet *inBoard, const char *inNewCard ) {
    if( inBoard->numCards < 5 ) {

        inBoard->cards[ inBoard->numCards ] = inNewCard;

        inBoard->numCards ++;
        }
    }

    



static void setupFreshDeck( Deck *inDeck ) {

    for( int i=0; i<52; i++ ) {
        inDeck->cards[i] = allCards[i];
        }
    
    inDeck->numCards = 52;
    }



static int randRange( int inLow, int inHigh ) {
    // code found here:
    // https://c-faq.com/lib/randrange.html
    return inLow + rand() / (RAND_MAX / (inHigh - inLow + 1) + 1);
    }



static void shuffle( Deck *inDeck ) {
    // Fisher–Yates shuffle
        
    for( int i = inDeck->numCards - 1; i>=1; i-- ) {
        
        int j = randRange( 0, i );
        const char *temp = inDeck->cards[j];
        inDeck->cards[j] = inDeck->cards[i];
        inDeck->cards[i] = temp;
        }
    }



static void print( Deck *inDeck ) {
    for( int i=0; i < inDeck->numCards; i++ )
        printf( "%d:  %s\n", i+1, inDeck->cards[i] );
    }



static void print( CardSet *inCards ) {
    for( int i=0; i < inCards->numCards; i++ )
        printf( "%2s ", inCards->cards[i] );
    }



// can handle "?" for unknown cards that shouldn't be removed
// returns true if remove, false if removal fails (and card is not ?)
static char remove( Deck *inDeck, const char *inCard ) {
    int findIndex = -1;
    for( int i=0; i < inDeck->numCards; i++ ) {
        if( strcmp( inDeck->cards[i], inCard ) == 0 ) {
            findIndex = i;
            break;
            }
        }

    if( findIndex != -1 ) {
        for( int i=findIndex; i< inDeck->numCards -1; i++ ) {
            inDeck->cards[i] = inDeck->cards[ i+1 ];
            }
        inDeck->numCards--;
        return true;
        }

    if( strcmp( inCard, "?" ) != 0 ) {
        return false;
        }
    else {
        return true;
        }
    }



// can handle "?" for unknown cards that shouldn't be removed
// returns true if all present in deck and removed (or ?)
// returns false if non-? card is not in deck
static char remove( Deck *inDeck, CardSet *inCards ) {
    char allRemoved = true;
    for( int i=0; i<inCards->numCards; i++ ) {
        allRemoved &= remove( inDeck, inCards->cards[i] );
        }
    return allRemoved;
    }



static Deck copy( Deck *inDeck ) {
    Deck newDeck;
    
    newDeck.numCards = inDeck->numCards;

    for( int i=0; i < newDeck.numCards; i++ ) {
        newDeck.cards[i] = inDeck->cards[i];
        }
    
    return newDeck;
    }



// -1 for A, 1 for B
// 0 for tie
int simWinner( CardSet *inHandA, CardSet *inHandB ) {
    Deck d;
    setupFreshDeck( &d );
    remove( &d, inHandA );
    remove( &d, inHandB );
    
    shuffle( &d );
    
    phevaluator::Rank rankA = 
        phevaluator::EvaluateCards(
            inHandA->cards[0],
            d->cards[0],
            d->cards[1],
            d->cards[2],
            d->cards[3],
            d->cards[4] );

    phevaluator::Rank rankB = 
        phevaluator::EvaluateCards(
            inHandB->cards[0],
            d->cards[0],
            d->cards[1],
            d->cards[2],
            d->cards[3],
            d->cards[4] );
    

    if( rankA.value() < rankB.value() ) {
        return -1;
        }
    else if( rankA.value() > rankB.value() ) {
        return 1;
        }
    else {
        return 0;
        }
    }





int main() {
    
    CardSet handA = makeHand( "Ac" );
    CardSet handB = makeHand( "Kh" );
    

    int numSims = 10000;

    int aWinCount = 0;
    
    int tieCount = 0;

    for( int i=0; i<numSims; i++ ) {
        
        int result = simWinner( &handA, &handB );
        
        if( result < 0 ) {
            aWinCount ++;
            }
        else if( result == 0 ) {
            tieCount++;
            }
        }
    
    int bWinCount = numSims - aWinCount - tieCount;
    
    printf( "Hand " );
    print( handA );
    
    printf( "wins %.1%%\n", 100 * (float)aWinCount / (float)numSims );
    
    printf( "Hand " );
    print( handB );
    
    printf( "wins %.1%%\n", 100 * (float)bWinCount / (float)numSims );
    
    printf( "Tie %.1%%\n",  100 * (float)tieCount / (float)numSims );
    }
