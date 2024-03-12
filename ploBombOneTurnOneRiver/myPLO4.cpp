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




char isStraightPresent( char inRankPresentMap[14] ) {
    
    int seqCount = 0;
    
    for( int i=0; i<14; i++ ) {
        if( inRankPresentMap[i] ) {
            seqCount ++;
            if( seqCount == 5 ) {
                return true;
                }
            }
        else {
            seqCount = 0;
            }
        }
    return false;
    }


int countStraightFillOne( char inRankPresentMap[14] ) {
    
    int countStraights = 0;
    
    for( int i=0; i<14; i++ ) {
        if( ! inRankPresentMap[i] ) {
            // empty spot
            // try filling it, and see if there's a straight now
            inRankPresentMap[i] = true;
            
            char straightNow = isStraightPresent( inRankPresentMap );
            
            if( straightNow ) {
                countStraights ++;
                }

            // empty it again
            inRankPresentMap[i] = false;
            }
        }
    
    return countStraights;
    }



int countStraightFillTwo( char inRankPresentMap[14] ) {
    
    int countStraights = 0;
    
    for( int i=0; i<14; i++ ) {
        for( int j=i+1; j<14; j++ ) {
            if( ! inRankPresentMap[i] && ! inRankPresentMap[j] ) {
                // two empty spots
                // try filling both, and see if there's a straight now
                inRankPresentMap[i] = true;
                inRankPresentMap[j] = true;
                
                char straightNow = isStraightPresent( inRankPresentMap );
            
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
char *categorizeHand( CardSet *inFiveCardHand, char inStraightDrawOnly,
                      char inFlushDrawOnly,
                      char *outIsFlushOrBetter, char *outIsStraightOrBetter ) {
    
    char isFlush = false;
    char isStraight = false;

    char isFlushDraw = false;
    
    char isBackdoorFlushDraw = false;

    // since this is omaha, look at both hole cards for flush
    char possibleFlushSuit = '?';
    
    if( getSuit( inFiveCardHand->cards[0] ) ==
        getSuit( inFiveCardHand->cards[1] ) ) {
    
        possibleFlushSuit = getSuit( inFiveCardHand->cards[3] ) ;
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
    
    isStraight = isStraightPresent( rankPresentMap );
    
    int countStraightDraws = countStraightFillOne( rankPresentMap );
    
    int countStraightBackdoorDraws = countStraightFillTwo( rankPresentMap );
    

    char isOpenEndedStraightDraw = false;
    char isGutshotStraightDraw = false;
    
    char isBackdoorStraightDraw = false;
    
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

    *outIsFlushOrBetter = isFlush;

    *outIsStraightOrBetter = isStraight || isFlush;


    char *buffer = new char[100];
    
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

                if( k != bestI && l != bestJ ) {
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
