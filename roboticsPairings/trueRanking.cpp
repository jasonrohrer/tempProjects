#include <assert.h>
#include <string.h>
#include <stdio.h>

#include "minorGems/util/SimpleVector.h"


#define NUM_TEAMS 40

const char *teams[40] = {
    "2820D",
    "2820G",
    "2820W",
    "2820T",
    "2820M",
    "2820E",
    "2701G",
    "2820Y",
    "72X",
    "2820A",
    "103E",
    "2701E",
    "44163A",
    "6033D",
    "2820B",
    "2820F",
    "2820K",
    "3931B",
    "2701F",
    "603E",
    "56047A",
    "3243D",
    "40940C",
    "2820R",
    "2820Z",
    "2701D",
    "2820J",
    "3243A",
    "3243C",
    "40940A",
    "36498D",
    "1359C",
    "63599D",
    "63599B",
    "3931A",
    "6033C",
    "40940F",
    "40940B",
    "6033H",
    "56047B" 
    };



typedef struct TeamRating {
        const char *team;
        float rating;
    } TeamRating;
        


#define NUM_MATCHES 160

typedef struct Match {
        const char *teamA;
        const char *teamB;
        int score;
    } Match;


Match matches[ NUM_MATCHES ] = { 
    { "72X", "40940B", 80 },
    { "2820Y", "2820D", 85 },
    { "2820W", "6033C", 81 },
    { "3931B", "63599D", 32 },
    { "3243A", "2701D", 50 },
    { "3243D", "2820T", 80 },
    { "2701E", "2820G", 90 },
    { "56047B", "40940A", 33 },
    { "2820E", "63599B", 68 },
    { "2820Z", "40940C", 46 },
    { "40940F", "2820A", 29 },
    { "36498D", "2820J", 58 },
    { "103E", "56047A", 62 },
    { "2820K", "1359C", 71 },
    { "3931A", "6033H", 30 },
    { "2820B", "2820R", 78 },
    { "2701F", "2820M", 77 },
    { "6033D", "2820F", 71 },
    { "2701G", "44163A", 73 },
    { "603E", "3243C", 50 },
    { "2820G", "40940F", 84 },
    { "2820D", "72X", 62 },
    { "63599D", "103E", 69 },
    { "2820Y", "2820E", 87 },
    { "2820K", "56047B", 32 },
    { "40940A", "40940C", 38 },
    { "56047A", "2820W", 84 },
    { "2820J", "2820A", 46 },
    { "2820M", "3243A", 76 },
    { "6033H", "6033C", 45 },
    { "3931A", "36498D", 37 },
    { "44163A", "2701F", 76 },
    { "2820F", "63599B", 42 },
    { "40940B", "2701E", 40 },
    { "3243D", "2820B", 75 },
    { "2701D", "1359C", 51 },
    { "3243C", "6033D", 74 },
    { "2820T", "3931B", 71 },
    { "2820Z", "2701G", 34 },
    { "2820R", "603E", 72 },
    { "6033C", "40940A", 37 },
    { "40940F", "56047A", 33 },
    { "63599B", "2820J", 60 },
    { "40940B", "6033H", 42 },
    { "3243A", "103E", 48 },
    { "63599D", "44163A", 56 },
    { "1359C", "56047B", 43 },
    { "2820D", "2820G", 106 },
    { "2820M", "2820K", 88 },
    { "3243C", "2701E", 75 },
    { "2820E", "603E", 89 },
    { "2701D", "2820Y", 52 },
    { "40940C", "72X", 81 },
    { "2820T", "2820W", 89 },
    { "2820R", "3931A", 51 },
    { "2820B", "2820F", 86 },
    { "2701F", "3243D", 42 },
    { "2820A", "2701G", 90 },
    { "6033D", "3931B", 70 },
    { "36498D", "2820Z", 40 },
    { "6033H", "2820K", 44 },
    { "63599B", "2820Y", 67 },
    { "56047A", "2820J", 57 },
    { "6033C", "3243C", 33 },
    { "2820R", "40940F", 47 },
    { "56047B", "2820G", 83 },
    { "2701F", "40940B", 21 },
    { "2701E", "3243D", 76 },
    { "603E", "3931B", 60 },
    { "103E", "40940A", 83 },
    { "2820A", "40940C", 55 },
    { "72X", "2820W", 94 },
    { "2701D", "3931A", 46 },
    { "2820Z", "3243A", 24 },
    { "2820M", "63599D", 20 },
    { "1359C", "2820B", 77 },
    { "2820E", "2820T", 89 },
    { "2820D", "6033D", 87 },
    { "2820F", "2701G", 81 },
    { "44163A", "36498D", 73 },
    { "2820Y", "56047A", 76 },
    { "2820G", "2820A", 88 },
    { "56047B", "2820R", 55 },
    { "2820J", "2701F", 34 },
    { "40940A", "603E", 67 },
    { "3243C", "40940F", 28 },
    { "6033C", "63599B", 25 },
    { "40940C", "2820B", 69 },
    { "2820Z", "2820M", 85 },
    { "2701G", "2820T", 79 },
    { "3243D", "40940B", 32 },
    { "1359C", "36498D", 46 },
    { "2820K", "44163A", 84 },
    { "103E", "2820D", 104 },
    { "2820W", "6033H", 95 },
    { "2820F", "63599D", 62 },
    { "6033D", "2701D", 77 },
    { "3243A", "2820E", 55 },
    { "2701E", "3931A", 70 },
    { "3931B", "72X", 66 },
    { "2820T", "2820Y", 89 },
    { "2820B", "56047A", 63 },
    { "40940B", "2820G", 47 },
    { "103E", "3243C", 48 },
    { "603E", "36498D", 65 },
    { "2701G", "6033C", 37 },
    { "2701D", "2701F", 70 },
    { "2820A", "2820M", 89 },
    { "2820W", "2820K", 86 },
    { "63599B", "72X", 77 },
    { "3243A", "56047B", 20 },
    { "3931B", "2701E", 78 },
    { "40940A", "2820R", 63 },
    { "40940F", "2820Z", 76 },
    { "6033H", "40940C", 25 },
    { "2820J", "6033D", 76 },
    { "63599D", "3243D", 39 },
    { "1359C", "2820F", 59 },
    { "2820E", "2820D", 90 },
    { "3931A", "44163A", 85 },
    { "2820T", "103E", 90 },
    { "2701F", "2820Y", 79 },
    { "2820A", "3931B", 80 },
    { "2820G", "3243C", 85 },
    { "6033H", "56047B", 21 },
    { "40940B", "603E", 50 },
    { "63599D", "2820R", 60 },
    { "6033C", "2701D", 40 },
    { "36498D", "63599B", 38 },
    { "6033D", "1359C", 53 },
    { "40940C", "2820F", 74 },
    { "40940F", "40940A", 20 },
    { "72X", "2701G", 94 },
    { "2820B", "2820Z", 69 },
    { "2820W", "3243A", 86 },
    { "56047A", "2701E", 76 },
    { "2820M", "2820D", 96 },
    { "3931A", "2820K", 51 },
    { "44163A", "2820J", 77 },
    { "3243D", "2820E", 81 },
    { "2820T", "2820A", 90 },
    { "2820G", "603E", 66 },
    { "72X", "63599D", 70 },
    { "103E", "2701F", 78 },
    { "63599B", "40940F", 23 },
    { "1359C", "6033H", 34 },
    { "2820Y", "2701G", 87 },
    { "2820K", "3243A", 52 },
    { "2820M", "2701D", 83 },
    { "40940C", "2820W", 79 },
    { "2820J", "3243D", 55 },
    { "56047A", "56047B", 33 },
    { "2820R", "2820F", 76 },
    { "3931B", "3931A", 42 },
    { "2820D", "2820Z", 80 },
    { "3243C", "40940B", 33 },
    { "40940A", "2701E", 75 },
    { "6033D", "2820E", 78 },
    { "36498D", "2820B", 77 },
    { "44163A", "6033C", 73 } 
    };
    


