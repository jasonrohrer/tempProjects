#include <stdio.h>

int main() {

    int numFunctions = 4000;
    int numLoops = 10;
    int i, j;
    
    for( i=0; i<numFunctions; i++ ) {
        printf( "int function_%i( int inX ) { return inX + %i; }\n\n",
                i, i );
        }
    
    printf( "int main() {\n\n" );

    printf( "int x = 0;\n\n" );

    for( j=0; j<numLoops; j++ ) {
        for( i=0; i<numFunctions; i++ ) {
            printf( "x += function_%i( x );\n\n", i );
            }
        }
    
    printf( "return 0;\n\n}\n" );
    

    return 0;
    }
