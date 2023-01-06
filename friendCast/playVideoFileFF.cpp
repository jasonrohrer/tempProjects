#include "playVideoFile.h"
#include "streamNetwork.h"



#include "avcodec.h"
#include "avformat.h"

#include "minorGems/graphics/ScreenGraphics.h"
#include "minorGems/graphics/GraphicBuffer.h"
#include "minorGems/graphics/IconMap.h"

#include "minorGems/ui/Keyboard.h"


#include "minorGems/util/SimpleVector.h"

#include "minorGems/network/Socket.h"
#include "minorGems/network/SocketServer.h"

#include "minorGems/system/MutexLock.h"
#include "minorGems/system/Thread.h"
#include "minorGems/system/Time.h"


#include "minorGems/sound/SoundPlayer.h"


#include <math.h>



// the size of the actual video data frame
int frameWidth = -1;
int frameHeight = -1;

// aspect ratio may dictate that the frame be distorted for proper display

// the display width may differ from the data frame width according to
// the aspect ratio
int displayWidth = -1;
int displayHeight = -1;


int screenWidth = 400;
int screenHeight = 300;


// one static screen
ScreenGraphics screen( screenWidth, screenHeight );
unsigned long *screenData = NULL;
GraphicBuffer *graphics = NULL;


//   1 / framerate
double secondsPerFrame = 0;


SimpleVector<Socket *> clientSockets;


// set by application if we should jump out of our play loop
extern char quitFlag;

// we set this if user changes channel
extern char channelChangeFlag;


// set by DataSource_read if there has been a format change
char formatChange = false;



class FFSampleSource : public SampleSource {
        
    public:

        FFSampleSource();

        
        // virtual destructor so subclasses can override it
        virtual ~FFSampleSource() {
            }

        
        // implements the SampleSource interface
        virtual void getSamples( int inNumSamples,
                                 float *outSampleBuffer,
                                 double inPlayTime ); 

    private:
        int mIndex;
    };



FFSampleSource::FFSampleSource()
    : mIndex( 0 ) {

    }


MutexLock audioSampleBufferLock;
float *audioSampleBuffer = NULL;
int audioSampleBufferLength = 0;


// current timestamp for the audio stream
// Set this to the timestamp of the first sample in the audio sample buffer
// (in other words, the oldest yet-unused sample)
double audioTime = 0;



// this takes stereo or mono audio into account
// For stereo, this value is 1 / ( sampleRate * 2 )
double secondsPerAudioSample = 0;


// start time is time that first video frame is flipped onto screen
// or time that first audio frame is made available for play
char startTimeSet = false;
unsigned long startSeconds, startMilliseconds;



double getSystemMasterClock() {
    unsigned long millisecondsElapsed = 0;
    if( startTimeSet ) {
        millisecondsElapsed = 
            Time::getMillisecondsSince( startSeconds,
                                        startMilliseconds );
        }
    
    double systemMasterClock =
        (double)millisecondsElapsed / 1000.0;

    return systemMasterClock;
    }



