/*
 * Modification History
 *
 * 2001-November-7   Jason Rohrer
 * Created.
 */



#ifndef LIGHT_INCLUDED
#define LIGHT_INCLUDED



/**
 * Interface for a light controler.
 *
 * @author Jason Rohrer
 */
class Light {

	public:

		

		/**
		 * Gets whether the light is on at a given time.
		 *
		 * @param inSeconds the time in seconds.
		 * @param inMilliseconds the extra milliseconds in the time.
		 */
		virtual char isLightOn( unsigned long inSeconds,
								unsigned long inMilliseconds ) = 0; 


		
		/**
		 * Virtual destructor so that memory deallocation
		 * happens correctly for subclasses.
		 */
		virtual ~Light();


		
	};




inline Light::~Light() {
	// does nothing
	}


#endif
