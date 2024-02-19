#include <assert.h>

#include "minorGems/util/random/JenkinsRandomSource.h"


JenkinsRandomSource randSource;

#define NUM_PAIRINGS 8

#define NUM_TEAMS 40


typedef struct Team {
        int number;
        char red;
        char blue;
        
        Team *pairings[ NUM_PAIRINGS ];
    } Team;

    

int numTeams = 40;

int numRed = 14;
int numBlue = 2;


void initTeams( Team *inTeams, int inNumTeams, int inNumRed, int inNumBlue ) {
    int numRedCreated = 0;
    int numBlueCreated = 0;
    
    for( int i=0; i<inNumTeams; i++ ) {
        inTeams[i].number = i+1;
        inTeams[i].red = false;
        inTeams[i].blue = false;
        
        if( numRedCreated < inNumRed ) {
            inTeams[i].red = true;
            inTeams[i].number += 100;
            numRedCreated++;
            }
        else if( numBlueCreated < inNumBlue ) {
            inTeams[i].blue = true;
            inTeams[i].number += 200;
            numBlueCreated++;
            }
        
        for( int p=0; p<NUM_PAIRINGS; p++ ) {
            inTeams[i].pairings[p] = NULL;
            }
        }
    }



char isPairingOkay( Team *inTeams, int inNumTeams, 
                    int inIndexA, int inTeamNumberB ) {
    if( inTeams[inIndexA].number == inTeamNumberB ) {
        return false;
        }
    
    for( int p=0; p<NUM_PAIRINGS; p++ ) {
        for( int t=0; t< NUM_PAIRINGS; t++ ) {
            if( inTeams[inIndexA].pairings[t] != NULL
                && inTeams[inIndexA].pairings[t]->number == inTeamNumberB ) {
                
                // already paired
                return false;
                }
            }
        }
    return true;
    }



void generatePairings( Team *inTeams, int inNumTeams ) {
    
    for( int i=0; i<inNumTeams; i++ ) {
        for( int p=0; p<NUM_PAIRINGS; p++ ) {
            
            char found = false;

            while( ! found ) {

                // pick another team at random
            
                int pickI = randSource.getRandomBoundedInt( 0, inNumTeams - 1 );
            
                if( isPairingOkay( inTeams, inNumTeams, 
                                   i,
                                   inTeams[pickI].number ) ) {
                    inTeams[i].pairings[p] = & inTeams[pickI];
                    found = true;
                    }
                }
            }
        }
    }



void printPairings( Team *inTeams, int inNumTeams ) {
    for( int i=0; i<inNumTeams; i++ ) {
        int redCount = 0;
        
        for( int p=0; p<NUM_PAIRINGS; p++ ) {
            if( inTeams[i].pairings[p]->red ) {
                redCount ++;
                }
            }
        
        printf( "Team %d paired with (%d red):\n", 
                inTeams[i].number, redCount );
        
        for( int p=0; p<NUM_PAIRINGS; p++ ) {
            printf( "  %d\n", inTeams[i].pairings[p]->number );
            }
        }
    
    }




