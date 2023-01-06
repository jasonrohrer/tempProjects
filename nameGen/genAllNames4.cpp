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



    
    
    SimpleVector<char*> *allLetters = new SimpleVector<char*>();
        

    int numRead = 1;

    while( numRead == 1 ) {
        char *buffer = new char[10];
        
        numRead = fscanf( vowelFile, "%9s", buffer );

        if( numRead == 1 ) {
            allLetters->push_back( buffer );
            }
        else {
            delete [] buffer;
            }
        }

    int numVowels = allLetters->size();
    

    numRead = 1;
    while( numRead == 1 ) {
        char *buffer = new char[10];
        
        numRead = fscanf( consonantFile, "%9s", buffer );

        if( numRead == 1 ) {
            allLetters->push_back( buffer );
            }
        else {
            delete [] buffer;
            }
        }
    
    int numConsonants = allLetters->size() - numVowels;

    fclose( vowelFile );
    fclose( consonantFile );


    int numLetters = allLetters->size();
    
    printf( "num letters = %d\n", numLetters );
    
    for( int i=0; i<numLetters; i++ ) {

        for( int j=0; j<numLetters; j++ ) {

            for( int k=0; k<numLetters; k++ ) {
                
                for( int m=0; m<numLetters; m++ ) {
                    
                    int vowelCount = 0;
                    if( i < numVowels ) vowelCount++;
                    if( j < numVowels ) vowelCount++;
                    if( k < numVowels ) vowelCount++;
                    if( m < numVowels ) vowelCount++;
                    
                    if( vowelCount == 2 &&
                        ( k < numVowels || m < numVowels ) &&
                        ( i < numVowels || j < numVowels ) &&
                        i != j &&
                        j != k &&
                        k != m ) {
                        
                        printf( "%s%s%s%s\n", 
                                *( allLetters->getElement( i ) ),
                                *( allLetters->getElement( j ) ),
                                *( allLetters->getElement( k ) ),
                                *( allLetters->getElement( m ) ) );
                        }
                    }
                
                
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
