

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "minorGems/util/SimpleVector.h"
#include "minorGems/util/stringUtils.h"



// prints usage message and exits
void usage( char *inAppName );


class Entry {
    public:

        Entry( double inNumber, double inPrice )
            : mNumber( inNumber ), mPrice( inPrice ) {
            }
        
        double mNumber;
        double mPrice;
    };



class Sheet {

    public:

        Sheet() {
            mEntries = new SimpleVector<Entry *>();
            }
        
        ~Sheet() {
            for( int i=0; i<mEntries->size(); i++ ) {
                delete *( mEntries->getElement( i ) );
                }
            delete mEntries;
            }
                
        SimpleVector<Entry *> *mEntries;

    };
        


int main( int inNumArgs, char **inArgs ) {
    SimpleVector<Sheet*> *sheets = new SimpleVector<Sheet*>();

    // temp char buffer for reading input strings
    char *buffer = new char[100];


    char *fileName;

    if( inNumArgs == 2 ) {
        fileName = stringDuplicate( inArgs[1] );
        }
    else if( inNumArgs == 1 ) {
        char readFileName = false;

        while( ! readFileName ) {
            printf( "Enter data file name (can be the name of a new file): " );
            fflush( stdout );
            if( scanf( "%99s", buffer ) == 1 ) {
                fileName = stringDuplicate( buffer );
                readFileName = true;
                }
            }
        }
    else {
        usage( inArgs[0] );
        delete [] buffer;
        }
                             
        
            
    // try to open for reading
    FILE *file = fopen( fileName, "r" );

    if( file != NULL ) {
        printf( "\nFile %s exists, reading in data...\n", fileName );
        
        // read existing sheets in

        char error = false;

        while( !error ) {
            // read in another sheet
            Sheet *currentSheet = new Sheet();

            while( !error ) {
                // read another entry
                double number;
                double price;

                int numRead = fscanf( file, "%lf", &number );

                numRead += fscanf( file, "%lf", &price );

                if( numRead == 2 ) {
                    Entry *entry = new Entry( number, price );

                    currentSheet->mEntries->push_back( entry );
                    }
                else {
                    error = true;
                    }
                }
            sheets->push_back( currentSheet );

            // error == true here

            // check if there is another sheet, or if we are at the true end

            

            if( fscanf( file, "%99s", buffer ) == 1 ) {
                // we were able to read the SHEET break string

                // not really an error, just a new sheet
                error = false;
                }
            }
        fclose( file );

        // re-open for writing, positioning at end
        file = fopen( fileName, "a" );

        printf( "Read %d sheets from file.\n", sheets->size() );
        
        }
    else {
        printf( "\nFile %s does not exist, starting a new file.\n", fileName );
        // open a new file
        file = fopen( fileName, "w" );
        }


    char quit = false;


    printf( "\nEnter p instead of quantity to start a new sheet.\n" );
    printf( "Enter q instead of quantity to quit and save a CSV file.\n\n" );

    Sheet *currentSheet;
    if( sheets->size() > 0 ) {
        currentSheet = *( sheets->getElement( sheets->size() - 1 ) );
        }
    else {        
        currentSheet = new Sheet();

        sheets->push_back( currentSheet );
        }
        
    while( !quit ) {

        printf( "Sheet %d\n", sheets->size() );

        
        
        
        char newSheet = false;

        while( !newSheet && !quit ) {

            char entryError = true;
            while( entryError ) {
                printf( "Enter quantity: " );

                fflush( stdout );

                double number;
                double price;
                
                int numRead = scanf( "%lf", &number );

                if( numRead == 1 ) {
                    printf( "Enter price:    " );

                    fflush( stdout );
                    numRead += scanf( "%lf", &price );
                    }
                
                if( numRead != 2 ) {

                    if( scanf( "%99s", buffer ) == 1 ) {
                        if( strcmp( "p", buffer ) == 0 ) {
                            // new page
                            newSheet = true;
                            entryError = false;
                            }
                        else if( strcmp( "q", buffer ) == 0 ) {
                            quit = true;
                            entryError = false;
                            }

                        }

                    if( !newSheet && !quit ) {
                        
                        printf( "Bad entry\n" );
                        entryError = true;
                        }
                    }
                else {
                    // good entry
                    currentSheet->mEntries->push_back(
                        new Entry( number, price ) );

                    // write into file too
                    fprintf( file, "%.2f  %.2f\n", number, price );
                    fflush( stdout );

                    printf( "    %.2f = %.2f * %.2f\n",
                            number * price, number, price );
                    
                    entryError = false;
                    }
                }
            }
        // sheet done

        if( !quit ) {
            fprintf( file, "END_SHEET_%d\n\n", sheets->size() );

            currentSheet = new Sheet();

            sheets->push_back( currentSheet );
            }
        }


    fclose( file );

    
    // quitting... but save a CSV file first


    char *csvFileName = autoSprintf( "%s.csv", fileName );

    delete [] fileName;
    
    printf( "\n\nSaving a CSV (comman separated value) file: %s\n",
            csvFileName );


    FILE *csvFile = fopen( csvFileName, "w" );
    delete [] csvFileName;
    

    char doneWithLines = false;
    int lineNumber = 0;
    
    // write values from sheets row-by-row
    // some sheets have more rows than others
    while( !doneWithLines ) {

        char someStillHaveLines = false;

        for( int i=0; i<sheets->size(); i++ ) {
            Sheet *currentSheet = *( sheets->getElement( i ) );
            
            if( currentSheet->mEntries->size() > lineNumber ) {
                // this sheet has an entry in this row
                Entry *entry =
                    *( currentSheet->mEntries->getElement( lineNumber ) );
                
                fprintf( csvFile, "=%.2f*%.2f", entry->mNumber,
                         entry->mPrice );

                someStillHaveLines = true;
                }
            else {
                // empty cell
                fprintf( csvFile, "0" );
                }

            
            if( i < sheets->size() - 1 ) {
                // comma at end of cell
                fprintf( csvFile, "," );
                }
            else {
                // end of line
                fprintf( csvFile, "\n" );
                }
            }

        if( !someStillHaveLines ) {
            // none have lines left
            doneWithLines = true;
            }
        else {
            // go on to next line
            lineNumber++;
            }
        
        }

    fclose( csvFile );
    

    delete [] buffer;

    
    printf( "\nHit Enter to exit...\n" );

    // wait for enter
    // need to read 2 chars to strip off the enter that is left
    // over from the user's quit command
    getchar();
    getchar();
    
    return 0;
    }



void usage( char *inAppName ) {

	printf( "Usage:\n" );
	printf( "\t%s file_name\n", inAppName );

	printf( "Example:\n" );
	printf( "\t%s inventory_2002.txt\n", inAppName );

    printf( "\nHit Enter to exit...\n" );

    // wait for enter
    getchar();

    
	exit( 1 );
	}

    
