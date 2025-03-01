#include <stdio.h>

int main() {

    int numFunctions = 10;
    int numLoops = 4000;
    int i, j;

    printf( "#import \"Basic\";\n\n" );
    
    
    for( i=0; i<numFunctions; i++ ) {
        printf( "function_%i :: ( inX : int ) -> int { return inX + %i; }\n\n",
                i, i );
        }
    
    printf( "main :: () {\n\n" );

    printf( "x : int = 0;\n\n" );

    for( j=0; j<numLoops; j++ ) {
        for( i=0; i<numFunctions; i++ ) {
            printf( "x = function_%i( x );\n\n", i );
            }
        }
    
    printf( "print( \"Result = %%\\n\", x );" );
    printf( "\n\n}\n" );
    

    return 0;
    }
