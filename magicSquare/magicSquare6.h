

// generates a 6x6 magic square into a newly-allocated length 36 array
// uses the current system time as the random seed
int *generateMagicSquare6();


// generates inNumSquares squares, using the current system time as
// the random seed starting point for the first square
// returns a newly-allocated array of newly-allocated length-36 arrays
int **generateMagicSquare6( int inNumSquares );
