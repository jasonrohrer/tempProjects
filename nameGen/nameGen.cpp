/*
 * Modification History
 *
 * 2002-November-26   Jason Rohrer
 * Created.
 *
 * 2002-November-28   Jason Rohrer
 * Moved code for weighting transitions into a function call.
 * Added training from a corpus.  Seems to work.
 *
 * 2003-March-12   Jason Rohrer
 * Added support for specifying a training corpus on the command line.
 */


#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <ctype.h>

#include "minorGems/util/SimpleVector.h"
#include "minorGems/util/random/StdRandomSource.h"
#include "minorGems/util/random/RandomSource.h"



// prototypes
void usage( char *inAppName );

void normalizeRow( double *inRow, int inLength );

int pickTransition( double *inWeights, int inLength,
                    RandomSource *inRandSource );

int printRow( double *inRow, int inLength );


void weightTransitions( double *inStartStateTransitions,
                        double **inMatrix,
                        SimpleVector<long> *inTransitions,
                        int inNumFormants,
                        double inChangeFactor );

int findCharacterIndex( SimpleVector<char*> *inFormants,
                        char inCharacter );

void train( double *inStartStateTransitions,
            double **inMatrix,
            SimpleVector<char*> *inFormants,
            char *inNameFile,
            double inLearningRate );



