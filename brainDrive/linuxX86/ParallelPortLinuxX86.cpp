/*
 * Modification History
 *
 * 2001-November-8   Jason Rohrer
 * Created.
 */



/**
 * X86 Linux implemenation of ParallelPort.
 */


#include "minorGems/temp/brainDrive/ParallelPort.h"

#include <stdio.h>
#include <stdlib.h>
//#include <unistd.h>
#include <sys/io.h>

#define base 0x378  //printer port base address

char checkPortAllowed = true;



void ParallelPort::setByte( char inByte ) {
	if( checkPortAllowed && ioperm( base, 1, 1 ) ) {
		printf( "Could not write to the parallel port at address %x\n", base );
		exit(1);
		}
	else {
		outb( inByte, base );
		
		// if we were allowed once, assume we will always be allowed
		checkPortAllowed = false;
		}
	}