int getTeamIndex( const char *inTeam ) {
    for( int t=0; t<NUM_TEAMS; t++ ) {
        if( strcmp( teams[t], inTeam ) == 0 ) {
            return t;
            }
        }
    assert( false );
    }


// rate of convergence per round
#define ALPHA 1


#define NUM_ROUNDS 40

int main() {

    float expectedContribution[NUM_ROUNDS][ NUM_TEAMS ];
    

    for( int r=0; r<NUM_ROUNDS; r++ ) {    
        for( int t=0; t<NUM_TEAMS; t++ ) {
            expectedContribution[ r ][ t ] = 1;    
            }
        }

    for( int r=1; r<NUM_ROUNDS; r++ ) {    
        // compute new expected contribution

        
        int numMatches[ NUM_TEAMS ];
        
        float sumDiffs[ NUM_TEAMS ];

        // init this round with last round's expected contributions
        for( int t=0; t<NUM_TEAMS; t++ ) {
            numMatches[ t ] = 0;
            sumDiffs[ t ] = 0;
            
            expectedContribution[ r ][ t ] = 
                expectedContribution[ r-1 ][ t ];    
            }

        // adjust up or down based on how matches deviate from our
        // expectation from last round
        for( int m=0; m<NUM_MATCHES; m++ ) {
            
            const char *a = matches[m].teamA;
            const char *b = matches[m].teamB;
            int score = matches[m].score;
            
            int aI = getTeamIndex( a );
            int bI = getTeamIndex( b );
            
            float contA = expectedContribution[r-1][aI];
            float contB = expectedContribution[r-1][bI];
            
            float expectedScore = contA + contB;
            
            float diff = score - expectedScore;

            float fractA = contA / expectedScore;
            float fractB = contB / expectedScore;
            
            float diffA = fractA * diff;
            float diffB = fractB * diff;
            
            sumDiffs[aI] += diffA;
            sumDiffs[bI] += diffB;
            
            numMatches[aI] ++;
            numMatches[bI] ++;
            }

        
        for( int t=0; t<NUM_TEAMS; t++ ) {
            expectedContribution[r][t] += ALPHA * sumDiffs[t] / numMatches[t];
            }
        }    
    

    int startRound = 35;
    
    printf( "\t" );
    for( int r=startRound; r<NUM_ROUNDS; r++ ) {    
        printf( "%d\t", r );
        }
    printf( "\n" );
    
    for( int t=0; t<NUM_TEAMS; t++ ) {
        printf( "%s:\t", teams[t] );
    
        for( int r=startRound; r<NUM_ROUNDS; r++ ) {    
            printf( "%.1f\t", expectedContribution[r][t] );
            }
        printf( "\n" );
        }
    

    printf( "\n\n" );
    
    TeamRating ratings[NUM_TEAMS];
    for( int t=0; t<NUM_TEAMS; t++ ) {
        ratings[t].team = teams[t];
        ratings[t].rating = expectedContribution[NUM_ROUNDS-1][t];
        }
    
    // sort them
    SimpleVector<TeamRating> allRatings;
    
    allRatings.push_back( ratings, NUM_TEAMS );
    
    SimpleVector<TeamRating> sortedRatings;
    
    
    while( allRatings.size() > 0 ) {
        // find highest rating left
        int highestI = -1;
        float highestR = 0;
        
        for( int i=0; i<allRatings.size(); i++ ) {
            
            TeamRating *r = allRatings.getElement( i );
            
            if( r->rating > highestR ) {
                highestR = r->rating;
                highestI = i;
                }
            }
        assert( highestI != -1 );
        
        sortedRatings.push_back( allRatings.getElementDirect( highestI ) );
        
        allRatings.deleteElement( highestI );
        }

    printf( "True rankings:\n" );
    for( int i=0; i<sortedRatings.size(); i++ ) {
        TeamRating *r = sortedRatings.getElement( i );
        
        printf( "%d:\t%s\t%.1f\n", i+1, r->team, r->rating );
        }
    

    printf( "\nCode output:\n" );
    
    printf( "const char *rankings[%d] = {\n", NUM_TEAMS );
    for( int i=0; i<sortedRatings.size(); i++ ) {
        TeamRating *r = sortedRatings.getElement( i );
        printf( "\"%s\",\n", r->team );
        }
    
    return 0;
    }
