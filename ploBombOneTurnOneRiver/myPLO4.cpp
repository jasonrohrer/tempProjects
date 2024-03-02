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
        printf( "%2s ", inCards->cards[i] );
    }



// can handle "?" for unknown cards that shouldn't be removed
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


// can handle "?" for unknown cards that shouldn't be removed
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



typedef struct Result {
        char winners[9];
    } Result;
        

// simulates one run of double-board bomb pot, one turn, one river
// inHands CardSets can contain ?,?,?,? for unknown hands
// inHands can be NULL for empty seats (or folded players)
// returns map of winners
Result simWinner( CardSet *inFlopTop, CardSet *inFlopBot,
                  CardSet *inHands[9] ) {
    Deck d;
    setupFreshDeck( &d );
    remove( &d, inFlopTop );
    remove( &d, inFlopBot );

    // we're going to modify unknown hands by drawing cards
    // do that in copies of the hands
    CardSet workingHands[9];

    CardSet *workingHandsPointers[9];
    
    for( int i=0; i<9; i++ ) {
        if( inHands[i] != NULL ) {
            
            remove( &d, inHands[i] );

            workingHands[i] = copy( inHands[i] );
            workingHandsPointers[i] = &( workingHands[i] );
            }
        else {
            workingHandsPointers[i] = NULL;
            }
        }    
    
    shuffle( &d );
    
    // now draw two cards, for turn and river
    const char *turn = d.cards[0];
    const char *river = d.cards[1];

    // now draw cards to populate player's hands that have unknown cards
    int nextCard = 2;

    for( int i=0; i<9; i++ ) {
        if( workingHandsPointers[i] != NULL ) {
            for( int c=0; c<workingHandsPointers[i]->numCards; c++ ) {
                if( strcmp( workingHandsPointers[i]->cards[c], "?" ) == 0 ) {
                    workingHandsPointers[i]->cards[c] = d.cards[ nextCard ];
                    nextCard++;
                    }
                }
            }
        }

    phevaluator::Rank handRanksTop[9];
    phevaluator::Rank handRanksBottom[9];

    for( int i=0; i<9; i++ ) {
        if( workingHandsPointers[i] != NULL ) {
            handRanksTop[i] =
                getRank( inFlopTop, turn, river, workingHandsPointers[i] );
            handRanksBottom[i] =
                getRank( inFlopBot, turn, river, workingHandsPointers[i] );
            }
        }


    
    int bestHandValues[9];

    for( int i=0; i<9; i++ ) {
        if( workingHandsPointers[i] != NULL ) {
            bestHandValues[i] = handRanksTop[i].value();
            int botVal = handRanksBottom[i].value();

            if( botVal < bestHandValues[i] ) {
                bestHandValues[i] = botVal;
                }
            }
        }
    

    
    Result r;

    // first, find best overall
    int bestValue = 999999999;
    int bestValueIndex = -1;

    for( int i=0; i<9; i++ ) {
        r.winners[i] = false;
        if( workingHandsPointers[i] != NULL ) {
            if( bestHandValues[i] < bestValue ) {
                bestValue = bestHandValues[i];
                bestValueIndex = i;
                }
            }
        }

    assert( bestValueIndex != -1 );
    
    r.winners[bestValueIndex] = true;

    // look for ties with winner
    for( int i=0; i<9; i++ ) {
        if( workingHandsPointers[i] != NULL ) {
            if( bestHandValues[i] == bestValue ) {
                r.winners[i] = true;
                }
            }
        }

    /*
    printf( "\n\nBoard = \n" );
    print( inFlopTop );

    printf( "\n         " );
    printf( "%s ", turn );
    printf( "%s\n", river );

    print( inFlopBot );
    printf( "\n" );


    printf( "\nHands:\n" );
    for( int i=0; i<9; i++ ) {
        if( workingHandsPointers[i] != NULL ) {
            print( workingHandsPointers[i] );

            if( r.winners[i] ) {
                printf( "   WINNER" );
                }
            printf( "\n" );
            }
        }

    printf( "\n\n" );
    */
    
    freeDeck( &d );
    
    
    return r;
    }



void usage() {
    printf( "Usage:\n\n" );
    printf( "myPLO4 handFile.txt\n\n" );
    exit( 1 );
    }



// const strings for all possible cards in hand
// used when reading CardSet from file to avoid allocating memory
static const char *allCards[52] = {
    "Ad","2d","3d","4d","5d","6d","7d","8d","9d","Td","Jd","Qd","Kd",
    "As","2s","3s","4s","5s","6s","7s","8s","9s","Ts","Js","Qs","Ks",
    "Ac","2c","3c","4c","5c","6c","7c","8c","9c","Tc","Jc","Qc","Kc",
    "Ah","2h","3h","4h","5h","6h","7h","8h","9h","Th","Jh","Qh","Kh" };