int main( char inNumArgs, char **inArgs ) {
    if( inNumArgs != 4 && inNumArgs != 5 ) {
        usage( inArgs[0] );
        }

    FILE *vowelFile = fopen( inArgs[1], "r" );
    FILE *consonantFile = fopen( inArgs[2], "r" );

    if( vowelFile == NULL || consonantFile == NULL ) {
        printf( "Failed to open configuration file(s)\n" );
        usage( inArgs[0] );
        }

    int numNames;
    int numRead = sscanf( inArgs[3], "%d", &numNames );

    if( numRead != 1 ) {
        usage( inArgs[0] );
        }

    
    
    SimpleVector<char*> *vowels = new SimpleVector<char*>();
    SimpleVector<char*> *consonants = new SimpleVector<char*>();
    

    numRead = 1;

    while( numRead == 1 ) {
        char *buffer = new char[10];
        
        numRead = fscanf( vowelFile, "%9s", buffer );

        if( numRead == 1 ) {
            vowels->push_back( buffer );
            }
        else {
            delete [] buffer;
            }
        }

    numRead = 1;
    while( numRead == 1 ) {
        char *buffer = new char[10];
        
        numRead = fscanf( consonantFile, "%9s", buffer );

        if( numRead == 1 ) {
            consonants->push_back( buffer );
            }
        else {
            delete [] buffer;
            }
        }
    
    fclose( vowelFile );
    fclose( consonantFile );

    int i, j, k;


    
    SimpleVector<char *> *allFormants = new SimpleVector<char*>();
    for( i=0; i<vowels->size(); i++ ) {
        allFormants->push_back( *( vowels->getElement(i) ) );
        }

    for( i=0; i<consonants->size(); i++ ) {
        allFormants->push_back( *( consonants->getElement(i) ) );
        }
    
    int vowelStartIndex = 0;
    int vowelEndIndex = vowels->size();
    int consonantStartIndex = vowels->size();
    int consonantEndIndex = allFormants->size();

    // one more formant... the end of the name
    allFormants->push_back( "|" );
    
    int numFormants = allFormants->size();
    

    double *startStateTransitions = new double[ numFormants ];
    
    double **markovTransitions = new double*[ numFormants ];
    for( i=0; i<numFormants; i++ ) {
        startStateTransitions[i] = 1.0 / numFormants; 
        
        markovTransitions[i] = new double[ numFormants ];
        for( j=0; j<numFormants; j++ ) {
            markovTransitions[i][j] = 1.0 / numFormants; 
            }
        }

    for( i=0; i<numFormants; i++ ) {
        //printRow( markovTransitions[i], numFormants );
        }

    // transition from start->end is 0
    startStateTransitions[ numFormants - 1 ] = 0;
    normalizeRow( startStateTransitions, numFormants );


    if( inNumArgs == 5 ) {
        // train the transitions
        train( startStateTransitions,
               markovTransitions,
               allFormants,
               inArgs[4],
               1.04 );
               //1.1 );
        }
    else {
    
        // transitions from vowel->vowel are 0
        for( i=vowelStartIndex; i<vowelEndIndex; i++ ) {
            for( j=vowelStartIndex; j<vowelEndIndex; j++ ) {
                markovTransitions[i][j] = 0;
                }
            normalizeRow( markovTransitions[i], numFormants );
            }
        // transitions from consonant->consonant are 0
        for( i=consonantStartIndex; i<consonantEndIndex; i++ ) {
            for( j=consonantStartIndex; j<consonantEndIndex; j++ ) {
                markovTransitions[i][j] = 0;
                }
            normalizeRow( markovTransitions[i], numFormants );
            }
        }
    
    for( i=0; i<numFormants; i++ ) {
        printRow( markovTransitions[i], numFormants );
        }
    
    
    StdRandomSource *randSource = new StdRandomSource();
    

    char interactive = true;
    
    for( i=0; i<numNames; i++ ) {
        if( interactive ) {
            printf( "\n\n" );
            }
        else {
            printf( "\n" );
            }
        
        int maxNameLength = randSource->getRandomBoundedInt( 5, 10 );
        /*
        float randVal = randSource->getRandomFloat();
        
        char currentConsonant = false;

        if( randVal > 0.5 ) {
            currentConsonant = true;
            }
        */
        long currentIndex = pickTransition( startStateTransitions,
                                             numFormants, randSource );

        SimpleVector<long> *indexList = new SimpleVector<long>();

        int totalNameLength = 0;
        
        while( currentIndex != numFormants - 1 &&
               totalNameLength < maxNameLength) { 
            totalNameLength++;

            indexList->push_back( currentIndex );
            
            printf( "%s", *( allFormants->getElement( currentIndex ) ) );
            
            currentIndex = pickTransition( markovTransitions[ currentIndex ],
                                           numFormants, randSource );
            /*
            if( currentConsonant ) {
                int index = randSource->getRandomBoundedInt(
                    0,
                    consonants->size() - 1 );

                printf( "%s", *( consonants->getElement( index ) ) );
                }
            else {
                int index = randSource->getRandomBoundedInt(
                    0,
                    vowels->size() - 1 );

                printf( "%s", *( vowels->getElement( index ) ) );
                }
            currentConsonant = !currentConsonant;
            */
            }

        if( interactive ) {
            printf( "\t\t\tDo you like it? [y/n/q]\n" );
            char *buffer = new char[5];
            numRead = scanf( "%4s", buffer );

            char likeIt = false;
            if( numRead == 1 ) {
                if( strstr( buffer, "y" ) != NULL ||
                    strstr( buffer, "Y" ) != NULL ) {
                    likeIt = true;
                    }
                if( strstr( buffer, "q" ) != NULL ||
                    strstr( buffer, "Q" ) != NULL ) {
                    interactive = false;;
                    }
                }

            if( interactive ) {
                
                double changeFactor;
                
                if( !likeIt ) {
                    changeFactor = 0.8;
                    }
                else {
                    changeFactor = 1.2;
                    }
                
                // modify probabilities of each transition by change factor
                
                weightTransitions( startStateTransitions,
                                   markovTransitions,
                                   indexList,
                                   numFormants,
                                   changeFactor );
                }
            
            
            }
        delete indexList;
        
        }
    

    delete randSource;


    /*
      // temp fix:  don't delete to avoid crash of unknown cause

    for( i=0; i<vowels->size(); i++ ) {
        delete [] ( *( vowels->getElement(i) ) );
        }
    delete vowels;

    for( i=0; i<consonants->size(); i++ ) {
        delete [] ( *( consonants->getElement(i) ) );
        }
    delete consonants;
    */
    return 0;
    }



