#define STB_IMAGE_IMPLEMENTATION

#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "stb_image_write.h"


#include <stdlib.h>
#include <stdio.h>


#define MAX_PIXELS 1000


void usage() {
    printf( "\n\nUsage:\n\n" );
    printf( "getPixels imageDirectory out.png\n\n" );

    printf( "imageDirectory must contain files named like this:\n\n" );
    printf( "   frame-000059.png\n\n" );

    printf( "In other words, frame- with the frame number 0-padded "
            "to 6 digits.\n\n" );

    printf( "At most %d frames are supported\n\n", MAX_PIXELS );
    
    exit( 0 );
    }



#define NUM_LIGHTS 2

int pixelsR[NUM_LIGHTS][MAX_PIXELS];
int pixelsG[NUM_LIGHTS][MAX_PIXELS];
int pixelsB[NUM_LIGHTS][MAX_PIXELS];

unsigned char outPNGBuffer[ NUM_LIGHTS * MAX_PIXELS * 3 ];



int lightPixelCoordsX[ NUM_LIGHTS ] = { 116, 239 };
int lightPixelCoordsY[ NUM_LIGHTS ] = { 66, 73 };


int main( int inNumArgs, const char *inArgs[] ) {


    
    int numFrames = 0;
    
    
    if( inNumArgs != 3 ) {
        usage();
        }

    const char *dirName = inArgs[1];
    const char *outFileName = inArgs[2];

    char buffer[900];

    char lastFileGood = true;

    int fileNumber = 0;
    
    while( lastFileGood ) {

        if( numFrames >= MAX_PIXELS ) {
            printf( "\n\nToo many frames found in %s\n\n", dirName );
            usage();
            }
        

        sprintf( buffer, "%s/frame-%06d.png", dirName, fileNumber );

        int width, height, channels;
        unsigned char *data =
            stbi_load( buffer, &width, &height, &channels, 0 );

        if( data == NULL ) {
            lastFileGood = false;
            }
        else {
            
            printf( "Read frame %s\n", buffer );
            
            char imageBigEnough = true;

            for( int i=0; i<NUM_LIGHTS; i++ ) {
                if( width <= lightPixelCoordsX[i] ) {
                    imageBigEnough = false;
                    }
                if( height <= lightPixelCoordsY[i] ) {
                    imageBigEnough = false;
                    }
                }

            if( channels < 3 ) {
                imageBigEnough = false;
                }

            if( imageBigEnough ) {

                for( int i=0; i<NUM_LIGHTS; i++ ) {
                    int x = lightPixelCoordsX[ i ];
                    int y = lightPixelCoordsY[ i ];

                    int rStart = y * channels * width + x * channels;

                    pixelsR[i][numFrames] = data[ rStart ];
                    pixelsG[i][numFrames] = data[ rStart + 1 ];
                    pixelsB[i][numFrames] = data[ rStart + 2 ];
                    }

                numFrames++;
                }
            
            stbi_image_free( data );
            }
        
        
        

        fileNumber++;

        
        }
    
    int numRows = NUM_LIGHTS;

    int numCols = numFrames;
    
    for( int i=0; i<NUM_LIGHTS; i++ ) {

        for( int f=0; f<numFrames; f++ ) {

            int rStart = i * numCols * 3 + f * 3;
            
            outPNGBuffer[ rStart ] = pixelsR[i][f];
            outPNGBuffer[ rStart + 1 ] = pixelsG[i][f];
            outPNGBuffer[ rStart + 2 ] = pixelsB[i][f];
            }
        }
    
    stbi_write_png( outFileName, numCols, numRows, 3, outPNGBuffer,
                    numCols * 3 );
    
    
    
    return 0;
    }
