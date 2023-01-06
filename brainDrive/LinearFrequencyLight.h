/*
 * Modification History
 *
 * 2001-November-7   Jason Rohrer
 * Created.
 * Fixed several bugs in the interpolation code.
 * Fixed an inconsistent frequency bug.
 * Fixed a bug in computing the time of the next state change.
 * It has been successfully tested with an LED.
 *
 * 2001-November-8   Jason Rohrer
 * Added support for inverting the light output.
 */


#ifndef LINEAR_FREQUENCY_LIGHT_INCLUDED
#define LINEAR_FREQUENCY_LIGHT_INCLUDED


#include "Light.h"

#include <math.h>

#ifndef M_PI
#define M_PI 3.14158265359
#endif



#include <stdio.h>

/**
 * Implementation of Light interface that
 * performs linear interpolation between a set of
 * fixed frequency control points.
 *
 * @author Jason Rohrer
 */
class LinearFrequencyLight : public Light {

	public:


		/**
		 * Constructs a linear frequency light.
		 *
		 * The time at each control point must be
		 * strictly increasing as the index increases.
		 *
		 * @param inNumPoints the number of control points.
		 *   Must be at least 2.
  		 * @param inSeconds an array of times in seconds,
		 *   one for each control point.
		 *   Will be destroyed when this class is destroyed.
		 * @param inMilliseconds an array of extra milliseconds
		 *   for each control point.
		 *   Will be destroyed when this class is destroyed.
		 * @param inFrequency an array of light flash frequencies
		 *   for each control point.
		 *   Will be destroyed when this class is destroyed.
		 * @param inInverted true iff this light should
		 *   invert its pulses.  Defaults to false.
		 *   Useful for creating two lights controllers that
		 *   are identical to eachother except for an inversion
		 *   of their light output.
		 */
		LinearFrequencyLight( int inNumPoints,
							  unsigned long *inSeconds,
							  unsigned long *inMilliseconds,
							  double *inFrequency,
							  char inInverted = false);


		
		virtual ~LinearFrequencyLight();



		/**
		 * Sets a start offset time for this light.
		 *
		 * All times in the control set will adjusted
		 * relative to this time so that the
		 * first point in the set equals this time
		 * and all intervals between points remain the same length.
		 * Thus, the control set is essentially start-time
		 * independent.
		 *
		 * @param inSeconds the offset in seconds.
		 * @param inMilliseconds the extra milliseconds of the offset.
		 */
		virtual void setStartOffset( unsigned long inSeconds,
									 unsigned long inMilliseconds );


		
		// implements isLightOn() from Light interface
		virtual char isLightOn( unsigned long inSeconds,
								unsigned long inMilliseconds ); 
		

	protected:

		int mNumPoints;
		unsigned long *mSeconds;
		unsigned long *mMilliseconds;
		double *mFrequency;

		char mCurrentLightState;

		// the time of the next state change
		unsigned long mSecondsNextStateChange;
		unsigned long mMillisecondsNextStateChange;

		
	};



inline LinearFrequencyLight::LinearFrequencyLight(
	int inNumPoints,
	unsigned long *inSeconds,
	unsigned long *inMilliseconds,
	double *inFrequency,
	char inInverted )
	: mNumPoints( inNumPoints ),
	  mSeconds( inSeconds ),
	  mMilliseconds( inMilliseconds ),
	  mFrequency( inFrequency ),
	  mCurrentLightState( inInverted ) {


	// change state at our first control point
	mSecondsNextStateChange = mSeconds[0];
	mMillisecondsNextStateChange = mMilliseconds[0];
	}



inline LinearFrequencyLight::~LinearFrequencyLight() {

	delete [] mSeconds;
	delete [] mMilliseconds;
	delete [] mFrequency;
	}



inline void LinearFrequencyLight::setStartOffset(
	unsigned long inSeconds,
	unsigned long inMilliseconds ) {

	// first, subtract the current starting point
	// from all points
	for( int i=1; i<mNumPoints; i++ ) {
		mSeconds[i] -= mSeconds[0];
		mMilliseconds[i] -= mMilliseconds[0];

		// borrow
		if( mMilliseconds[i] < 0 ) {
			mSeconds[i]--;
			mMilliseconds[i] += 1000;
			}
		}
	mSeconds[0] = 0;
	mMilliseconds[0] = 0;

	// now add the offset
	for( int i=0; i<mNumPoints; i++ ) {
		mSeconds[i] += inSeconds;
		mMilliseconds[i] += inMilliseconds;

		// carry
		if( mMilliseconds[i] >= 1000 ) {
			mSeconds[i]++;
			mMilliseconds[i] -= 1000;
			}
		}

	
	// change state at our first control point
	mSecondsNextStateChange = mSeconds[0];
	mMillisecondsNextStateChange = mMilliseconds[0];
	}



