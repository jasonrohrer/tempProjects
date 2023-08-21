/* Use the newer ALSA API */
#define ALSA_PCM_NEW_HW_PARAMS_API

#include <alsa/asoundlib.h>



#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <stdbool.h>
#include <stdio.h>
#include <math.h>
#include <stdint.h>


static int bufferSize = 64;


static double rateMultiplier = 0.003;
static double minRate = 0.75;
static double maxRate = 1.25;



int main() {
    Display* dpy = XOpenDisplay(NULL);
    char keys_return[32];
    
    
    int val;

    printf("ALSA library version: %s\n",
           SND_LIB_VERSION_STR);

    printf("\nPCM stream types:\n");
    for (val = 0; val <= SND_PCM_STREAM_LAST; val++)
        printf("  %s\n",
               snd_pcm_stream_name((snd_pcm_stream_t)val));

    printf("\nPCM access types:\n");
    for (val = 0; val <= SND_PCM_ACCESS_LAST; val++)
        printf("  %s\n",
               snd_pcm_access_name((snd_pcm_access_t)val));

    printf("\nPCM formats:\n");
    for (val = 0; val <= SND_PCM_FORMAT_LAST; val++)
        if (snd_pcm_format_name((snd_pcm_format_t)val)
            != NULL)
            printf("  %s (%s)\n",
                   snd_pcm_format_name((snd_pcm_format_t)val),
                   snd_pcm_format_description(
                       (snd_pcm_format_t)val));

    printf("\nPCM subformats:\n");
    for (val = 0; val <= SND_PCM_SUBFORMAT_LAST;
         val++)
        printf("  %s (%s)\n",
               snd_pcm_subformat_name((
                                          snd_pcm_subformat_t)val),
               snd_pcm_subformat_description((
                                                 snd_pcm_subformat_t)val));

    printf("\nPCM states:\n");
    for (val = 0; val <= SND_PCM_STATE_LAST; val++)
        printf("  %s\n",
               snd_pcm_state_name((snd_pcm_state_t)val));



    int rc;
    int size;
    snd_pcm_t *handleOut;
    snd_pcm_hw_params_t *paramsOut;
    unsigned int valStream;
    int dir;
    snd_pcm_uframes_t frames;
    unsigned char *buffer;

    /* Open PCM device for playback. */
    rc = snd_pcm_open(&handleOut, "default",
                      SND_PCM_STREAM_PLAYBACK, 0);
    if (rc < 0) {
        fprintf(stderr,
                "unable to open pcm device: %s\n",
                snd_strerror(rc));
        exit(1);
        }

    /* Allocate a hardware parameters object. */
    snd_pcm_hw_params_alloca(&paramsOut);

    /* Fill it in with default values. */
    snd_pcm_hw_params_any(handleOut, paramsOut);

    /* Set the desired hardware parameters. */

    /* Interleaved mode */
    snd_pcm_hw_params_set_access(handleOut, paramsOut,
                                 SND_PCM_ACCESS_RW_INTERLEAVED);

    /* Signed 16-bit little-endian format */
    snd_pcm_hw_params_set_format(handleOut, paramsOut,
                                 SND_PCM_FORMAT_S16_LE);

    /* Two channels (stereo) */
    snd_pcm_hw_params_set_channels(handleOut, paramsOut, 2);

    /* 44100 bits/second sampling rate (CD quality) */
    valStream = 44100;
    snd_pcm_hw_params_set_rate_near(handleOut, paramsOut,
                                    &valStream, &dir);

    /* Set period size to 32 frames. */
    frames = bufferSize;
    snd_pcm_hw_params_set_period_size_near(handleOut,
                                           paramsOut, &frames, &dir);

    /* Write the parameters to the driver */
    rc = snd_pcm_hw_params(handleOut, paramsOut);
    if (rc < 0) {
        fprintf(stderr,
                "unable to set hw parameters: %s\n",
                snd_strerror(rc));
        exit(1);
        }

    /* Use a buffer large enough to hold one period */
    snd_pcm_hw_params_get_period_size(paramsOut, &frames,
                                      &dir);
    size = frames * 4; /* 2 bytes/sample, 2 channels */
    buffer = (unsigned char *) malloc(size);



    snd_pcm_t *handleIn;
    snd_pcm_hw_params_t *paramsIn;

    /* Open PCM device for recording (capture). */
    rc = snd_pcm_open(&handleIn, "default",
                      SND_PCM_STREAM_CAPTURE, 0);
    if (rc < 0) {
        fprintf(stderr,
                "unable to open pcm device: %s\n",
                snd_strerror(rc));
        exit(1);
        }

    /* Allocate a hardware parameters object. */
    snd_pcm_hw_params_alloca(&paramsIn);

    /* Fill it in with default values. */
    snd_pcm_hw_params_any(handleIn, paramsIn);

    /* Set the desired hardware parameters. */

    /* Interleaved mode */
    snd_pcm_hw_params_set_access(handleIn, paramsIn,
                                 SND_PCM_ACCESS_RW_INTERLEAVED);

    /* Signed 16-bit little-endian format */
    snd_pcm_hw_params_set_format(handleIn, paramsIn,
                                 SND_PCM_FORMAT_S16_LE);

    /* Two channels (stereo) */
    snd_pcm_hw_params_set_channels(handleIn, paramsIn, 2);

    /* 44100 bits/second sampling rate (CD quality) */
    valStream = 44100;
    snd_pcm_hw_params_set_rate_near(handleIn, paramsIn,
                                    &valStream, &dir);

    /* Set period size to 32 frames. */
    frames = bufferSize;
    snd_pcm_hw_params_set_period_size_near(handleIn,
                                           paramsIn, &frames, &dir);

    /* Write the parameters to the driver */
    rc = snd_pcm_hw_params(handleIn, paramsIn);
    if (rc < 0) {
        fprintf(stderr,
                "unable to set hw parameters: %s\n",
                snd_strerror(rc));
        exit(1);
        }

    /* Use a buffer large enough to hold one period */
    snd_pcm_hw_params_get_period_size(paramsIn,
                                      &frames, &dir);
    size = frames * 4; /* 2 bytes/sample, 2 channels */
    


    // 20 seconds of stereo 16-bit samples
    int storageSize = 20 * 44100 * 2 * 2;
    
    uint8_t *sampleStorage = new uint8_t[ storageSize ];

    int nextWritePos = 0;
    int nextReadPos = 0;
    

    double rate = 1.0;

    char forceNormalRate = false;
    
  
    while( true ) {
        XQueryKeymap( dpy, keys_return );
        KeyCode kc2 = XKeysymToKeycode( dpy, XK_Shift_L );
        bool bShiftPressed = !!( keys_return[ kc2>>3 ] & ( 1<<(kc2&7) ) );
        //printf("Shift is %spressed\n", bShiftPressed ? "" : "not ");
      
        rc = snd_pcm_readi(handleIn, buffer, frames);
        if (rc == -EPIPE) {
            /* EPIPE means overrun */
            fprintf(stderr, "overrun occurred\n");
            snd_pcm_prepare(handleIn);
            } else if (rc < 0) {
            fprintf(stderr,
                    "error from read: %s\n",
                    snd_strerror(rc));
            } else if (rc != (int)frames) {
            fprintf(stderr, "short read, read %d frames\n", rc);
            }
        memcpy( &( sampleStorage[nextWritePos] ),
                buffer, size );
        nextWritePos += size;

        if( bShiftPressed ) {
            rate *= 1.0 - rateMultiplier;
            //rate = 0.84088;
            if( rate < minRate ) {
                rate = minRate;
                }
            }
        /*
        else if( rate < 1.0 ) {
            rate = 1.0;
            nextReadPos = nextWritePos - size;
            }
        */
        else if( rate < 1.0 ) {
            rate *= 1.00 + rateMultiplier;
            
            if( rate >= 1.0 ) {
                // keep speeding up now until we catch up

                if( nextWritePos > nextReadPos + size ) {

                    if( rate == 1.0 ) {
                        rate *= 1.0 + rateMultiplier;
                        }
                    }
                }
            }
        else if( rate > 1.0 ) {
            
            if( nextWritePos > nextReadPos + size * rate ) {
                // keep speeding up
                rate *= 1.0 + rateMultiplier;
                if( rate > maxRate ) {
                    rate = maxRate;
                    }
                }
            else {
                // pick perfect rate for final buffer to catch up
                rate =
                    (double)( nextWritePos - nextReadPos ) / (double)size;
                forceNormalRate = true;
                }
            }
        
        if( rate != 1.0 ) {
            // walk through at half speed, and lerp samples
            
            int numFramesNeeded = size / 4;

            double sourceStepPerOutputFrame = rate;


            int farthestRead = nextReadPos;

            // for each frame in output buffer
            for( int b=0; b<numFramesNeeded; b++ ) {

                double sourcePos = b * sourceStepPerOutputFrame;
                
                int frameBelow = (int)floor( sourcePos );
                int frameAbove = (int)ceil( sourcePos );
                
                double aboveWeight = sourcePos - frameBelow;
                double belowWeight = 1.0 - aboveWeight;


                int belowBytePos = frameBelow * 4 + nextReadPos;
                int aboveBytePos = frameAbove * 4 + nextReadPos;
                
                farthestRead = aboveBytePos + 4;
                
                // little endian
                int16_t leftBelow = sampleStorage[belowBytePos] |
                    sampleStorage[belowBytePos + 1] << 8;
                int16_t rightBelow = sampleStorage[belowBytePos + 2] |
                    sampleStorage[belowBytePos + 3] << 8;
                
                int16_t leftAbove = sampleStorage[aboveBytePos] |
                    sampleStorage[aboveBytePos + 1] << 8;
                int16_t rightAbove = sampleStorage[aboveBytePos + 2] |
                    sampleStorage[aboveBytePos + 3] << 8;

                
                
                int16_t leftLerp =
                    (int16_t)lrint( leftBelow * belowWeight +
                           leftAbove * aboveWeight );
                
                int16_t rightLerp =
                    (int16_t)lrint( rightBelow * belowWeight +
                           rightAbove * aboveWeight );                
                           
                int bytePos = b * 4;
                
                buffer[ bytePos ] = (uint8_t)( leftLerp & 0xFF );
                buffer[ bytePos + 1 ] =  (uint8_t)( ( leftLerp >> 8 ) & 0xFF );
                buffer[ bytePos + 2 ] = (uint8_t)( rightLerp & 0xFF) ;
                buffer[ bytePos + 3 ] =  (uint8_t)( ( rightLerp >> 8 ) & 0xFF );
                }
            
            nextReadPos = farthestRead;
            }
        else {
            // no pitch change, simply copy directly
            memcpy( buffer, &( sampleStorage[ nextReadPos ] ), size );
            nextReadPos += size;
            }

        if( forceNormalRate ) {
            nextReadPos = nextWritePos;
            rate = 1.0;
            forceNormalRate = false;
            }
        
        
        if( nextReadPos == nextWritePos ) {
            // caught up, rewind both
            //printf( "resetting\n" );
            nextWritePos = 0;
            nextReadPos = 0;
            }

        if( nextReadPos >= storageSize ||
            nextWritePos >= storageSize ) {
            printf( "Overran storage buffer.\n" );
            nextReadPos = 0;
            nextWritePos = 0;
            }
        
        
        rc = snd_pcm_writei(handleOut, buffer, frames);
        if (rc == -EPIPE) {
            /* EPIPE means underrun */
            fprintf(stderr, "underrun occurred\n");
            snd_pcm_prepare(handleOut);
            } else if (rc < 0) {
            fprintf(stderr,
                    "error from writei: %s\n",
                    snd_strerror(rc));
            }  else if (rc != (int)frames) {
            fprintf(stderr,
                    "short write, write %d frames\n", rc);
            }

        }

    snd_pcm_drain(handleIn);
    snd_pcm_close(handleIn);
  
    snd_pcm_drain(handleOut);
    snd_pcm_close(handleOut);
    free(buffer);

    XCloseDisplay(dpy);


    delete [] sampleStorage;
    
    return 0;
    }
