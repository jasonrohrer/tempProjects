/*
To compile:

git clone https://github.com/HenryRLee/PokerHandEvaluator.git
cd PokerHandEvaluator/cpp
make libphevalplo4.a libphevalplo5.a libphevalplo6.a

cp myPLO4.cpp PokerHandEvaluator/cpp/examples
cd examples

g++ -g -I../include -o myPLO4 myPLO4.cpp ../libpheval.a ../libphevalplo4.a ../libphevalplo5.a ../libphevalplo6.a
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



phevaluator::Rank getRank( CardSet *inFlop,
                           const char *inTurn, const char *inRiver,
                           CardSet *inHand ) {
    phevaluator::Rank rank;
    
    if( inHand->numCards == 4 ) {
        rank =
            phevaluator::EvaluatePlo4Cards( inFlop->cards[0],
                                            inFlop->cards[1],
                                            inFlop->cards[2],
                                            inTurn,
                                            inRiver,
                                            inHand->cards[0],
                                            inHand->cards[1],
                                            inHand->cards[2],
                                            inHand->cards[3] );
        }
    else if( inHand->numCards == 5 ) {
        rank =
            phevaluator::EvaluatePlo5Cards( inFlop->cards[0],
                                            inFlop->cards[1],
                                            inFlop->cards[2],
                                            inTurn,
                                            inRiver,
                                            inHand->cards[0],
                                            inHand->cards[1],
                                            inHand->cards[2],
                                            inHand->cards[3],
                                            inHand->cards[4] );
        }
    else if( inHand->numCards == 6 ) {
        rank =
            phevaluator::EvaluatePlo6Cards( inFlop->cards[0],
                                            inFlop->cards[1],
                                            inFlop->cards[2],
                                            inTurn,
                                            inRiver,
                                            inHand->cards[0],
                                            inHand->cards[1],
                                            inHand->cards[2],
                                            inHand->cards[3],
                                            inHand->cards[4],
                                            inHand->cards[5] );
        }
    
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
// inHands can be 4-card, 5-card, or 6-card hands
// inHands can be NULL for empty seats (or folded players)
// returns map of winners
Result simWinner( CardSet *inFlopTop, CardSet *inFlopBot,
                  CardSet *inHands[9], char inPrintResult = false ) {
    Deck d;
    setupFreshDeck( &d );
    remove( &d, inFlopTop );
    remove( &d, inFlopBot );

    // allow unknown flops too
    CardSet workingFlops[2];

    // but not NULL flops
    workingFlops[0] = copy( inFlopTop );    
    workingFlops[1] = copy( inFlopBot );    
    
    
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

    // now draw cards to populate unknown flops too
    for( int i=0; i<2; i++ ) {
        for( int c=0; c<workingFlops[i].numCards; c++ ) {
            if( strcmp( workingFlops[i].cards[c], "?" ) == 0 ) {
                workingFlops[i].cards[c] = d.cards[ nextCard ];
                nextCard++;
                }
            }
        }
    
    
    phevaluator::Rank handRanksTop[9];
    phevaluator::Rank handRanksBottom[9];

    for( int i=0; i<9; i++ ) {
        if( workingHandsPointers[i] != NULL ) {
            handRanksTop[i] =
                getRank( &workingFlops[0],
                         turn, river, workingHandsPointers[i] );
            handRanksBottom[i] =
                getRank( &workingFlops[1],
                         turn, river, workingHandsPointers[i] );
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

    if( inPrintResult ) {
        
        printf( "\n\nBoard = \n" );
        print( &workingFlops[0] );

        printf( "\n         " );
        printf( "%s ", turn );
        printf( "%s\n", river );

        print( &workingFlops[1] );
        printf( "\n" );


        printf( "\nHands:\n" );
        for( int i=0; i<9; i++ ) {
            if( workingHandsPointers[i] != NULL ) {
                print( workingHandsPointers[i] );

                if( r.winners[i] ) {
                    printf( "   WINNER" );
                    }
                else {
                    printf( "         " );
                    }
                
                const char *des = describe_rank( bestHandValues[i] );
                
                printf( "    %s", des );

                printf( "\n" );
                }
            }

        printf( "\n\n" );
        }

    
    return r;
    }



void usage() {
    printf( "Usage:\n\n" );
    printf( "myPLO4 handFile.txt\n\n" );
    printf( "OR Usage:\n\n" );
    printf( "myPLO4 handSize numPlayers\n\n" );
    printf( "OR Usage:\n\n" );
    printf( "myPLO4 badBeat card1 card2 card3 card4 card5\n\n" );
    exit( 1 );
    }





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
    c.numCards = 6;
    for( int i=0; i<6; i++ ) {
        c.cards[i] = "?";
        }
    
    char cardA[3];
    char cardB[3];
    char cardC[3];
    char cardD[3];
    char cardE[3];
    char cardF[3];

    char *cards[6];
    cards[0] = cardA;
    cards[1] = cardB;
    cards[2] = cardC;
    cards[3] = cardD;
    cards[4] = cardE;
    cards[5] = cardF;


    long startPos = ftell( inFile );

    char lineBuffer[200];

    // read full line
    char *line = fgets( lineBuffer, 200, inFile );

    if( line == NULL ) {
        // failed
        return c;
        }

    
    // try reading 6, but count num read to determine hand size
    int numRead = sscanf( line,
                          "hand %2s %2s %2s %2s %2s %2s\n",
                          cardA, cardB, cardC, cardD, cardE, cardF  );
    if( numRead == 4 || numRead == 5 || numRead == 6 ) {
        *outSuccess = true;
        
        c.numCards = numRead;
        
        for( int i=0; i<c.numCards; i++ ) {
            c.cards[i] = getConstCard( cards[i] );
            }
        /*
        printf( "Read hand from file:  " );
        print( &c );
        printf( "\n" );
        */
        }
    else {
        // not a valid hand
        // rewind to start of line
        fseek( inFile, startPos, SEEK_SET );
        
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



char getRank( const char *inCard ) {
    return inCard[0];
    }

char getSuit( const char *inCard ) {
    return inCard[1];
    }


// 2 = 0
// 3 = 1
// 4 = 2
// ...
// K = 11
// A = 12
int getRankNum( const char *inCard, char inAceLow = false ) {
    char r = getRank( inCard );

    if( inAceLow ) {
        if( r <= '9' ) {
            return ( r - '1' );
            }
        switch( r ) {
            case 'T':
                return 9;
            case 'J':
                return 10;
            case 'Q':
                return 11;
            case 'K':
                return 12;
            case 'A':
                return 0;
            }
        }
    else {
        
        if( r <= '9' ) {
            return ( r - '1' ) - 1;
            }
        switch( r ) {
            case 'T':
                return 8;
            case 'J':
                return 9;
            case 'Q':
                return 10;
            case 'K':
                return 11;
            case 'A':
                return 12;
            }
        }
    return 0;
    }


char numToRank( int inRankNum ) {
    return allRanks[ inRankNum ];
    }




char rankPresent( CardSet *inSet, int inRankNumber ) {
    for( int i=0; i<inSet->numCards; i++ ) {
        int r = getRankNum( inSet->cards[i] );
        
        if( r == inRankNumber ) {
            return true;
            }
        }
    
    return false;
    }


int countRankOccurence( CardSet *inSet, int inRankNumber ) {
    int count = 0;
    for( int i=0; i<inSet->numCards; i++ ) {
        int r = getRankNum( inSet->cards[i] );
        
        if( r == inRankNumber ) {
            count++;
            }
        }
    
    return count;
    }



int countSuitHits( CardSet *inSet, char inSuit ) {
    int count = 0;
    
    for( int i=0; i<inSet->numCards; i++ ) {
        char s = getSuit( inSet->cards[i] );
        if( s == inSuit ) {
            count++;
            }
        }
    
    return count;
    }



char isStraightPresent( char inRankPresentMap[14],
                        int inRequireR1, int inRequireR2 ) {
    
    int seqCount = 0;
    char r1Hit = false;
    char r2Hit = false;
    int r1HitAgo = 0;
    int r2HitAgo = 0;
    
    for( int i=0; i<14; i++ ) {
        if( r1Hit ) {
            r1HitAgo ++;
            }
        if( r2Hit ) {
            r2HitAgo ++;
            }
        
        if( inRankPresentMap[i] ) {
            if( i == 0 || i == 13 ) {
                // A rank
                if( inRequireR1 == 12 ) {
                    r1Hit = true;
                    r1HitAgo = 0;
                    }
                if( inRequireR2 == 12 ) {
                    r2Hit = true;
                    r2HitAgo = 0;
                    }
                }
            else {
                // non-A rank
                int r = i - 1;
                if( inRequireR1 == r ) {
                    r1Hit = true;
                    r1HitAgo = 0;
                    }
                if( inRequireR2 == r ) {
                    r2Hit = true;
                    r2HitAgo = 0;
                    }
                }
            
            seqCount ++;
            if( seqCount == 5 ) {
                if( r1Hit && r2Hit ) {
                    return true;
                    }
                else if( r1HitAgo < 4 && r2HitAgo < 4 ) {
                    // decrement sequence count, and continue looking
                    // for both to be hit
                    seqCount --;
                    }
                else {
                    // reached 5, but one that we've hit is already too
                    // far away
                    return false;
                    }
                }
            }
        else {
            seqCount = 0;
            r1Hit = false;
            r2Hit = false;
            }
        }
    return false;
    }


int countStraightFillOne( char inRankPresentMap[14],
                          int inRequireR1, int inRequireR2 ) {
    
    int countStraights = 0;
    
    for( int i=0; i<14; i++ ) {
        if( ! inRankPresentMap[i] ) {
            // empty spot
            // try filling it, and see if there's a straight now
            inRankPresentMap[i] = true;
            
            char straightNow = isStraightPresent( inRankPresentMap,
                                                  inRequireR1,
                                                  inRequireR2 );
            
            if( straightNow ) {
                countStraights ++;
                }

            // empty it again
            inRankPresentMap[i] = false;
            }
        }
    
    return countStraights;
    }



int countStraightFillTwo( char inRankPresentMap[14],
                        int inRequireR1, int inRequireR2 ) {
    
    int countStraights = 0;
    
    for( int i=0; i<14; i++ ) {
        for( int j=i+1; j<14; j++ ) {
            if( ! inRankPresentMap[i] && ! inRankPresentMap[j] ) {
                // two empty spots
                // try filling both, and see if there's a straight now
                inRankPresentMap[i] = true;
                inRankPresentMap[j] = true;
                
                char straightNow = isStraightPresent( inRankPresentMap,
                                                      inRequireR1,
                                                      inRequireR2 );
            
                if( straightNow ) {
                    countStraights ++;
                    }
                
                // empty both again
                inRankPresentMap[i] = false;
                inRankPresentMap[j] = false;
                }
            }
        
        }
    return countStraights;
    }





    
// custom hand categorization code, assuming a 5-card hand (two hole
// cards, 3 board cards, 2 cards to come)
// This categorizes draws too

// if in____DrawOnly is true, we only describe draws for hands that
// are not straights or flushes (we don't describe one or two pair,
// for example).  If both are true, we describe both  

// string destroyed by caller
char *categorizeHand( CardSet *inFiveCardHand,
                      char inFlushDrawOnly,
                      char inStraightDrawOnly,
                      char *outIsFlushOrBetter, char *outIsStraightOrBetter ) {
    
    char isFlush = false;
    char isStraight = false;

    char isFlushDraw = false;
    
    char isBackdoorFlushDraw = false;

    // since this is omaha, look at both hole cards for flush
    char possibleFlushSuit = '?';
    
    if( getSuit( inFiveCardHand->cards[0] ) ==
        getSuit( inFiveCardHand->cards[1] ) ) {
    
        possibleFlushSuit = getSuit( inFiveCardHand->cards[0] ) ;
        }
    
    int suitHits = countSuitHits( inFiveCardHand, possibleFlushSuit );
    
    switch( suitHits ) {
        case 5:
            isFlush = true;
            break;
        case 4:
            isFlushDraw = true;
            break;
        case 3:
            isBackdoorFlushDraw = true;
            break;
        }
    


    // flags for where rank present
    // A at top and bottom
    char rankPresentMap[14];

    for( int i=0; i<14; i++ ) {
        rankPresentMap[i] = false;
        }
    

    int maxRank = 0;

    if( rankPresent( inFiveCardHand, 12 ) ) {
        // A present
        rankPresentMap[0] = true;
        rankPresentMap[13] = true;
        maxRank = 12;
        }

    
    for( int i=0; i<12; i++ ) {
        char p = rankPresent( inFiveCardHand, i );
        rankPresentMap[ i + 1 ] = p;

        if( p && i > maxRank ) {
            maxRank = i;
            }
        }

    
    char isOpenEndedStraightDraw = false;
    char isGutshotStraightDraw = false;
    
    char isBackdoorStraightDraw = false;
 


    int r1 = getRankNum( inFiveCardHand->cards[0] );
    int r2 = getRankNum( inFiveCardHand->cards[1] );
    

    if( r1 != r2 ) {
        // can only have straights and straight draws
        // if our hole cards are different

        
        isStraight = isStraightPresent( rankPresentMap, r1, r2 );
    
        int countStraightDraws = countStraightFillOne( rankPresentMap,
                                                       r1, r2 );
        
        int countStraightBackdoorDraws =
            countStraightFillTwo( rankPresentMap, r1, r2 );
        
        
        if( ! isStraight ){
            if( countStraightDraws > 0 ) {
                
                if( countStraightDraws > 1 ) {
                    isOpenEndedStraightDraw = true;
                    }
                else {
                    isGutshotStraightDraw = true;
                    }
                }
            else if( countStraightBackdoorDraws > 0 ) {
                isBackdoorStraightDraw = true;
                }
            }
        

        if( isStraight && maxRank == 12 &&
            ! rankPresent( inFiveCardHand, 11 ) ) {
            // straight with an A in it, but no K
            // 5 must actually be the max rank
            maxRank = 3;
            }
        }

    
    *outIsFlushOrBetter = isFlush;

    *outIsStraightOrBetter = isStraight || isFlush;


    char *buffer = new char[100];


    if( ! inFlushDrawOnly && ! inStraightDrawOnly ) {
        
        if( isStraight && ! isFlush ) {
            const char *extra = "";
            if( isBackdoorFlushDraw ) {
                extra = " with Backdoor Flush Draw";
                }
            else if( isFlushDraw ) {
                extra = " with Flush Draw";
                }
            
            snprintf( buffer, 100, "%c-high Straight%s", 
                      numToRank( maxRank ), extra );
            
            return buffer;
            }
        
        if( isFlush && ! isStraight ) {
            snprintf( buffer, 100, "%c-high Flush", 
                      numToRank( maxRank ) );
            
            return buffer;
            }
        if( isFlush && isStraight ) {
            snprintf( buffer, 100, "%c-high Straight Flush", 
                      numToRank( maxRank ) );
            
            return buffer;
            }
        }
    
    // flushes and straights never have duplicate ranks (never have pairs)
    
    // now look for things with duplicate ranks (quads, full houses, sets,
    // two pair, 
    
    // these can also be other draws (if not quads or full houses)
    const char *extraA = "";
    if( isBackdoorFlushDraw ) {
        extraA = " with Backdoor Flush Draw";
        }
    else if( isFlushDraw ) {
        extraA = " with Flush Draw";
        }

    const char *extraB = "";
    if( isOpenEndedStraightDraw ) {
        extraB = " with Open-ended Straight Draw";
        }
    else if( isGutshotStraightDraw ) {
        extraB = " with Gutshot Straight Draw";
        }
    else if( isBackdoorStraightDraw ) {
        extraB = " with Backdoor Straight Draw";
        }


    // now handle case where we want to describe our draws only
    if( inStraightDrawOnly && inFlushDrawOnly ) {
        snprintf( buffer, 100, "%s%s", extraA, extraB );
        return buffer;
        }
    else if( inFlushDrawOnly ) {
        snprintf( buffer, 100, "%s", extraA );
        return buffer;
        }
    else if( inStraightDrawOnly ) {
        snprintf( buffer, 100, "%s", extraB );
        return buffer;
        }


    
    int rankCountMap[13];

    int numRanksOccurring = 0;

    int maxRankCount = 0;
    int maxOccurringRank = -1;
    
    int secondMaxRankCount = 0;
    int secondMaxOccurringRank = -1;
    
    int highestRank = 0;

    for( int i=0; i<13; i++ ) {
        rankCountMap[i] = countRankOccurence( inFiveCardHand, i );
        
        if( rankCountMap[i] > maxRankCount ) {
            // demote previous to second
            secondMaxRankCount = maxRankCount;
            secondMaxOccurringRank = maxOccurringRank;
            
            maxRankCount = rankCountMap[i];
            maxOccurringRank = i;
            }
        else if( rankCountMap[i] > secondMaxRankCount ) {
            secondMaxRankCount = rankCountMap[i];
            secondMaxOccurringRank = i;
            }

        if( rankCountMap[i] > 0 ) {
            numRanksOccurring ++;
            if( i > highestRank ) {
                highestRank = i;
                }
            }
        }
    
    
    if( maxRankCount == 4 ) {
        *outIsStraightOrBetter = true;
        *outIsFlushOrBetter = true;
        
        snprintf( buffer, 100, "Quad %c's", numToRank( maxOccurringRank ) );
        
        return buffer;
        }
    
    if( maxRankCount == 3 ) {
        // either full house or set
        
        if( numRanksOccurring == 2 ) {
            // full house
            *outIsStraightOrBetter = true;
            *outIsFlushOrBetter = true;

            // check if underfull or overfull
            const char *fullType = "overfull";
            
            if( secondMaxOccurringRank > maxOccurringRank ) {
                fullType = "underfull";
                }
            
            snprintf( buffer, 100, "%c's full of %c's (%s)", 
                      numToRank( maxOccurringRank ),
                      numToRank( secondMaxOccurringRank ), fullType );
        
            return buffer;
            }
        else {
            // set
            snprintf( buffer, 100, "Set of %c's%s%s", 
                      numToRank( maxOccurringRank ),
                      extraA, extraB );
            return buffer;
            }    
        }
    else if( maxRankCount == 2 ) {
        // one or two pair
        if( numRanksOccurring == 3 ) {
            // two pair
            snprintf( buffer, 100, "Two Pair, %c's and %c's%s%s", 
                      numToRank( maxOccurringRank ),
                      numToRank( secondMaxOccurringRank ),
                      extraA, extraB );
            return buffer;
            }
        else {
            // one pair
            snprintf( buffer, 100, "One Pair of %c's%s%s", 
                      numToRank( maxOccurringRank ),
                      extraA, extraB );
            return buffer;
            }
        }
    else {
        // no pair
        // what is the highest card?
        
        snprintf( buffer, 100, "%c High%s%s", 
                      numToRank( highestRank ),
                      extraA, extraB );

        return buffer;
        }
        
    }



// gets string for best hand you can make with a 4, 5, or 6-card hand
// plus a 3 (flop) or 5-card (flop, turn, river) board
// Includes descriptions of draws for hand+flop
//
// string destroyed by caller
// can return NULL if hand or board contain ?
char *categorizeHand( CardSet *inHand, CardSet *inBoard ) {

    for( int i=0; i<inHand->numCards; i++ ) {
        if( strcmp( inHand->cards[i], "?" ) == 0 ) {
            return NULL;
            }
        }
    for( int i=0; i<inBoard->numCards; i++ ) {
        if( strcmp( inBoard->cards[i], "?" ) == 0 ) {
            return NULL;
            }
        }
    
    
    // if board has 5 cards, then we use library categorization code
    // since draws irrelevant
    if( inBoard->numCards == 5 ) {
        phevaluator::Rank rank;
        
        if( inHand->numCards == 4 ) {
            rank =
                phevaluator::EvaluatePlo4Cards( inBoard->cards[0],
                                                inBoard->cards[1],
                                                inBoard->cards[2],
                                                inBoard->cards[3],
                                                inBoard->cards[5],
                                                inHand->cards[0],
                                                inHand->cards[1],
                                                inHand->cards[2],
                                                inHand->cards[3] );
            }
        else if( inHand->numCards == 5 ) {
            rank =
                phevaluator::EvaluatePlo5Cards( inBoard->cards[0],
                                                inBoard->cards[1],
                                                inBoard->cards[2],
                                                inBoard->cards[3],
                                                inBoard->cards[5],
                                                inHand->cards[0],
                                                inHand->cards[1],
                                                inHand->cards[2],
                                                inHand->cards[3],
                                                inHand->cards[4] );
            }
        else if( inHand->numCards == 6 ) {
            rank =
                phevaluator::EvaluatePlo6Cards( inBoard->cards[0],
                                                inBoard->cards[1],
                                                inBoard->cards[2],
                                                inBoard->cards[3],
                                                inBoard->cards[5],
                                                inHand->cards[0],
                                                inHand->cards[1],
                                                inHand->cards[2],
                                                inHand->cards[3],
                                                inHand->cards[4],
                                                inHand->cards[5] );
            }
        
        const char* des = describe_rank( rank.value() );
        char *buffer = new char[100];

        snprintf( buffer, 100, "%s", des );
        
        return buffer;
        }
    else if( inBoard->numCards != 3 ) {
        // if board has 4 (or not 3) cards, we just don't support this
        
        char *buffer = new char[100];

        snprintf( buffer, 100, 
                  "Boards with %d cards not supported",
                  inBoard->numCards );
        
        return buffer;
        }
    
    


    


    // if board has 3 cards, then we want to use our categorization code
    // to look at draws too


    // first, find highest-rank 2-card subset of hole cards that go
    // along with the board to make the best hand

    int bestI = -1;
    int bestJ = -1;
    
    int bestValue = 999999;
    
    for( int i=0; i<inHand->numCards; i++ ) {
        const char *cardI = inHand->cards[i];
        
        for( int j=i+1; j<inHand->numCards; j++ ) {
            // test pair of cards from spot i and j in hand
            
            const char *cardJ = inHand->cards[j];
            
            phevaluator::Rank r =
                phevaluator::EvaluateCards( cardI, cardJ,
                                            inBoard->cards[0],
                                            inBoard->cards[1],
                                            inBoard->cards[2] );
            int v = r.value();
            
            if( v < bestValue ) {
                bestValue = v;
                bestI = i;
                bestJ = j;
                }
            }
        }

    
    
    
    CardSet c;
    c.numCards = 5;
    c.cards[0] = inHand->cards[bestI];
    c.cards[1] = inHand->cards[bestJ];
    c.cards[2] = inBoard->cards[0];
    c.cards[3] = inBoard->cards[1];
    c.cards[4] = inBoard->cards[2];

    char straightOrBetter = false;
    char flushOrBetter = false;
    
    char *mainDes = categorizeHand( &c, false, false,
                                    &flushOrBetter,
                                    &straightOrBetter );

    if( !flushOrBetter ) {
        // run again with our alternate pairs of hold cards

        for( int k=0; k<inHand->numCards; k++ ) {
            for( int l=k+1; l<inHand->numCards; l++ ) {

                if( k != bestI || l != bestJ ) {
                    // not our best hole cards
                    
                    c.cards[0] = inHand->cards[k];
                    c.cards[1] = inHand->cards[l];

                    for( int p=0; p<2; p++ ) {
                        
                    
                        char *drawDes;

                        if( p == 0 ) {
                            
                            drawDes = categorizeHand( &c,
                                                      true,
                                                      false,
                                                      &flushOrBetter,
                                                      &straightOrBetter );
                            }
                        else if( p == 1 ) {
                            if( straightOrBetter ) {
                                break;
                                }
                            drawDes = categorizeHand( &c,
                                                      false,
                                                      true,
                                                      &flushOrBetter,
                                                      &straightOrBetter );
                            }

                        if( strstr( mainDes, drawDes ) == NULL ) {
                            // this draw des not already present
                            // add it
                            
                            char *buffer = new char[200];
                            
                            snprintf( buffer, 200, "%s%s", mainDes, drawDes );
                            
                            delete [] mainDes;
                            delete [] drawDes;
                            mainDes = buffer;
                            }
                        else {
                            // already present, discard it
                            delete [] drawDes;
                            }
                        }
                    }
                }
            }
        }
    
    return mainDes;
    }




float computeWinEquity(
    CardSet *inFlopTop, CardSet *inFlopBottom,
    CardSet *inHand, int inNumTotalPlayers, int inNumSims ) {

    int numRuns = 0;
    int winCount[9] = { 0,0,0,0,0,0,0,0,0 };
    int tieCount[9] = { 0,0,0,0,0,0,0,0,0 };

    int numCardsInHand = inHand->numCards;

    CardSet hands[9];

    CardSet *handPointers[9];

    for( int i=0; i<9; i++ ) {
        handPointers[i] = NULL;
        }
    handPointers[0] = inHand;

    for( int i=1; i<inNumTotalPlayers; i++ ) {
        hands[i].numCards = inHand->numCards;
        for( int c=0; c< inHand->numCards; c++ ) {
            hands[i].cards[c] = "?";
            }
        handPointers[i] = &( hands[i] );
        }
    
    
    for( int i=0; i<inNumSims; i++ ) {
        Result r = simWinner( inFlopTop, inFlopBottom, handPointers );

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


    
    return 100 * (float) winCount[0] / (float) numRuns;
    }



typedef struct BombPotSituation {
        CardSet flopTop;
        CardSet flopBottom;
        CardSet hand;
    } BombPotSituation;
        


BombPotSituation getRandomBombPotSituation( int inHandSize ) {
    Deck d;
    setupFreshDeck( &d );
    shuffle( &d );

    int nextCard = 0;

    BombPotSituation s;

    s.flopTop.numCards = 3;
    s.flopBottom.numCards = 3;
    
    for( int i=0; i<3; i++ ) {
        s.flopTop.cards[i] = d.cards[nextCard];
        nextCard++;
        s.flopBottom.cards[i] = d.cards[nextCard];
        nextCard++;        
        }
    s.hand.numCards = inHandSize;
    for( int i=0; i<inHandSize; i++ ) {
        s.hand.cards[i] = d.cards[nextCard];
        nextCard++;
        }

    return s;
    }


// finds situation with equity in band
BombPotSituation findBombPotSituationWithEquity( int inHandSize,
                                                 int inNumTotalPlayers,
                                                 float inLowEquity,
                                                 float inHighEquity,
                                                 float *outEquity ) {

    while( true ) {
        BombPotSituation s = getRandomBombPotSituation( inHandSize );

        float roughEquity = computeWinEquity(
            &( s.flopTop ), &( s.flopBottom ),
            &( s.hand ),
            inNumTotalPlayers, 20 );

        if( roughEquity >= inLowEquity &&
            roughEquity <= inHighEquity ) {
            // our rough/quick equity calculation makes this a candidate

            float finerEquity = computeWinEquity(
                &( s.flopTop ), &( s.flopBottom ),
                &( s.hand ),
                inNumTotalPlayers, 200 );

            if( finerEquity >= inLowEquity &&
                finerEquity <= inHighEquity ) {

            
                // do more sims to verify equity is really in the desired band
                float fineEquity = computeWinEquity(
                    &( s.flopTop ), &( s.flopBottom ),
                    &( s.hand ),
                    inNumTotalPlayers, 2000 );

                if( fineEquity >= inLowEquity &&
                    fineEquity <= inHighEquity ) {
                    *outEquity = fineEquity;
                    return s;
                    }
                }       
            // else try again
            }
        }
    }





int runUntilBadBeat( int inRankToBeat ) {
    
    int runCount = 0;
        
    int countBetterOccurred = 0;
    
    CardSet playerHands[9];
    CardSet board;
    
    int rankValues[9];
        
    while( runCount < 100000 ) {
        Deck d;
        setupFreshDeck( &d );
        shuffle( &d );
            

        int cardI = 0;
            
        // deal to players
        for( int p=0; p<9; p++ ) {
            playerHands[p].numCards = 2;
            for( int c=0; c<2; c++ ) {
                playerHands[p].cards[c] = d.cards[cardI];
                cardI++;
                }
            }
            
        // deal board
        board.numCards = 5;
        for( int c=0; c<5; c++ ) {
            board.cards[c] = d.cards[cardI];
            cardI++;
            }
            
        // bigger or equal to
        int numBiggerThanMinRank = 0;
        char playersBiggerThanMinRank[9];
            
        int playerValues[9];
            
        for( int p=0; p<9; p++ ) {
            playersBiggerThanMinRank[p] = false;
                
            phevaluator::Rank rank = 
                phevaluator::EvaluateCards(
                    playerHands[p].cards[0],
                    playerHands[p].cards[1],
                    board.cards[0],
                    board.cards[1],
                    board.cards[2],
                    board.cards[3],
                    board.cards[4] );
                 
            int val = rank.value();

            playerValues[p] = val;

                
            if( val <= inRankToBeat ) {
                    
                // make sure both hole cards play
                int bestValWithBoth = 9999999;
                    
                // loop over cards from board to skip
                for( int s1=0; s1<5; s1++ ) {
                    for( int s2=s1+1; s2<5; s2++ ) {
                            
                        const char *testBoard[5];
                            
                        memcpy( testBoard,
                                board.cards, sizeof( char * ) * 5 );
                            
                        testBoard[s1] = playerHands[p].cards[0];
                        testBoard[s2] = playerHands[p].cards[1];
                            
                        phevaluator::Rank testRank = 
                            phevaluator::EvaluateCards(
                                testBoard[0],
                                testBoard[1],
                                testBoard[2],
                                testBoard[3],
                                testBoard[4] );
                            
                        int testValue = testRank.value();
                            
                        if( testValue < bestValWithBoth ) {
                            bestValWithBoth = testValue;
                            }
                        }
                    }
                    
                if( bestValWithBoth == val ) {
                    // best val the same when both hole cards play


                    char badQuads = false;
                    
                    // but if we have QUADS, we need to do another test
                    // need to make sure both hole cards are part of quads
                    if( val <= 166 && val > 10 ) {
                        // don't run this test on straight flushes
                        // which are value 10 and lower

                        if( playerHands[p].cards[0][0]  !=
                            playerHands[p].cards[1][0] ) {
                            // hole cards have different rank chars
                            // not part of quads
                            badQuads = true;
                            }
                        }
                    

                    if( ! badQuads ) {
                        // we can count this
                        numBiggerThanMinRank++;
                        
                        playersBiggerThanMinRank[p] = true;
                        }
                    }
                }
            }
            
        if( numBiggerThanMinRank >= 1 ) {
            countBetterOccurred ++;
            }
        if( numBiggerThanMinRank >= 2 ) {
            // make sure they aren't tied
            int val1 = -1;
            int val2 = -1;

            for( int p=0; p<9; p++ ) {
                if( playersBiggerThanMinRank[p] ) {
                    if( val1 == -1 ) {
                        val1 = playerValues[p];
                        }
                    else if( val2 == -1 ) {
                        val2 = playerValues[p];
                        }
                    }
                }

            if( val1 != val2 ) {
                
                printf( "Bad beat occured on run %d\n", runCount );
                
                printf( "\nBoard:  " );
                print( & board );
                printf( "\n\nQualifying hands:\n" );
                
                for( int p=0; p<9; p++ ) {
                    if( playersBiggerThanMinRank[p] ) {
                        printf( "Seat %d:  ", p + 1 );
                        
                        print( & playerHands[p] );
                        printf( "     " );
                        const char *des = describe_rank( playerValues[p] );
                        printf( "%s, rank=%d\n", des, playerValues[p] );
                        }
                    }
                
                break;
                }
            }
            

        runCount++;
        }
        
        
    printf( "%d runs occurred where 1 hand qualified\n", 
            countBetterOccurred );

    return runCount;
    }





int main( int inNumArgs, const char **inArgs ) {
    
    srand( time( NULL ) );
    //srand( 3974987 );

    if( inNumArgs != 2 && inNumArgs != 3 && inNumArgs != 7) {
        usage();
        }


    if( inNumArgs == 7 && strcmp( inArgs[1], "badBeat" ) == 0 ) {
        
        int rankToBeat = phevaluator::EvaluateCards(
            inArgs[2], inArgs[3], inArgs[4], inArgs[5], inArgs[6] ).value();
        
        printf( "Hand to beat = %s %s %s %s %s, rank = %d, %s\n",
                inArgs[2], inArgs[3], inArgs[4], inArgs[5], inArgs[6],
                rankToBeat, describe_rank( rankToBeat ) );

        int numFullRuns = 100;

        float runningAverage = 0;

        for( int i=0; i<numFullRuns; i++ ) {
            int numRuns = runUntilBadBeat( rankToBeat );

            runningAverage += numRuns / (float)numFullRuns;
            }

        printf( "Average num runs until bad beat:  %f\n", runningAverage );
        
        return 0;
        }
    else if( inNumArgs == 3 ) {
        int handSize = 0;
        int numPlayers = 0;

        sscanf( inArgs[1], "%d", &handSize );
        sscanf( inArgs[2], "%d", &numPlayers );

        // 10 equity bands
        for( int b=0; b<100; b+= 5 ) {

            float low = b;
            float high = b + 5;

            float equity;
            BombPotSituation s = findBombPotSituationWithEquity( handSize,
                                                                 numPlayers,
                                                                 low, high,
                                                                 &equity );
            char *catTop = categorizeHand( &( s.hand ),
                                           &( s.flopTop ) );
            char *catBot = categorizeHand( &( s.hand ),
                                           &( s.flopBottom ) );

            printf( "\n\nEquity in band %.0f%% - %.0f%% (%.1f%%):\n",
                    low, high,
                    equity );
            printf( "  Board:\n   " );
            print( &( s.flopTop ) );
            printf( "\n   " );
            print( &( s.flopBottom ) );
            printf( "\n  Hand:\n   " );
            print( &( s.hand ) );
            printf( "\n" );

            if( catTop != NULL ) {
                printf( "  On top board: %s\n", catTop );
                delete [] catTop;
                }
            if( catBot != NULL ) {
                printf( "  On bottom board: %s\n", catBot );
                delete [] catBot;
                }
            }
        
        
        
        return 0;
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

    // make sure there are no duplicate (or bad) cards in flops or hands
    char allGoodCards = true;

    Deck d;
    setupFreshDeck( &d );


    
    char thisGood = remove( &d, &flopTop );
    if( ! thisGood ) {
        printf( "Duplicate card present in Top Flop: " );
        print( &flopTop );
        printf( "\n" );
        }
    allGoodCards &= thisGood;

    thisGood = remove( &d, &flopBot );
    if( ! thisGood ) {
        printf( "Duplicate card present in Bottom Flop: " );
        print( &flopBot );
        printf( "\n" );
        }
    allGoodCards &= thisGood;
    

    int numPlayerCards = 0;
    for( int p=0; p<9; p++ ) {
        if( handPointers[p] != NULL ) {
            numPlayerCards += handPointers[p]->numCards;
            
            thisGood = remove( &d, handPointers[p] );
            if( ! thisGood ) {
                printf( "Duplicate card present in Hand %d: ", p + 1 );
                print( handPointers[p] );
                printf( "\n" );
                }
            allGoodCards &= thisGood;
            }
        }


    // 2 flops, 1 turn, 1 river, 8 board cards
    if( numPlayerCards + 8 > 52 ) {
        // we can get here without duplicate cards, if there are ?
        // catch case where we have too many players for hand size
        
        printf( "Too many players for hand sizes (%d cards required)",
                numPlayerCards + 8 );
        allGoodCards = false;
        }


    if( !allGoodCards ) {
        return 1;
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
            printf( "Seat %d  empty / folded\n", p+1 );
            }
        else {
            printf( "Seat %d has  ", p + 1 );
            print( handPointers[p] );
            
            printf( "  wins: %0.1f%%  ties: %0.1f%%\n",
                    100 * (float) winCount[p] / (float) numRuns,
                    100 * (float) tieCount[p] / (float) numRuns );

            char *topDes = categorizeHand( handPointers[p], &flopTop );
            char *botDes = categorizeHand( handPointers[p], &flopBot );

            if( topDes != NULL ) {
                printf( "  On top board: %s\n", topDes );
                delete [] topDes;
                }
            if( botDes != NULL ) {
                printf( "  On bottom board: %s\n", botDes );
                delete [] botDes;
                }
            }
        }

    printf( "\nExample run-out:" );
    simWinner( &flopTop, &flopBot, handPointers, true );
    
    return 0;
    }
