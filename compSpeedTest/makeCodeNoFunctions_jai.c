#include <stdio.h>
#include <stdlib.h>

int main() {

    int numLoops = 160000;
    int j;

    printf( "#import \"Basic\";\n\n" );
    
    
    printf( "main :: () {\n\n" );

    printf( "x : int = 0;\n\n" );

    srand( 420 );
    
    for( j=0; j<numLoops; j++ ) {
        printf( "x = x + %d;\n\n", (int)( 10 * (float)rand() / RAND_MAX ) );
        }
    
    printf( "print( \"Result = %%\\n\", x );" );
    printf( "\n\n}\n" );
    

    return 0;
    }
