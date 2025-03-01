#include <stdio.h>


long long int funcA( long long int i, long long int j ) {
    return 2 * i - j;
    }

long long int funcB( long long int i, long long int j ) {
    return 3 * i - j;
    }

int main() {


    long long int x = 0;
    
	long long int iLimit = 1000;
	long long int jLimit = 1000;
	
	long long int kLimit = 1000;

    int k, i, j;

	
	for( k=1; k<=kLimit; k++ ) {
	
		for( i=1; i<=iLimit; i++ ) {
			for( j=1; j<jLimit; j++ ) {
				x += funcA( i, j );
			}
		}
		for( i=1; i<=iLimit; i++ ) {
			for( j=1; j<jLimit; j++ ) {
				x -= funcB( i, j );
			}
		}
	}
	printf( "Result = %lld\n", x );

    return 0;
}
