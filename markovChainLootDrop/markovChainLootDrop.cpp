#include <stdio.h>
#include <math.h>

#include "minorGems/util/random/XoshiroRandomSource.h"
#include "minorGems/util/random/StdRandomSource.h"
#include "minorGems/util/random/JenkinsRandomSource.h"


class FakeRandomSource {
    public:
        FakeRandomSource() {
            counter = 0;
            }
        
        double getRandomDouble() {
            // every 50 times
            counter++;
            
            if( counter >= 50 ) {
                counter = 0;
                return 0;
                }
            return 1;
            }
    protected:
        int counter;
        
        
    };


    


int main() {
    XoshiroRandomSource randSource;
    //StdRandomSource randSource;
    //JenkinsRandomSource randSource;
    //FakeRandomSource randSource;
    

    int chainLength = 98;
    
    double chainStepChance = 98 / 100.0;
    
    
    if( false ) {
        
        // simulation of num steps needed until one hit.  Pascal random variable

        int numTrials = 1000000;
        
        double totalStepsForAllTrials = 0;
        
        double totalStepsSquaredForAllTrials = 0;
        
        double theoreticalExpectedValue = chainLength * 1.0 / chainStepChance;
        
        double theoreticalVariance = 
            chainLength * ( 1 - chainStepChance ) /
            ( chainStepChance * chainStepChance );
        
        
        for( int i=0; i<numTrials; i++ ) {
        
            int currentChainPos = 0;
        
            int stepCount = 0;
        

            while( currentChainPos < chainLength ) {
            
                stepCount++;
            
                if( randSource.getRandomDouble() < chainStepChance ) {
                    currentChainPos++;
                    }
                }
            totalStepsForAllTrials += stepCount;
            totalStepsSquaredForAllTrials += stepCount * stepCount;
            }
    
        double computedExpectedValue = 
            totalStepsForAllTrials / (double)numTrials;
    
        double computedVariance = 
            totalStepsSquaredForAllTrials / (double)numTrials
            - computedExpectedValue * computedExpectedValue;
    

        printf( "Computed EV = %f, Computed VAR = %f, Computed SD = %f\n", 
                computedExpectedValue, computedVariance, 
                sqrt( computedVariance ) );
        
        printf( "Theoretical EV = %f, Theoretical VAR = %f, "
                "Theoretical SD = %f\n", 
                theoreticalExpectedValue, theoreticalVariance,
                sqrt( theoreticalVariance ) );
    
        }
    


    if( true ) {
        
        // simulation of num hits in n steps
        // Chain with one step is Binomial random variable
    
        int numTrials = 100000;
        
        int numStepsPerTrial = 1000;
        

        double totalHitsForAllTrials = 0;
    
        double totalHitsSquaredForAllTrials = 0;

        double theoreticalExpectedValue = 
            numStepsPerTrial / 
            ( chainLength * 1.0 / chainStepChance );
    
        // uknown...
        double theoreticalVariance = 0;


        for( int i=0; i<numTrials; i++ ) {
        
            int currentChainPos = 0;

            double hitCount = 0;
            
            for( int s=0; s<numStepsPerTrial; s++ ) {
                
                if( randSource.getRandomDouble() <= chainStepChance ) {
                    currentChainPos++;
                    
                    if( currentChainPos >= chainLength ) {
                        // hit end of chain
                        hitCount++;
                        // go back to start of chain and keep stepping
                        currentChainPos = 0;
                        }    
                    }
                }
            
            // here's a trick...
            // if we got part-way through the chain before running out of steps
            // count this as a fractional hit.
            //
            // This simiplifies the math dramatically, since we aren't
            // effectively creating a derived random variable using the floor
            // function.
            hitCount += currentChainPos / (double) chainLength;

            totalHitsForAllTrials += hitCount;
            totalHitsSquaredForAllTrials += hitCount * hitCount;
            }
    
        double computedExpectedValue = 
            totalHitsForAllTrials / (double)numTrials;
    
        double computedVariance = 
            totalHitsSquaredForAllTrials / (double)numTrials
            - computedExpectedValue * computedExpectedValue;
            //- theoreticalExpectedValue * theoreticalExpectedValue;
        

        printf( "Computed EV = %f, Computed VAR = %f, Computed SD = %f\n", 
                computedExpectedValue, computedVariance, 
                sqrt( computedVariance ) );
        
        printf( "Theoretical EV = %f, Theoretical VAR = %f, "
                "Theoretical SD = %f\n", 
                theoreticalExpectedValue, theoreticalVariance,
                sqrt( theoreticalVariance ) );
        }
    

    return 0;
    }
