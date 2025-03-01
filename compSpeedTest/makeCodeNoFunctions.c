#include <stdio.h>
#include <stdlib.h>

int main() {

    int numLoops = 160000;
    int j;

    printf( "#include <stdio.h>\n" );
    
   
    printf( "int main() {\n\n" );

    printf( "long long int x = 0;\n\n" );

    srand( 420 );
    
    for( j=0; j<numLoops; j++ ) {
        printf( "x = x + %d;\n\n", (int)( 10 * (float)rand() / RAND_MAX ) );
        }
    
    printf( "printf( \"Result = %%lld\\n\", x );\n\n}\n" );
    

    return 0;
    }
