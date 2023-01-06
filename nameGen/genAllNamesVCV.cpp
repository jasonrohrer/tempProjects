/*
 * Modification History
 *
 * 2007-December-12   Jason Rohrer
 * Created.
 * Copied from genAllNames.cpp
 */


#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <ctype.h>

#include "minorGems/util/SimpleVector.h"



// prototypes
void usage( char *inAppName );



int main( char inNumArgs, char **inArgs ) {
    if( inNumArgs != 3 ) {
        usage( inArgs[0] );
        }
    
    FILE *vowelFile = fopen( inArgs[1], "r" );
    FILE *consonantFile = fopen( inArgs[2], "r" );

    if( vowelFile == NULL || consonantFile == NULL ) {
        printf( "Failed to open configuration file(s)\n" );
        usage( inArgs[0] );
        }



    
    
    SimpleVector<char*> *vowels = new SimpleVector<char*>();
    SimpleVector<char*> *consonants = new SimpleVector<char*>();
    

    int numRead = 1;

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


    int numVowels = vowels->size();
    int numConsonants = consonants->size();

    for( int i=0; i<numVowels; i++ ) {

        for( int j=0; j<numConsonants; j++ ) {

            for( int k=0; k<numVowels; k++ ) {

                printf( "%s%s%s\n", *( vowels->getElement( i ) ),
                        *( consonants->getElement( j ) ),
                        *( vowels->getElement( k ) ) );
                        
                
                }
            }
        }
    
    
    }


void usage( char *inAppName ) {
    printf( "Usage:\n" );
    printf( "    %s vowel_file consonant_file\n",
            inAppName );
    printf( "Example:\n" );
    printf( "    %s vowels.txt consonants.txt\n", inAppName );

    exit( 0 );
    }
