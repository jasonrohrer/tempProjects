/*
 * Modification History
 *
 * 2002-May-5   Jason Rohrer
 * Created.
 */



#include "WebClient.h"

#include <stdio.h>


int main() {

    char *buffer = new char[200];

    
    
    while( strcmp( "q", buffer ) != 0 ) {
        printf( "Enter URL, or q to quit:\n" );
    
    
        int numRead = scanf( "%s", buffer );
        
        if( numRead == 1 && strcmp( "q", buffer ) != 0 ) {

            printf( "Fetching %s\n", buffer );
            
            char *returnString = WebClient::getWebPage( buffer );

            if( returnString != NULL ) {
                printf( "%s\n", returnString );
                delete [] returnString;
                }
            else {
                printf( "Request failed.\n" );
                }
            }
        }

    delete [] buffer;

    return 0;
    }


