#include <stdio.h>
#include <stdlib.h>

int main() {

    int numLoops = 10000;
    int numFunctions = 16;
    
    int j, f;

    srand( 420 );
    
    printf( "#import \"Basic\";\n\n" );


    for( f=0; f<numFunctions; f++ ) {
        printf( "function_%d :: () -> int {\n\n", f );
        printf( "x : int = 0;\n\n" );

        for( j=0; j<numLoops; j++ ) {
            printf( "x = x + %d;\n\n", (int)( 10 * (float)rand() / RAND_MAX ) );
            }
        printf( "return x;\n\n}\n\n" );
        }
    
    
    
    
    printf( "main :: () {\n\n" );

    printf( "x : int = 0;\n\n" );

    for( f=0; f<numFunctions; f++ ) {
        printf( "x = x + function_%d();\n\n", f );
        }
    
    
    printf( "print( \"Result = %%\\n\", x );" );
    printf( "\n\n}\n" );
    

    return 0;
    }
