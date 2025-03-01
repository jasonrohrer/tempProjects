#include <stdio.h>

int main() {

    int numFunctions = 10;
    int numLoops = 16000;
    int i, j;

    printf( "#include <stdio.h>\n" );
    
    
    for( i=0; i<numFunctions; i++ ) {
        printf( "long long int function_%i( long long int inX ) { return inX + %i; }\n\n",
                i, i );
        }
    
    printf( "int main() {\n\n" );

    printf( "long long int x = 0;\n\n" );

    for( j=0; j<numLoops; j++ ) {
        for( i=0; i<numFunctions; i++ ) {
            printf( "x = function_%i( x );\n\n", i );
            }
        }
    
    printf( "printf( \"Result = %%lld\\n\", x );\n\n}\n" );
    

    return 0;
    }
