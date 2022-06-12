
#include "minorGems/util/random/CustomRandomSource.h"
#include "minorGems/util/random/JenkinsRandomSource.h"


#include <stdio.h>
#include <math.h>

/*
 * Demonstration of Kelly Criterion through simulation.
 *
 * Kelly tells us what fraction of bankroll to bet to maximize growth rate.
 *
 * The only way to measure this reliably is to count the number of bets made
 * before reaching a bankroll goalpost, across many trials.
 *
 * The fraction suggested by Kelly does reach the goalpost faster, on average,
 * than any other fraction.
 *
 * However, other average measures (like average peak BR achieved after a 
 * fixed number of rounds) don't seem to match Kelly.
 *
 * This is because Kelly maximized the geometric mean of such values, not
 * the arithmetic mean.  Taking the geometric mean of the max value reached
 * shows that the Kelly-optimal strategy has the highest geometric mean
 * of bankroll at the end of 200 betting rounds.
 */


double coinHeadsP = 0.55;

double winPayout = 1.0;

CustomRandomSource randSource;
//JenkinsRandomSource randSource( 459 );

int startingBankroll = 1000;

int richThreshold = 2000;

int numTrials = 100000;

//int numGames = 200;



int main() {
    printf( "    Chance of winning: %.0f%%:\n\n", coinHeadsP * 100 );

    // what percentage to bet each round
    for( double p=1.0; p<90.0; p+=1.0  ) {
        
        double outcomeSum = 0;
        double max = 0;
        double min = 999999999999999999999999.0;
        
        double geoMean = 1;
        

        // run trials
        for( int t=0; t<numTrials; t++ ) {
            double br = startingBankroll;
            
            int gameCount = 0;
            while( br < richThreshold ) {
            //for( int g=0; g<numGames; g++ ) {
            
                double betAmount = ( br * p ) / 100.0;

                br -= betAmount;

                // flip coin
                if( randSource.getRandomDouble() <= coinHeadsP ) {
                    // win!
                    br += betAmount;
                    br += winPayout * betAmount;
                    }
                // else loss

                gameCount ++;                
                }

            //outcomeSum += br;
            
            geoMean *= pow( br, 1.0/numTrials );
            

            if( br > max ) {
                max = br;
                }
            else if( br < min ) {
                min = br;
                }
            outcomeSum += gameCount;
            }

        double averageOutcome = outcomeSum / numTrials;
        
        
        printf( "    Betting %.1f%% of BR:  "
                "Double BR after %.2f bets on average\n", 
                p, averageOutcome );
        /*
        printf( "    Betting %.1f%% of BR:  "
                //"Double BR after %.2f bets on average\n", 
                "After 200 bets, BR reached %.0f on geo mean (max=%.0f, min=%.0f)\n", 
                p, geoMean, max, min );
        */
        }

    return 0;
    }
