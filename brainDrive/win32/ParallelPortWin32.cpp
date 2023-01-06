/*
 * Modification History
 *
 * 2001-November-6   Jason Rohrer
 * Created.
 */



/**
 * Win32 implemenation of ParallelPort.
 */


#include "minorGems/temp/brainDrive/ParallelPort.h"

#include <stdio.h>
#include <conio.h>


void ParallelPort::setByte( char inByte ) {
	_outp( 0x378, inByte );	
	}