int main() {
    
    Team teams[ NUM_TEAMS ];


    int numRunsWithFivePlus = 0;
    int numRunsWithSixPlus = 0;
    int numRunsWithSevenPlus = 0;
    int numRunsWithEightPlus = 0;
    
    int numRunsWhereTeamOneWithFivePlus = 0;
    int numRunsWhereTeamOneWithFivePlusAndTwoBlue = 0;
    int numRunsWhereTeamOneWithSixPlusAndTwoBlue = 0;
    
    int numRunsWhereBlueWithBlue = 0;

    int numRunsWhereTeamOneWithBothBlue = 0;
    

    int numRuns = 100000;
    
    for( int r=0; r<numRuns; r++ ) {    
    
        initTeams( teams, NUM_TEAMS, numRed, numBlue );
        
        generatePairings( teams, NUM_TEAMS );
        
        //printPairings( teams, NUM_TEAMS );
        

        int numRed5OrOver = 0;
        int numRed6OrOver = 0;
        int numRed7OrOver = 0;
        int numRed8OrOver = 0;
        
        for( int i=0; i<NUM_TEAMS; i++ ) {
            int redCount = 0;
            int blueCount = 0;
            
            for( int p=0; p<NUM_PAIRINGS; p++ ) {
                if( teams[i].pairings[p]->red ) {
                    redCount ++;
                    }
                if( teams[i].pairings[p]->blue ) {
                    blueCount ++;
                    }
                }
            if( teams[i].blue ) {
                assert( blueCount <= 1 );
                
                if( blueCount > 0 ) {
                    numRunsWhereBlueWithBlue++;
                    }
                }
            
            if( teams[i].red ) {
                
                if( redCount >= 5 ) {
                    numRed5OrOver ++;
                    }
                if( redCount >= 6 ) {
                    numRed6OrOver ++;
                    }
                if( redCount >= 7 ) {
                    numRed7OrOver ++;
                    }
                if( redCount >= 8 ) {
                    numRed8OrOver ++;
                    }
                }

            if( i==0 &&
                blueCount >=2 ) {
                numRunsWhereTeamOneWithBothBlue ++;
                }
            if( i==0 &&
                redCount >=5 ) {
                numRunsWhereTeamOneWithFivePlus ++;
                
                if( blueCount >= 2 ) {
                    numRunsWhereTeamOneWithFivePlusAndTwoBlue ++;
                    }
                }
            if( i==0 &&
                redCount >=6 ) {
                
                if( blueCount >= 2 ) {
                    numRunsWhereTeamOneWithSixPlusAndTwoBlue ++;
                    }
                }
            }
        
        //printf( "5+ = %d, 6+ = %d, 7+ = %d, 8+ = %d\n", 
        //        numRed5OrOver, numRed6OrOver, numRed7OrOver, numRed8OrOver );
        
        if( numRed5OrOver ) {
            numRunsWithFivePlus ++;
            }
        
        if( numRed6OrOver ) {
            numRunsWithSixPlus ++;
            }
        
        if( numRed7OrOver ) {
            numRunsWithSevenPlus ++;
            }
        
        if( numRed8OrOver ) {
            numRunsWithEightPlus ++;
            }
        }

    printf( "Out of %d tournaments:\n"
            "  %d ( 1 in %.1f ) have some Oyster teams with 5+ Oyster pairings\n"
            "  %d ( 1 in %.1f ) have some Oyster teams with 6+ Oyster pairings\n"
            "  %d ( 1 in %.1f ) have some Oyster teams with 7+ Oyster pairings\n"
            "  %d ( 1 in %.1f ) have some Oyster teams with 8+ Oyster pairings\n"
            "  %d ( 1 in %.1f ) where team 2820D had 5+ Oyster pairings\n"
            "  %d ( 1 in %.1f ) where team 2820D had 5+ Oyster pairings AND 2 Dover pairings\n"
            "  %d ( 1 in %.1f ) where team 2820D had 6+ Oyster pairings AND 2 Dover pairings\n"
            "  %d ( 1 in %.1f ) where Dover teams were paired\n"
            "  %d ( 1 in %.1f ) where Dover teams were BOTH paired with 2820D\n"
            ,
            numRuns,
            numRunsWithFivePlus, 1 / ((float)numRunsWithFivePlus / numRuns ),
            numRunsWithSixPlus, 1 / ( (float)numRunsWithSixPlus / numRuns ),
            numRunsWithSevenPlus, 1 / ( (float)numRunsWithSevenPlus / numRuns ),
            numRunsWithEightPlus, 1 / ( (float)numRunsWithEightPlus / numRuns ),
            numRunsWhereTeamOneWithFivePlus, 
            1 / ( (float)numRunsWhereTeamOneWithFivePlus / numRuns ),
            numRunsWhereTeamOneWithFivePlusAndTwoBlue, 
            1 / ( (float)numRunsWhereTeamOneWithFivePlusAndTwoBlue / numRuns ), 
            numRunsWhereTeamOneWithSixPlusAndTwoBlue, 
            1 / ( (float)numRunsWhereTeamOneWithSixPlusAndTwoBlue / numRuns ), 
            numRunsWhereBlueWithBlue, 
            1 / ( (float)numRunsWhereBlueWithBlue / numRuns ),
            numRunsWhereTeamOneWithBothBlue, 
            1 / ( (float)numRunsWhereTeamOneWithBothBlue / numRuns )

);
            
        
    return 1;
    }
