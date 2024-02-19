/*
To compile:

git clone https://github.com/HenryRLee/PokerHandEvaluator.git
cd PokerHandEvaluator/cpp
make libphevalplo4.a

cp myPLO4.cpp PokerHandEvaluator/cpp/examples
cd examples

g++ -g -I../include -o myPLO4 myPLO4.cpp ../libphevalplo4.a
*/


#include <iostream>
#include <phevaluator/phevaluator.h>
#include <phevaluator/rank.h>
#include <cassert>
#include <string.h>

static const char suits[] = { 'd', 's', 'c', 'h' };

static const char ranks[] =
{ 'A','2','3','4','5','6','7','8','9','T','J','Q','K' };


// a full or partial deck
typedef struct Deck {
        char *cards[52];
        int numCards;
    } Deck;



// represents either a hand or a board
// for PLO, this is 5 cards max (had can be 4, board can be 3, 4, or 5)
typedef struct CardSet {
        const char *cards[5];
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



void boardAdd( CardSet *inBoard, const char *inNewCard ) {
    if( inBoard->numCards < 5 ) {

        inBoard->cards[ inBoard->numCards ] = inNewCard;

        inBoard->numCards ++;
        }
    }

    



static void setupFreshDeck( Deck *inDeck ) {

    for( int s=0; s<4; s++ ) {
        for( int r=0; r<13; r++ ) {
            int i = r * 4 + s;
            inDeck->cards[ i ] = new char[3];
            inDeck->cards[ i ][2] = '\0';

            inDeck->cards[ i ][0] = ranks[r];
            inDeck->cards[ i ][1] = suits[s];
            }
        }
    inDeck->numCards = 52;
    }



static void freeDeck( Deck *inDeck ) {
    for( int i=0; i<52; i++ ) {
        delete [] inDeck->cards[i];
        }
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
        char *temp = inDeck->cards[j];
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
        printf( "%s ", inCards->cards[i] );
    }



static void remove( Deck *inDeck, const char *inCard ) {
    int findIndex = -1;
    for( int i=0; i < inDeck->numCards; i++ ) {
        if( strcmp( inDeck->cards[i], inCard ) == 0 ) {
            findIndex = i;
            break;
            }
        }

    if( findIndex != -1 ) {
        for( int i=findIndex; i< inDeck->numCards -1; i++ ) {
            memcpy( inDeck->cards[i], inDeck->cards[ i+1 ], 3 );
            }
        inDeck->numCards--;
        }
    }


static void remove( Deck *inDeck, CardSet *inCards ) {
    for( int i=0; i<inCards->numCards; i++ ) {
        remove( inDeck, inCards->cards[i] );
        }
    }



static Deck copy( Deck *inDeck ) {
    Deck newDeck;

    for( int i=0; i < 52; i++ ) {
        newDeck.cards[i] = new char[3];
        }
    
    for( int i=0; i < inDeck->numCards; i++ ) {
        memcpy( newDeck.cards[i], inDeck->cards[i], 3 );
        }

    newDeck.numCards = inDeck->numCards;
    
    return newDeck;
    }



phevaluator::Rank getRank( CardSet *inFlop,
                           const char *inTurn, const char *inRiver,
                           CardSet *inHand ) {
    phevaluator::Rank rank =
        phevaluator::EvaluatePlo4Cards( inFlop->cards[0],
                                        inFlop->cards[1],
                                        inFlop->cards[2],
                                        inTurn,
                                        inRiver,
                                        inHand->cards[0],
                                        inHand->cards[1],
                                        inHand->cards[2],
                                        inHand->cards[3] );
    /*
    printf( "Board: %s %s %s %s %s, hand: %s %s %s %s,  ",
            inFlop->cards[0],
            inFlop->cards[1],
            inFlop->cards[2],
            inTurn,
            inRiver,
            inHand->cards[0],
            inHand->cards[1],
            inHand->cards[2],
            inHand->cards[3] );
    
    printf( "Rank = %d\n", rank.value() );
    */
    return rank;
    }



// simulates one run of double-board bomb pot, one turn, one river
// returns -1 if A wins, 0 if tie, +1 if B wins
int simWinner( CardSet *inFlopTop, CardSet *inFlopBot,
               CardSet *inHandA, CardSet *inHandB ) {
    Deck d;
    setupFreshDeck( &d );
    remove( &d, inFlopTop );
    remove( &d, inFlopBot );
    remove( &d, inHandA );
    remove( &d, inHandB );

    shuffle( &d );

    // now draw two cards, for turn and river
    const char *turn = d.cards[0];
    const char *river = d.cards[1];

    // player A on top board
    phevaluator::Rank rankTopA = getRank( inFlopTop, turn, river, inHandA );

    // player B on top board
    phevaluator::Rank rankTopB = getRank( inFlopTop, turn, river, inHandB );


    // player A on bottom board
    phevaluator::Rank rankBotA = getRank( inFlopBot, turn, river, inHandA );

    // player B on bottom board
    phevaluator::Rank rankBotB = getRank( inFlopBot, turn, river, inHandB );

    /*
    printf( "Board = \n" );
    print( inFlopTop );

    printf( "\n         " );
    printf( "%s ", turn );
    printf( "%s\n", river );

    print( inFlopBot );
    printf( "\n" );
    */

    freeDeck( &d );
    

    int bestA = rankTopA.value();

    if( rankBotA.value() < bestA ) {
        bestA = rankBotA.value();
        }

    int bestB = rankTopB.value();

    if( rankBotB.value() < bestB ) {
        bestB = rankBotB.value();
        }


    if( bestA < bestB ) {
        return -1;
        }
    if( bestA > bestB ) {
        return 1;
        }
    
    // tie
    return 0;
    }





int main() {
    srand( time( NULL ) );

    CardSet handA = makeHand( "Qd", "2d", "5c", "9h" );
    CardSet handB = makeHand( "Kc", "Jc", "As", "Qs" );

    CardSet flopTop = makeBoard( "Ks", "Js", "3c" );
    CardSet flopBot = makeBoard( "Ad", "Kd", "7d" );


    int numRuns = 0;
    int winA = 0;
    int winB = 0;
    int tie = 0;

    for( int i=0; i<10000; i++ ) {
        int res = simWinner( &flopTop, &flopBot, &handA, &handB );

        if( res == -1 ) {
            winA ++;
            }
        else if( res == 1 ) {
            winB ++;
            }
        else {
            tie ++;
            }
        
        numRuns++;
        }

    printf( "A: %0.2f   B: %0.2f   Tie: %0.2f\n",
            (float) winA / (float) numRuns,
            (float) winB / (float) numRuns,
            (float) tie / (float) numRuns );
    
    
    return 0;
    }