inline char LinearFrequencyLight::isLightOn( unsigned long inSeconds,
											 unsigned long inMilliseconds ) {

	// compute our current frequency
	double currentFreq = 1;
	
	// look for two surrounding control points

	int leftPointIndex = -1;
	unsigned long secSinceLeftPoint = 0;
	unsigned long msecSinceLeftPoint = 0;
		
	int rightPointIndex = mNumPoints;
	unsigned long secUntilRightPoint = 0;
	unsigned long msecUntilRightPoint = 0;

		
		
	for( int i=0; i<mNumPoints; i++ ) {

		long secSincePoint = inSeconds - mSeconds[i];
		long msecSincePoint = inMilliseconds - mMilliseconds[i];

		// borrow
		if( msecSincePoint < 0 ) {
			secSincePoint--;
			msecSincePoint += 1000;
			}

		// if point is to our right (or equal to us) and it is closer
		// than the last right-most point we've seen
		if( ( secSincePoint < 0 ||
			  ( secSincePoint == 0 && msecSincePoint == 0 ) )
			&& i < rightPointIndex ) {

			rightPointIndex = i;

			// reverse borrow
			if( msecSincePoint > 0 ) {
				secUntilRightPoint = -( secSincePoint + 1 );
				msecUntilRightPoint = -( msecSincePoint - 1000 );
				}
			else {
				secUntilRightPoint = -( secSincePoint );
				msecUntilRightPoint = 0;
				}
			}

		// if point is to our left (or equal to us) and it is closer
		// than the last last-most point we've seen
		if( secSincePoint >= 0 && i > leftPointIndex ) {
			
			leftPointIndex = i;
				
			secSinceLeftPoint = secSincePoint;
			msecSinceLeftPoint = msecSincePoint;
			}
		}

	
	// if left and right points are identical, than we've hit
	// one point on the nose
	if( leftPointIndex == rightPointIndex ) {
		currentFreq = mFrequency[ leftPointIndex ];
		}
	else if( leftPointIndex == -1 ) {
		// there is no point to the left of us
		// use the left endpoint as our freq
		currentFreq = mFrequency[ 0 ];
		}
	else if( rightPointIndex == mNumPoints ) {
		// there is no point to the right of us
		// use the right endpoint as our freq
		currentFreq = mFrequency[ mNumPoints - 1 ];
		}
	else {
		// we're between two points.
		// interpolate

		long intervalLengthMS =
			1000 * ( secSinceLeftPoint
					 + secUntilRightPoint )
			+ msecSinceLeftPoint
			+ msecUntilRightPoint;

		
		double fractionRightPoint =
			(double)( 1000 * secSinceLeftPoint + msecSinceLeftPoint ) /
			(double)( intervalLengthMS );


		// linear interpolation of two frequencies
		currentFreq =
			fractionRightPoint * mFrequency[ rightPointIndex ] +
			( 1 - fractionRightPoint ) * mFrequency[ leftPointIndex ];
			
		}

	// now we have our current frequency
	
	// use it to compute a thresholded sinusoid (square wave)
	// based on the absolute time value passed in

	
	// compute sin( t * 2 * pi * currentFrequency )

	
	/*		
	double sinVal =
		sin( ( inSeconds + (double) inMilliseconds / 1000.0 )
			 * 2 * M_PI * currentFreq );

	
	// threshold at 0
	if( sinVal >= 0 ) {
		return true;
		}
	else {
		return false;
		}
	*/

	
	// problem:
	// currentFrequency is acutally a linear function of t,
	// so in effect we have
	// sin( ... * t^2 * ... ),
	// and the frequency of this function can be higher
	// than currentFrequency, which is not good.
	// in fact, this function looks not much like
	// a sine wave increasing in frequency linearly (since it isn't)

	// We need to instead produce a wave that has piecewise
	// constant frequency.
	// we can do this by tracking the time between state changes,
	// which are half-wavelengths

	long secondsUntilNextStateChange =
		mSecondsNextStateChange - inSeconds;
	long millisecondsUntilNextStateChange =
		mMillisecondsNextStateChange - inMilliseconds;

	// borrow
	if( millisecondsUntilNextStateChange < 0 ) {
		millisecondsUntilNextStateChange += 1000;
		secondsUntilNextStateChange--;
		}

	if( secondsUntilNextStateChange < 0
		|| ( secondsUntilNextStateChange == 0
			 && millisecondsUntilNextStateChange == 0 ) ) {

		// time to change state
		mCurrentLightState = !mCurrentLightState;
		
		double wavelength = 1.0 / currentFreq;

		// next state change is half a wavelength away
		double halfWavelength = 0.5 * wavelength;

		mSecondsNextStateChange =
			(int)( halfWavelength ) + inSeconds;

		mMillisecondsNextStateChange =
			(int)( 1000 * ( halfWavelength - (int)( halfWavelength ) ) )
			+ inMilliseconds;

		// carry
		if( mMillisecondsNextStateChange >= 1000 ) {
			mSecondsNextStateChange++;
			mMillisecondsNextStateChange -= 1000;
			}
		
		}
	

	return mCurrentLightState;
	}


#endif