void FFSampleSource::getSamples( int inNumSamples,
                                 float *outSampleBuffer,
                                 double inPlayTime ) {
    
    if( quitFlag ) {
        exit( 0 );
        }
    
    audioSampleBufferLock.lock();

    
    if( audioSampleBufferLength < inNumSamples ) {
        // not enough samples in buffer
        //printf( "Not enough samples\n" );

        // fill buffer with silence here to avoid
        // buzzing artifacts from old samples lingering in buffer.

        for( int i=0; i<inNumSamples; i++ ) {
            outSampleBuffer[i] = 0.0;
            }
        
        audioSampleBufferLock.unlock();
        return;
        }
    
    
    // copy into output buffer
    memcpy( outSampleBuffer, audioSampleBuffer,
            inNumSamples * sizeof( float ) );

    // copy remaining portion into a new buffer
    int remainingBufferLength = audioSampleBufferLength - inNumSamples;

    float *newAudioSampleBuffer = new float[ remainingBufferLength ];

    memcpy( newAudioSampleBuffer,
            // skip samples that we just used
            &( audioSampleBuffer[ inNumSamples ] ),
            remainingBufferLength * sizeof( float ) );

    // replace audioSampleBuffer with this new buffer
    delete [] audioSampleBuffer;
    audioSampleBuffer = newAudioSampleBuffer;
    audioSampleBufferLength = remainingBufferLength;

    // compute the timestamp of the first sample in our remaining buffer
    audioTime += secondsPerAudioSample * inNumSamples;
    
    //printf( "New audio time %0.3f, buffer size = %d\n", audioTime,
    //        audioSampleBufferLength );

    double systemMasterClock = getSystemMasterClock();

    double timeOffset = systemMasterClock - audioTime;
    //printf( "Audio time = %0.3f, system time = %0.3f,"
    //        "Audio time offset = %0.3f\n", audioTime, systemMasterClock,
    //        timeOffset );

    double allowedLag = secondsPerFrame;
    
    // if audio time lags too much
    if( timeOffset > allowedLag ) {
        // drop samples from buffer
        int sampleOffset = (int)( timeOffset / secondsPerAudioSample );

        if( sampleOffset > audioSampleBufferLength ) {
            sampleOffset = audioSampleBufferLength;
            }

        int remainingBufferLength = audioSampleBufferLength - sampleOffset;

        float *newAudioSampleBuffer = new float[ remainingBufferLength ];

        memcpy( newAudioSampleBuffer,
            // skip samples that we just used
            &( audioSampleBuffer[ sampleOffset ] ),
            remainingBufferLength * sizeof( float ) );

        // replace audioSampleBuffer with this new buffer
        delete [] audioSampleBuffer;
        audioSampleBuffer = newAudioSampleBuffer;
        audioSampleBufferLength = remainingBufferLength;

        // new audio time
        audioTime += secondsPerAudioSample * sampleOffset;
        }
    // if audio time too far aheadc
    else if( timeOffset < - allowedLag ) {
        // add silent samples to front of buffer
        int sampleOffset = - (int)( timeOffset / secondsPerAudioSample );

        int newBufferLength = audioSampleBufferLength + sampleOffset;

        float *newAudioSampleBuffer = new float[ newBufferLength ];

        // copy existing samples, and make room for silence
        memcpy( &( newAudioSampleBuffer[ sampleOffset ] ),
                audioSampleBuffer,
                audioSampleBufferLength * sizeof( float ) );

        // add silence
        for( int i=0; i<sampleOffset; i++ ) {
            newAudioSampleBuffer[i] = 0.0;
            }

        // replace audioSampleBuffer with this new buffer
        delete [] audioSampleBuffer;
        audioSampleBuffer = newAudioSampleBuffer;
        audioSampleBufferLength = remainingBufferLength;

        // new audio time
        audioTime -= secondsPerAudioSample * sampleOffset;        
        }

    timeOffset = systemMasterClock - audioTime;
    //printf( "After adjustment, audio time offset = %0.3f\n", timeOffset );
    
    audioSampleBufferLock.unlock();
    }




