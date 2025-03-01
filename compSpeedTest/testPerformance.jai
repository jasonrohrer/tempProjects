#import "Basic";


funcA :: ( i:int, j:int ) -> int {
	return 2 * i - j;
}
funcB :: ( i:int, j:int ) -> int {
	return 3 * i - j;
}

main :: () {


	x : int = 0;

	iLimit : int = 1000;
	jLimit : int = 1000;
	
	kLimit : int = 1000;


	
	for k: 1..kLimit {
	
		for i: 1..iLimit {
			for j: 1..jLimit {
				x += funcA( i, j );
			}
		}
		for i: 1..iLimit {
			for j: 1..jLimit {
				x -= funcB( i, j );
			}
		}
	}
	print( "Result = %\n", x );
}
