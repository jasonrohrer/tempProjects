#include <string.h>
#include <stdio.h>

#define NUM_TEAMS 40

// actual rankings
const char *rankings[ NUM_TEAMS ] = {
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


// true rankings computed by iterative expected score contribution algorithm
const char *computedRankings[ NUM_TEAMS ] = {
    "2820W",
    "2820G",
    "72X",
    "44163A",
    "2820M",
    "103E",
    "2701E",
    "2820E",
    "2820B",
    "2820Y",
    "2820T",
    "2820D",
    "2820F",
    "6033D",
    "2820R",
    "2820A",
    "603E",
    "2820K",
    "2701D",
    "2701G",
    "2820J",
    "2820Z",
    "3243D",
    "3931B",
    "1359C",
    "56047A",
    "36498D",
    "40940A",
    "2701F",
    "3243C",
    "40940C",
    "6033H",
    "3931A",
    "63599B",
    "6033C",
    "3243A",
    "40940F",
    "56047B",
    "40940B",
    "63599D" 
    };

    




#define NUM_MATCHES 160

const char *pairings[ NUM_MATCHES ][2] = {
    { "72X", "40940B" },
    { "2820Y", "2820D" },
    { "2820W", "6033C" },
    { "3931B", "63599D" },
    { "3243A", "2701D" },
    { "3243D", "2820T" },
    { "2701E", "2820G" },
    { "56047B", "40940A" },
    { "2820E", "63599B" },
    { "2820Z", "40940C" },
    { "40940F", "2820A" },
    { "36498D", "2820J" },
    { "103E", "56047A" },
    { "2820K", "1359C" },
    { "3931A", "6033H" },
    { "2820B", "2820R" },
    { "2701F", "2820M" },
    { "6033D", "2820F" },
    { "2701G", "44163A" },
    { "603E", "3243C" },
    { "2820G", "40940F" },
    { "2820D", "72X" },
    { "63599D", "103E" },
    { "2820Y", "2820E" },
    { "2820K", "56047B" },
    { "40940A", "40940C" },
    { "56047A", "2820W" },
    { "2820J", "2820A" },
    { "2820M", "3243A" },
    { "6033H", "6033C" },
    { "3931A", "36498D" },
    { "44163A", "2701F" },
    { "2820F", "63599B" },
    { "40940B", "2701E" },
    { "3243D", "2820B" },
    { "2701D", "1359C" },
    { "3243C", "6033D" },
    { "2820T", "3931B" },
    { "2820Z", "2701G" },
    { "2820R", "603E" },
    { "6033C", "40940A" },
    { "40940F", "56047A" },
    { "63599B", "2820J" },
    { "40940B", "6033H" },
    { "3243A", "103E" },
    { "63599D", "44163A" },
    { "1359C", "56047B" },
    { "2820D", "2820G" },
    { "2820M", "2820K" },
    { "3243C", "2701E" },
    { "2820E", "603E" },
    { "2701D", "2820Y" },
    { "40940C", "72X" },
    { "2820T", "2820W" },
    { "2820R", "3931A" },
    { "2820B", "2820F" },
    { "2701F", "3243D" },
    { "2820A", "2701G" },
    { "6033D", "3931B" },
    { "36498D", "2820Z" },
    { "6033H", "2820K" },
    { "63599B", "2820Y" },
    { "56047A", "2820J" },
    { "6033C", "3243C" },
    { "2820R", "40940F" },
    { "56047B", "2820G" },
    { "2701F", "40940B" },
    { "2701E", "3243D" },
    { "603E", "3931B" },
    { "103E", "40940A" },
    { "2820A", "40940C" },
    { "72X", "2820W" },
    { "2701D", "3931A" },
    { "2820Z", "3243A" },
    { "2820M", "63599D" },
    { "1359C", "2820B" },
    { "2820E", "2820T" },
    { "2820D", "6033D" },
    { "2820F", "2701G" },
    { "44163A", "36498D" },
    { "2820Y", "56047A" },
    { "2820G", "2820A" },
    { "56047B", "2820R" },
    { "2820J", "2701F" },
    { "40940A", "603E" },
    { "3243C", "40940F" },
    { "6033C", "63599B" },
    { "40940C", "2820B" },
    { "2820Z", "2820M" },
    { "2701G", "2820T" },
    { "3243D", "40940B" },
    { "1359C", "36498D" },
    { "2820K", "44163A" },
    { "103E", "2820D" },
    { "2820W", "6033H" },
    { "2820F", "63599D" },
    { "6033D", "2701D" },
    { "3243A", "2820E" },
    { "2701E", "3931A" },
    { "3931B", "72X" },
    { "2820T", "2820Y" },
    { "2820B", "56047A" },
    { "40940B", "2820G" },
    { "103E", "3243C" },
    { "603E", "36498D" },
    { "2701G", "6033C" },
    { "2701D", "2701F" },
    { "2820A", "2820M" },
    { "2820W", "2820K" },
    { "63599B", "72X" },
    { "3243A", "56047B" },
    { "3931B", "2701E" },
    { "40940A", "2820R" },
    { "40940F", "2820Z" },
    { "6033H", "40940C" },
    { "2820J", "6033D" },
    { "63599D", "3243D" },
    { "1359C", "2820F" },
    { "2820E", "2820D" },
    { "3931A", "44163A" },
    { "2820T", "103E" },
    { "2701F", "2820Y" },
    { "2820A", "3931B" },
    { "2820G", "3243C" },
    { "6033H", "56047B" },
    { "40940B", "603E" },
    { "63599D", "2820R" },
    { "6033C", "2701D" },
    { "36498D", "63599B" },
    { "6033D", "1359C" },
    { "40940C", "2820F" },
    { "40940F", "40940A" },
    { "72X", "2701G" },
    { "2820B", "2820Z" },
    { "2820W", "3243A" },
    { "56047A", "2701E" },
    { "2820M", "2820D" },
    { "3931A", "2820K" },
    { "44163A", "2820J" },
    { "3243D", "2820E" },
    { "2820T", "2820A" },
    { "2820G", "603E" },
    { "72X", "63599D" },
    { "103E", "2701F" },
    { "63599B", "40940F" },
    { "1359C", "6033H" },
    { "2820Y", "2701G" },
    { "2820K", "3243A" },
    { "2820M", "2701D" },
    { "40940C", "2820W" },
    { "2820J", "3243D" },
    { "56047A", "56047B" },
    { "2820R", "2820F" },
    { "3931B", "3931A" },
    { "2820D", "2820Z" },
    { "3243C", "40940B" },
    { "40940A", "2701E" },
    { "6033D", "2820E" },
    { "36498D", "2820B" },
    { "44163A", "6033C" } 
    };


int getRank( const char *inTeam ) {
    for( int i=0; i<NUM_TEAMS; i++ ) {
        if( strcmp( rankings[i], inTeam ) == 0 ) {
            return i + 1;
            }
        }
    return -1;
    }



int getComputedRank( const char *inTeam ) {
    for( int i=0; i<NUM_TEAMS; i++ ) {
        if( strcmp( computedRankings[i], inTeam ) == 0 ) {
            return i + 1;
            }
        }
    return -1;
    }



const char *getOtherTeam( int inMatchNumber, const char *inTeam ) {
    const char *otherTeam = NULL;
            

    if( strcmp( pairings[inMatchNumber][0], inTeam ) == 0 ) {
        otherTeam = pairings[inMatchNumber][1];
        }
    else if( strcmp( pairings[inMatchNumber][1], inTeam ) == 0 ) {
        otherTeam = pairings[inMatchNumber][0];
        }
    return otherTeam;
    }


    
int main() {

    float strengthOfSchedule[ NUM_TEAMS ];
    
    for( int i=0; i<NUM_TEAMS; i++ ) {
        strengthOfSchedule[i] = 0;
        
        const char *team = rankings[i];
        
        int matchCount = 0;

        for( int m=0; m<NUM_MATCHES; m++ ) {
            
            const char *otherTeam = getOtherTeam( m, team );

            if( otherTeam != NULL ) {
                int otherRank = getComputedRank( otherTeam );
                
                strengthOfSchedule[i] += otherRank;
                matchCount++;
                }
            }
        
        if( matchCount > 0 ) {
            strengthOfSchedule[i] /= matchCount;
            }
        }

    
    for( int i=0; i<NUM_TEAMS; i++ ) {
        printf( "%s:  SOS= %.1f\n", rankings[i], strengthOfSchedule[i] );
        }


    int numOysterPairings[ NUM_TEAMS ];
    
    for( int i=0; i<NUM_TEAMS; i++ ) {
        numOysterPairings[i] = 0;

        const char *team = rankings[i];
        
        for( int m=0; m<NUM_MATCHES; m++ ) {
            
            const char *otherTeam = getOtherTeam( m, team );

            if( otherTeam != NULL ) {
                if( strstr( otherTeam, "2820" ) != NULL ) {
                    numOysterPairings[i] ++;
                    
                    }
                }
            }
        }

    printf( "\n\n" );
    
    int numOysterTeams = 0;
    int numNonOysterTeams = 0;
    
    int sumOysterTeamsOysterPairings = 0;
    
    int sumNonOysterTeamsOysterPairings = 0;

    for( int i=0; i<NUM_TEAMS; i++ ) {
        printf( "%s:  Oyster Pairings= %d\n", 
                rankings[i], 
                numOysterPairings[i] );

        const char *team = rankings[i];

        if( strstr( team, "2820" ) != NULL ) {
            numOysterTeams ++;
            sumOysterTeamsOysterPairings += numOysterPairings[i];
            }
        else {
            numNonOysterTeams ++;
            sumNonOysterTeamsOysterPairings += numOysterPairings[i];
            }
        }

    printf( "%d Oyster teams with an average of %.2f Oyster pairings\n",
            numOysterTeams, 
            (float)sumOysterTeamsOysterPairings / numOysterTeams );

    printf( "%d Non-Oyster teams with an average of %.2f Oyster pairings\n",
            numNonOysterTeams, 
            (float)sumNonOysterTeamsOysterPairings / numNonOysterTeams );
    


    };