void playVideoFile( char *inFileName ) {

    executeNetworkStep( 20 );
    
    
    Keyboard keyboard;
    
    
    // set the starting state for our global variables
    formatChange = false;

    audioSampleBuffer = new float[0];
    audioSampleBufferLength = 0;
    
    audioTime = 0;
        
    
    startTimeSet = false;

    
    // much of this code copied from avcodec_sample.0.4.9.cpp
    // by Martin Bohme
    // http://www.inb.uni-luebeck.de/~boehme/libavcodec_update.html

    AVFormatContext *pFormatCtx;
    AVFormatParameters formatParameters;
    int             videoStreamIndex;
    int             audioStreamIndex;
    AVStream        *videoStream;
    AVStream        *audioStream;
    AVCodecContext  *pVideoCodecCtx;
    AVCodecContext  *pAudioCodecCtx;
    AVCodec         *pVideoCodec;
    AVCodec         *pAudioCodec;
    AVFrame         *pFrame; 
    AVFrame         *pFrameRGBA;
    AVPacket        packet;
    int             frameFinished;
    int             numBytes;
    uint8_t         *buffer;

    
    // Register all formats and codecs
    av_register_all();

    
    // Open video stream from file
    if( av_open_input_file( &pFormatCtx, inFileName,
                            NULL, 0, &formatParameters ) != 0 ) {
        printf( "Failed to open format context from data stream\n" );
        return;
        }
    
    // Retrieve stream information
    if( av_find_stream_info( pFormatCtx ) < 0 ) {
        printf( "Couldn't find stream information\n" );
        return;
        }
        
    // Dump information about file onto standard error
    dump_format( pFormatCtx, 0, inFileName, false );

    // Find the first video stream
    videoStreamIndex = -1;
    for( int i=0; i<pFormatCtx->nb_streams && videoStreamIndex == -1; i++ ) {
        if( pFormatCtx->streams[i]->codec->codec_type == CODEC_TYPE_VIDEO ) {
            videoStreamIndex = i;
            videoStream = pFormatCtx->streams[i];
            }
        }

    if( videoStreamIndex == -1 ) {
        printf( "Couldn't find a video stream\n" );
        // Fail completely if no video
        return;
        }

    // Find the first audio stream
    audioStreamIndex = -1;
    for( int i=0; i<pFormatCtx->nb_streams && audioStreamIndex == -1; i++ ) {
        if( pFormatCtx->streams[i]->codec->codec_type == CODEC_TYPE_AUDIO ) {
            audioStreamIndex = i;
            audioStream = pFormatCtx->streams[i];
            }
        }

    char audioPresent = true;
    
    if( audioStreamIndex == -1 ) {
        printf( "Couldn't find an audio stream\n" );
        // some clips may not have audio
        audioPresent = false;
        }

    // defaults
    int sampleRate = 44100;
    char stereo = true;
    secondsPerAudioSample = 1.0 / (double)sampleRate * 2;
    
    if( audioPresent ) {
        // grab true sample rate from stream

        sampleRate =
            pFormatCtx->streams[ audioStreamIndex ]->codec->sample_rate;

        int numChannels = 
            pFormatCtx->streams[ audioStreamIndex ]->codec->channels;

        switch( numChannels ) {
            case 1:
                stereo = false;
                secondsPerAudioSample = 1.0 / (double)sampleRate;
                break;
            case 2:
                stereo = true;
                secondsPerAudioSample = 1.0 / ( (double)sampleRate * 2 );
                break;
            default:
                printf( "Error:  only mono and stereo supported, but this "
                        "stream has %d channels\n", numChannels );
                printf( "Disabling audio\n" );
                audioPresent = false;
                break;
            }
        }
    
    
    
    // Get a pointer to the codec context for the video stream
    pVideoCodecCtx = videoStream->codec;

    // Find the decoder for the video stream
    pVideoCodec = avcodec_find_decoder( pVideoCodecCtx->codec_id );
    if( pVideoCodec==NULL ) {
        printf( "Couldn't find a codec for this video stream\n" );
        return;
        }
    
    // Open codec
    if( avcodec_open( pVideoCodecCtx, pVideoCodec ) < 0 ) {
        printf( "Couldn't open the video codec\n" );
        return;
        }

    if( audioPresent ) {
        // Get a pointer to the codec context for the audio stream
        pAudioCodecCtx = audioStream->codec;

        // Find the decoder for the audio stream
        pAudioCodec = avcodec_find_decoder( pAudioCodecCtx->codec_id );
        if( pAudioCodec==NULL ) {
            printf( "Couldn't find a codec for this audio stream\n" );
            return;
            }
        
        // Open codec
        if( avcodec_open( pAudioCodecCtx, pAudioCodec ) < 0 ) {
            printf( "Couldn't open the audio codec\n" );
            return;
            }
        }
    
    /*
    // Strange:  cannot find member frame_rate_base in AVCodecContext
    // Hack to correct wrong frame rates that seem to be generated by some 
    // codecs
    if( pVideoCodecCtx->frame_rate > 1000 &&
        pVideoCodecCtx->frame_rate_base == 1 ) {

        pVideoCodecCtx->frame_rate_base=1000;
        }
    */

    
    // set up the screen

    // width of video frame data
    frameWidth = pVideoCodecCtx->width;
    frameHeight = pVideoCodecCtx->height;

    float aspectRatio = 1.0;
    
    // I don't understand how aspect ratio changes the proper interpretation
    // of width and height
    // However, some videos look distorted in this player but fine in
    // ffplay.
    // Thus, I am adapting the aspect ratio code from ffplay here
    if( pVideoCodecCtx->sample_aspect_ratio.num == 0 ) {
        aspectRatio = 0;
        }
    else {
        aspectRatio =
            av_q2d( pVideoCodecCtx->sample_aspect_ratio )
            * frameWidth / frameHeight;
        }
    if( aspectRatio <= 0.0 ) {
        aspectRatio = (float)frameWidth / (float)frameHeight;
        }

    // Relationships:
    // aspectRatio * frameHeight = frameWidth
    // frameWidth / aspectRatio = frameHeight

    // always maintain the frame width and adjust the height according
    // to the aspect ratio

    // this way, we can always copy rows of pixels out of the video frame
    // without adjusting them
    // we can skip rows to deal with the aspect ratio
    displayHeight = (int)( frameWidth / aspectRatio );
    displayWidth = frameWidth;
    
    printf( "aspect ratio = %0.3f\n", aspectRatio );
    
    // screen = new ScreenGraphics( displayWidth, displayHeight );
    // screen = new ScreenGraphics( screenWidth, screenHeight );

    char screenNeedsToGrow= false;
    
    if( displayWidth > screenWidth ) {
        screenWidth = displayWidth;
        screenNeedsToGrow = true;
        }
    if( displayHeight > screenHeight ) {
        screenHeight = displayHeight;
        screenNeedsToGrow = true;
        }

    if( screenNeedsToGrow ) {
        screen.resize( screenWidth, screenHeight );
        }
    
    graphics = screen.getScreenBuffer();
    screenData = graphics->getBuffer();

    // fill screen with black to clear any old data
    Color black( 0, 0, 0, 1 );
    graphics->fill( black );
    screen.flipScreen();
    
    
    // Allocate video frame
    pFrame = avcodec_alloc_frame();

    // Allocate an AVFrame structure
    pFrameRGBA = avcodec_alloc_frame();

    if( pFrameRGBA == NULL ) {
        printf( "Failed to allocate RGBA frame\n" );
        return;
        }

    
    // Determine required buffer size and allocate buffer
    numBytes = avpicture_get_size( PIX_FMT_RGBA32, pVideoCodecCtx->width,
        pVideoCodecCtx->height );
    buffer = new uint8_t[ numBytes ];
    
    // Assign appropriate parts of buffer to image planes in pFrameRGBA
    avpicture_fill( (AVPicture *)pFrameRGBA, buffer, PIX_FMT_RGBA32,
                    pVideoCodecCtx->width, pVideoCodecCtx->height );


    // time_base is 1/frameRate
    secondsPerFrame = av_q2d( pVideoCodecCtx->time_base );
    // keep track of video time by summing the time from all frames decoded
    // so far
    double videoTime = 0;

    // sometimes PTS does not start at 0 for the first packet
    char firstAudioPTSSet = false;
    double firstAudioPTS = 0.0;
    double lastAudioPTS = 0.0;
    
    char firstVideoPTSSet = false;
    double firstVideoPTS = 0.0;
    double lastVideoPTS = 0.0;

    
    // we synch to audioTime, so if videoTime differs from audioTime,
    // we can hold a frame or skip a frame to compensate


    // start the sound player, if needed
    FFSampleSource *sampleSource = NULL;
    SoundPlayer *soundPlayer = NULL;

    if( audioPresent ) {
        sampleSource = new FFSampleSource();
        
        soundPlayer = new SoundPlayer( sampleRate, stereo, sampleSource );
        }

    

    // packet skip mode:
    // 0 = skip none
    // 1 = skip non-keyframes (and revert to 0 once we get a keyframe)
    // 2 = skip all frames (and revert to 1 when we catch up to clock)
    int skipMode = 0;
    char seenKeyFrameAfterCaughtUp = false;

    
    
    while( !quitFlag && !formatChange && !channelChangeFlag &&
           av_read_frame( pFormatCtx, &packet ) >= 0 ) {

        // one short network step per frame read
        // 10 ms
        executeNetworkStep( 10 );
        
        // check for keyboard input

        // consume all pending keyboard events
        int keyASCII = keyboard.getKeyPressedEvent();        
        while( keyASCII != -1 ) {

            if( (char)keyASCII == 'm' ) {
                channelChangeFlag = true;
                }
            
            keyASCII = keyboard.getKeyPressedEvent();
            }
        
        
        //printf( "(read packet) %d bytes read so far\n",
        //        totalBytesReadSoFar );

        
        if( pVideoCodecCtx->width != frameWidth ) {
            printf( "New width = %d\n", pVideoCodecCtx->width );
            }
        
        // Is this a packet from the video stream?
        if( packet.stream_index == videoStreamIndex ) {
            //printf( "Vid packet\n" );
            
            // presentation time stamp for packet
            double pts = 0;
            if( packet.dts != AV_NOPTS_VALUE ) {
                pts = av_q2d( videoStream->time_base ) * packet.dts;
                

                if( !firstVideoPTSSet ) {
                    firstVideoPTS = pts;
                    lastVideoPTS = pts;
                    firstVideoPTSSet = true;
                    printf( "First video PTS = %0.3f\n", firstVideoPTS );

                    // system clock may start running when first audio frame
                    // is made available for play

                    // however, video packets may not start until after
                    // several audio frames.  Thus, system time may be
                    // running by the time we get our first video packet

                    // further adjust first PTS based on this offset

                    firstVideoPTS -= getSystemMasterClock();

                    printf( "Adjusted first video PTS = %0.3f\n",
                            firstVideoPTS );
                    }

                // if we see a large jump in pts, the stream may have
                // skipped some packets (for example, if we connect in the
                // middle of a stream, we get header packets and then jump
                // to the current packets).
                if( pts > lastVideoPTS + 1 ) {
                    // a jump of more than one second
                    printf( "Video PTS jump\n" );

                    double offset = pts - lastVideoPTS;

                    firstVideoPTS += offset;
                    }
                
                lastVideoPTS = pts;
                
                //printf( "Video pts %0.3f\n", pts );

                // adjust so that pts of packet stream starts
                // at 0
                pts -= firstVideoPTS;

                videoTime = pts;

                //printf( "Video time %0.3f\n", videoTime );
                }
            
            //printf( "PTS for packet = %0.3f\n", pts );
            
            
            double systemMasterClock = getSystemMasterClock();

            //printf( "System clock: %0.3f\n", systemMasterClock );
            
            
            
            // how do we skip decoding frames, not just displaying them?
            char skipNonKeyFrames = false;
            
            // if more than 2 frame's time slow, go into packet-skip mode
            double allowedLagTime = 2 * secondsPerFrame;
            // never skip first frame (when videoTime == 0)
            if( videoTime > 0 &&
                videoTime < ( systemMasterClock - allowedLagTime ) ) {
                
                // skip this packet

                //printf( "Skipping packet, video %0.3f, system %0.3f\n",
                //        videoTime, systemMasterClock );
                
                pVideoCodecCtx->skip_frame = AVDISCARD_ALL;

                // skip all packets until we catch up
                skipMode = 2;
                }
            else if( skipMode == 2 &&
                     videoTime < systemMasterClock ) {
                // stay in skip mode until we agree exactly with
                // our system clock

                pVideoCodecCtx->skip_frame = AVDISCARD_ALL;
                
                }
            else {
                // caught up

                if( skipMode == 2 ) {
                    // we now need to skip to next keyframe

                    seenKeyFrameAfterCaughtUp = false;
                    skipMode = 1;

                    pVideoCodecCtx->skip_frame = AVDISCARD_NONKEY;
                    }
                else if( skipMode == 1 ) {
                    // already searching for next key frame

                    if( seenKeyFrameAfterCaughtUp ) {
                        // already found it

                        skipMode = 0;

                        // go back to skipping none
                        pVideoCodecCtx->skip_frame = AVDISCARD_DEFAULT;
                        }
                    else {
                        // still looking for next keyframe
                        pVideoCodecCtx->skip_frame = AVDISCARD_NONKEY;
                        }
                    }
                else if( skipMode == 0 ) {
                    pVideoCodecCtx->skip_frame = AVDISCARD_DEFAULT;
                    }                    
                }
            
            // Decode video frame
            //printf( "Calling decoder\n" );
            avcodec_decode_video( pVideoCodecCtx, pFrame, &frameFinished, 
                                  packet.data, packet.size );


            


            // Did we get a video frame?
            // note that the frameFinished flag seems to get set to true
            // even when we have AVDISCARD_ALL set
            // however, the previous frame's image is still present in our
            // frame buffer
            // thus, we ignore the frameFinished flag if we're skipping the
            // packet
            char showFrame = false;
            if( frameFinished ) {
                if( skipMode == 0 ) {
                    showFrame = true;

                    // if we are only 1 frame-time behind, simply skip
                    // showing the frame (instead of skipping the decoding
                    // of packets).
                    if( videoTime > 0 &&
                        videoTime < ( systemMasterClock - secondsPerFrame ) ) {
                        showFrame = false;
                        }
                    }
                else if( skipMode == 1 && pFrame->key_frame ) {
                    // this is a key frame
                    showFrame = true;

                    seenKeyFrameAfterCaughtUp = true;
                    }
                else {
                    showFrame = false;
                    //printf( "Skipping a frame\n" );
                    }
                }

            
            if( showFrame ) {
                
                // how far off we are from our master clock
                double timeOffset = systemMasterClock - videoTime;
                    

                double holdFrameTime = 0;

                // if we are more than 1 frame ahead
                if( timeOffset < - secondsPerFrame ) {
                    // hold frame a bit longer to account for offset
                    holdFrameTime = - timeOffset;
                    //printf( "holding frame\n" );
                    }
                
                
                
                // Convert the image from its native format to RGB
                img_convert( (AVPicture *)pFrameRGBA, PIX_FMT_RGBA32, 
                             (AVPicture*)pFrame, pVideoCodecCtx->pix_fmt,
                             pVideoCodecCtx->width,
                             pVideoCodecCtx->height );

                
                // flip onto screen
                int numPixels = displayWidth * displayHeight;

                
                // need to center image on screen
                int yOffset = (screenHeight - displayHeight) / 2;
                int xOffset = (screenWidth - displayWidth) / 2;

                
                    
                unsigned char *frameBuffer = pFrameRGBA->data[0];
                
                if( false && displayHeight == frameHeight ) {
                    // copy all rows of data as one block
                    memcpy( screenData, frameBuffer, 4 * numPixels );
                    }
                else {
                    // need to adjust aspect ratio to avoid display distortion
                    
                    
                    // copy row by row (and potentially skip rows)
                    // to achieve correct aspect ratio
                    
                    // the ratio to maintain of displayed rows
                    // to actual frame rows
                    float rowSkipRatio =
                        (float)displayHeight / (float)frameHeight;
                    
                    int screenRowNumber = 0;
                    int frameRowNumber = 0;
                    while( frameRowNumber < frameHeight &&
                           screenRowNumber < displayHeight ) {
                        float ourRatio = 1.0;
                            
                        if( frameRowNumber > 0 ) {
                            ourRatio =
                                (float)screenRowNumber /
                                (float)frameRowNumber;
                            }
                        
                        if( ourRatio > rowSkipRatio ) {
                            // displayed too many
                            // skip one
                            }
                        else {
                            // displayed too few (or right amount)
                            // display one

                            // use x and y offsets to center destination
                            // of memcpy on screen
                            unsigned long *screenRowPointer =
                                &( screenData[ ( screenRowNumber + yOffset ) *
                                               screenWidth  + xOffset ] );
                            
                            unsigned char *frameRowPointer =
                                &( frameBuffer[ frameRowNumber *
                                              frameWidth * 4 ] );
                            
                            memcpy( screenRowPointer,
                                    frameRowPointer, 4 * frameWidth );
                            
                            screenRowNumber ++;
                            }
                        
                        frameRowNumber++;
                        }
                    }

                //graphics->drawIconMap( iconMap, 0, 0 );
                    
                //screen.flipScreen();
                screen.flipDirtyRectangle( xOffset, yOffset,
                                           displayWidth, displayHeight );
                

                if( !startTimeSet ) {
                    // time that first frame flipped onto screen
                    Time::getCurrentTime( &startSeconds, 
                                          &startMilliseconds );
                    startTimeSet = true;
                    }
                
                if( holdFrameTime != 0 ) {
                    // hold on this frame 
                    unsigned long holdMilliseconds =
                        (unsigned long)( holdFrameTime * 1000 );

                    // spend time on network operations here

                    // track how long steps take in total

                    unsigned long
                        networkStartSeconds,
                        networkStartMilliseconds;
                    
                    Time::getCurrentTime( &networkStartSeconds,
                                          &networkStartMilliseconds );

                    executeNetworkStep( holdMilliseconds );

                    unsigned long networkStepTimeTotal =
                        Time::getMillisecondsSince(
                            networkStartSeconds,
                            networkStartMilliseconds );

                    printf( "Trying to do a network step of %d ms, and it"
                            " took %d ms\n", holdMilliseconds,
                            networkStepTimeTotal );
                    
                    int remainingHoldMilliseconds =
                        holdMilliseconds - networkStepTimeTotal;


                    if( remainingHoldMilliseconds > 0 ) {
                        // sleep to use up remaining time
                        Thread::staticSleep( remainingHoldMilliseconds );
                        }
                    }
                }
            }
        // else is it a packet from the audio stream (if we have one) ?
        else if( audioPresent && packet.stream_index == audioStreamIndex ) {
            //printf( "Audio packet\n" );

            audioSampleBufferLock.lock();

            
            // presentation time stamp for packet
            double pts = 0;
            if( packet.dts != AV_NOPTS_VALUE ) {
                pts = av_q2d( audioStream->time_base ) * packet.dts;
                

                if( !firstAudioPTSSet ) {
                    firstAudioPTS = pts;
                    lastAudioPTS = pts;
                    firstAudioPTSSet = true;
                    printf( "First audio PTS = %0.3f\n", firstAudioPTS );

                    // system clock may start running when first video frame
                    // flipped onto screen 

                    // however, audio packets may not start until after
                    // several video frames.  Thus, system time may be
                    // running by the time we get our first audio packet

                    // further adjust first PTS based on this offset

                    firstAudioPTS -= getSystemMasterClock();

                    printf( "Adjusted first audio PTS = %0.3f\n",
                            firstAudioPTS );
                    }
                
                // if we see a large jump in pts, the stream may have
                // skipped some packets (for example, if we connect in the
                // middle of a stream, we get header packets and then jump
                // to the current packets).
                if( pts > lastAudioPTS + 1 ) {
                    // a jump of more than one second
                    printf( "Audio PTS jump\n" );

                    double offset = pts - lastAudioPTS;

                    firstAudioPTS += offset;
                    }

                lastAudioPTS = pts;
                
                //printf( "Audio pts %0.3f\n", pts );
                
                // adjust so that pts of packet stream starts
                // at 0
                pts -= firstAudioPTS;

                // we have the pts of this packet, but there are other
                // samples that are waiting to play
                audioTime =
                    pts -
                    secondsPerAudioSample * audioSampleBufferLength;

                //printf( "Audio time %0.3f\n", audioTime );
                }
            
            
            
            // buffer that we reuse for each frame in the packet
            int16_t audioBuffer[ AVCODEC_MAX_AUDIO_FRAME_SIZE ];


            // there can be more than one frame in each audio packet

            // thus, we need to jump through the packet until we decode all
            // of them

            int packetDataOffset = 0;
            int remainingPacketSize = packet.size;

            while( remainingPacketSize > 0 ) {
            
                int decodedDataSize;
                int packetDataConsumed =
                    avcodec_decode_audio( pAudioCodecCtx,
                                          audioBuffer,
                                          &decodedDataSize,
                                          packet.data + packetDataOffset,
                                          remainingPacketSize );

                // decodedDataSize is number of bytes
                // 2 bytes per sample
                int numDecodedSamples = decodedDataSize / 2;

                // create a new sample buffer that is big enough to contain
                // the old data plus the new data
                float *newAudioSampleBuffer =
                    new float[ audioSampleBufferLength + numDecodedSamples ];

                // copy old data in
                memcpy( newAudioSampleBuffer, audioSampleBuffer,
                        audioSampleBufferLength * sizeof( float ) );

                // now copy new data, converted to float, in                
                for( int i=0; i<numDecodedSamples; i++ ) {
                    // convert each sample to a -1.0 .. +1.0 float
                    newAudioSampleBuffer[ audioSampleBufferLength + i ] =
                        ( audioBuffer[i] / 65535.0 );
                    }

                // replace audioSampleBuffer with this new buffer
                delete [] audioSampleBuffer;
                audioSampleBuffer = newAudioSampleBuffer;
                audioSampleBufferLength =
                    audioSampleBufferLength + numDecodedSamples;
                
                // now jump forward in the packet
                packetDataOffset += packetDataConsumed;
                remainingPacketSize -= packetDataConsumed;
                }

            audioSampleBufferLock.unlock();

            if( !startTimeSet ) {
                // time that first audio frame available for play
                Time::getCurrentTime( &startSeconds, 
                                      &startMilliseconds );
                startTimeSet = true;
                }
            
            }
        // end check for packet type
        
        
        // Free the packet that was allocated by av_read_frame
        av_free_packet( &packet );
        }

    if( soundPlayer != NULL ) {
        delete soundPlayer;
        delete sampleSource;
        }
    
    
    // Free the RGBA image
    delete [] buffer;
    av_free(pFrameRGBA);

    // Free the YUV frame
    av_free(pFrame);

    // Close the codec
    avcodec_close(pVideoCodecCtx);

    // Close the video file
    av_close_input_file(pFormatCtx);


    // destroy local screen components
    delete graphics;


    if( audioSampleBuffer != NULL ) {
        delete [] audioSampleBuffer;
        audioSampleBuffer = NULL;
        }

    if( formatChange ) {
        printf( "Format change\n" );
        }
    
    return;
    }