const char *getConstCard( char *inCard ) {
    for( int i=0; i<52; i++ ) {
        if( strcmp( allCards[i], inCard ) == 0 ) {
            return allCards[i];
            }
        }
    // no match, return blank
    return "?";
    }



CardSet readHandFromFile( FILE *inFile, char *outSuccess ) {
    *outSuccess = false;
    
    CardSet c;
    c.numCards = 4;
    for( int i=0; i<4; i++ ) {
        c.cards[i] = "?";
        }
    
    char cardA[3];
    char cardB[3];
    char cardC[3];
    char cardD[3];

    char *cards[4];
    cards[0] = cardA;
    cards[1] = cardB;
    cards[2] = cardC;
    cards[3] = cardD;
    
    
    int numRead = fscanf( inFile,
                          "hand %2s %2s %2s %2s\n",
                          cardA, cardB, cardC, cardD );
    if( numRead == 4 ) {
        *outSuccess = true;
        for( int i=0; i<4; i++ ) {
            c.cards[i] = getConstCard( cards[i] );
            }
        /*
        printf( "Read hand from file:  " );
        print( &c );
        printf( "\n" );
        */
        }
    else {
        /*
          printf( "Failed to read hand from file (numRead = %d)\n", numRead );
        */
        }

    
    return c;
    }




CardSet readFlopFromFile( FILE *inFile, char *outSuccess ) {
    *outSuccess = false;
    
    CardSet c;
    c.numCards = 3;
    for( int i=0; i<3; i++ ) {
        c.cards[i] = "?";
        }
    
    char cardA[3];
    char cardB[3];
    char cardC[3];

    char *cards[3];
    cards[0] = cardA;
    cards[1] = cardB;
    cards[2] = cardC;
    
    
    int numRead = fscanf( inFile,
                          "flop %2s %2s %2s\n",
                          cardA, cardB, cardC );
    if( numRead == 3 ) {
        *outSuccess = true;
        for( int i=0; i<3; i++ ) {
            c.cards[i] = getConstCard( cards[i] );
            }
        /*
        printf( "Read flop from file:  " );
        print( &c );
        printf( "\n" );
        */
        }
    else {
        /*
        printf( "Failed to read flop from file (numRead = %d)\n", numRead );
        */
        }

    
    return c;
    }



int main( int inNumArgs, const char **inArgs ) {
    
    srand( time( NULL ) );

    if( inNumArgs != 2 ) {
        usage();
        }

    FILE *f = fopen( inArgs[1], "r" );

    if( f == NULL ) {
        usage();
        }

    CardSet hands[9];
    
    CardSet *handPointers[9];

    for( int i=0; i<9; i++ ) {
        char success;

        hands[i] = readHandFromFile( f, &success );

        if( success ) {
            handPointers[i] = &( hands[i] );
            }
        else {
            handPointers[i] = NULL;
            }
        }
    

    char success = false;
    CardSet flopTop = readFlopFromFile( f, &success );

    if( ! success ) {
        printf( "Failed to read top flop from file after hands\n" );
        usage();
        }
    
    
    CardSet flopBot = readFlopFromFile( f, &success );

    if( ! success ) {
        printf( "Failed to read bottom flop from file after hands\n" );
        usage();
        }
    
    int numRuns = 0;
    int winCount[9] = { 0,0,0,0,0,0,0,0,0 };
    int tieCount[9] = { 0,0,0,0,0,0,0,0,0 };
    
        
    
    for( int i=0; i<20000; i++ ) {
        Result r = simWinner( &flopTop, &flopBot, handPointers );

        for( int p=0; p<9; p++ ) {
            if( r.winners[p] ) {
                char tie = false;

                for( int o=0; o<9; o++ ) {
                    if( o != p && r.winners[o] ) {
                        tie = true;
                        }
                    }
                if( tie ) {
                    tieCount[p]++;
                    }
                else {
                    winCount[p]++;
                    }
                }
            }
        
        numRuns++;
        }

    printf( "Top flop:     " );
    print( &flopTop );

    printf( "\nBottom flop:  " );
    print( &flopBot );

    printf( "\n\n" );
    
    
    for( int p=0; p<9; p++ ) {
        if( handPointers[p] == NULL ) {
            printf( "Seat %d  empty\n", p+1 );
            }
        else {
            printf( "Seat %d has  ", p + 1 );
            print( handPointers[p] );
            
            printf( "  wins: %0.1f%%  ties: %0.1f%%\n",
                    100 * (float) winCount[p] / (float) numRuns,
                    100 * (float) tieCount[p] / (float) numRuns );
            }
        }
    return 0;
    }