void usage( char *inAppName ) {
    printf( "Usage:\n" );
    printf( "    %s vowel_file consonant_file num_names [training_file]\n",
            inAppName );
    printf( "Examples:\n" );
    printf( "    %s vowels.txt consonants.txt 10\n", inAppName );
    printf( "    %s vowels.txt consonants.txt 10 boyNames.txt\n", inAppName );

    exit( 0 );
    }



void normalizeRow( double *inRow, int inLength) {

    double rowSum = 0;
    int i;
    for( i=0; i<inLength; i++ ) {
        rowSum += inRow[i];
        }
    for( i=0; i<inLength; i++ ) {
        inRow[i] = inRow[i] / rowSum;
        }
    
    }



int pickTransition( double *inWeights, int inLength,
                    RandomSource *inRandSource ) {

    double randVal = inRandSource->getRandomDouble();

    double weightSum = 0;
    for( int i=0; i<inLength; i++ ) {
        weightSum += inWeights[i];

        if( weightSum > randVal ) {
            return i;
            }
        }

    return inLength - 1;    
    }


int printRow( double *inRow, int inLength ) {
    for( int i=0; i<inLength; i++ ) {
        printf( "%f ", inRow[i] );
        }
    printf( "\n" );
    }



void weightTransitions( double *inStartStateTransitions,
                        double **inMatrix,
                        SimpleVector<long> *inTransitions,
                        int inNumFormants,
                        double inChangeFactor ) {

    // first change start transition
    int currentIndex = *( inTransitions->getElement( 0 ) );

    inStartStateTransitions[ currentIndex ] =
        inChangeFactor * inStartStateTransitions[ currentIndex ];
    
    normalizeRow( inStartStateTransitions, inNumFormants );
    
    for( int k=0; k<inTransitions->size() - 1; k++ ) {
        currentIndex = *( inTransitions->getElement( k ) );
        int nextIndex = *( inTransitions->getElement( k+1 ) );
        
        inMatrix[ currentIndex ][ nextIndex ] =
            inChangeFactor *
            inMatrix[ currentIndex ][ nextIndex ];
        
        normalizeRow( inMatrix[ currentIndex ], inNumFormants );
        }
    }


void train( double *inStartStateTransitions,
            double **inMatrix,
            SimpleVector<char*> *inFormants,
            char *inNameFile,
            double inLearningRate ) {

    FILE *nameFile = fopen( inNameFile, "r" );

    if( nameFile == NULL ) {
        printf( "Failed to open name file:  %s\n", inNameFile );
        return;
        }

    char *buffer = new char[ 10 ];
    
    int numRead = fscanf( nameFile, "%9s", buffer );

    int numFormants = inFormants->size();
    
    while( numRead == 1 ) {
        int nameLength = strlen( buffer );

        SimpleVector<long> *transitionIndices = new SimpleVector<long>();

        for( int i=0; i<nameLength; i++ ) {
            char lowerChar = (char)( tolower( buffer[i] ) );
            
            int formantIndex = findCharacterIndex( inFormants,
                                                   lowerChar );

            transitionIndices->push_back( formantIndex );
            }
        
        // now push the end of string transition
        transitionIndices->push_back( numFormants );

        // train this as a yes instance
        weightTransitions( inStartStateTransitions,
                           inMatrix,
                           transitionIndices,
                           numFormants,
                           inLearningRate );

        
        numRead = fscanf( nameFile, "%9s", buffer );
        }

    }


int findCharacterIndex( SimpleVector<char*> *inFormants,
                        char inCharacter ) {

    char *characterString = new char[2];
    sprintf( characterString, "%c", inCharacter );
    
    
    int numFormants = inFormants->size();

    for( int i=0; i<numFormants; i++ ) {
        char *formant = *( inFormants->getElement( i ) );

        if( strcmp( characterString, formant ) == 0 ) {
            delete [] characterString;
            return i;
            }
        }

    delete [] characterString;

    // return end of string index
    return numFormants + 1;    
    }
