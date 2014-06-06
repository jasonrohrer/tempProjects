

// generates a 6x6 magic square into a newly-allocated length 36 array
// using inSeed
int *generateMagicSquare6( unsigned int inSeed );


// generates inNumSquares squares, using inSeed as
// the random seed starting point for the first square
// Subsequent squares use sequential seeds after inSeed
// (inSeed+1, inSeed+2, etc.)
// returns a newly-allocated array of newly-allocated length-36 arrays
int **generateMagicSquare6( unsigned int inSeed, int inNumSquares );
