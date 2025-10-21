
/* #include <stdio.h> */

/* turns out that stb_vorbis.c isn't even c89 compatible */
/* neither is libvorbis (not even if code is directly included,
   because it depends on stdint at least */

/* proving that stdarg doesn't require stdlib to link it */
#include <stdarg.h>

int sum_integers(int count, ...);


int sum_integers(int count, ...) {
    va_list ap;
    int sum = 0;
    int i;

    va_start(ap, count);

    for (i = 0; i < count; i++) {
        
        sum += va_arg(ap, int);
    }

    va_end(ap);

    return sum;
}


/* proving that minivorbis doesn't require stdlib to link it */
/* nope */
/* #include "minivorbis.h" */


int main( void ) {
    int result;

    result = sum_integers( 3, 1, 2, 3 );

    /* printf( "Hey\n" ); */
    return result;
    }
