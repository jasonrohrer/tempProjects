/*
 * Modification History
 *
 * 2002-June-17   Jason Rohrer
 * Created.
 */


#include "TGAImageLoader.h"
#include "minorGems/graphics/converters/TGAImageConverter.h"
#include "minorGems/graphics/Image.h"

#include "minorGems/io/file/File.h"
#include "minorGems/io/file/FileOutputStream.h"


#include <stdlib.h>
#include <math.h>


/*
 * Some information about map projections:
 *
 * http://mac.usgs.gov/mac/isb/pubs/MapProjections/projections.html
 */


void usage( char *inAppName ) {

    printf( "Usage:\n" );
    printf( "\t%s map_tga_file output_file angle_alpha angle_beta\n",
            inAppName );

    printf( "Examples:\n" );
    printf( "put USA dead center:\n" );
    printf( "\t%s world.tga out.tga 0.28 0.20\n", inAppName );

    printf( "put USA at the very bottom:\n" );
    printf( "\t%s world.tga out.tga 0.28 0.70\n", inAppName );

    printf( "turn the world upside down:\n" );
    printf( "\t%s world.tga out.tga 0.0 1.0\n", inAppName );
	
    exit( 1 );
    }


int main( char inNumArgs, char **inArgs ) {

    if( inNumArgs != 5 ) {
        usage( inArgs[0] );
        }
    
    TGAImageLoader *loader = new TGAImageLoader( NULL );

    Image *baseImage = loader->loadImage( inArgs[1] );


    double pi = 3.14159;

    
    // angle to rotate around y axis of globe
    double alpha;
    // angle to rotate around x axis of globe
    double beta;

    int numRead = sscanf( inArgs[3], "%lf", &alpha );
    if( numRead != 1 ) {
        usage( inArgs[0] );
        }
    numRead = sscanf( inArgs[4], "%lf", &beta );
    if( numRead != 1 ) {
        usage( inArgs[0] );
        }
    
    
    double angleA = alpha * 2 * pi;
    double angleB = beta * pi;
    
    
    if( baseImage != NULL ) {

        int numChannels = baseImage->getNumChannels();
        int w = baseImage->getWidth();
        int h = baseImage->getHeight();

        Image *outImage = new Image( w, h, numChannels );


        for( int c=0; c<numChannels; c++ ) {
            double *baseChannel = baseImage->getChannel( c );
            double *outChannel = outImage->getChannel( c );
           
        
            for( int x=0; x<w; x++ ) {
                double u = (double)x / (double)( w - 1 );
                double angleU = 2 * pi * u;
                for( int y=0; y<h; y++ ) {
                    double v = (double)y / (double)( h - 1 );
                    double angleV = pi * v;
                    
                    // use sphere radius = 1
                    double oldX = cos( angleU ) * sin( angleV );
                    double oldY = cos( angleV );
                    double oldZ = -sin( angleU ) * sin( angleV );

                    double newX = oldX * cos( -angleA ) +
                        oldZ * sin( -angleA );
                    double tempZ = oldX * sin( -angleA )
                        - oldZ * cos( -angleA );

                    double newY = oldY * cos( -angleB ) -
                        tempZ * sin( -angleB );
                    double newZ = oldY * sin( -angleB ) +
                        tempZ * cos( -angleB );

                    
                    double newAngleU =
                        acos( newX / sqrt( newX * newX + newZ * newZ ) );
                    if( newZ >  0 ) {
                        newAngleU = 2 * pi - newAngleU;
                        }
                    newAngleU = 2 * pi - newAngleU;

                    double newAngleV = acos( newY );

                    double projU = newAngleU / ( 2 * pi );
                    double projV = newAngleV / pi;
                                        
                    if( projV < 0 ) {
                        projV = -projV;
                        projU = 0.5 + projU;
                        }
                    if( projV > 1 ) {
                        projV = 2 - projV;
                        projU = 0.5 + projU;
                        }
                    
                    if( projU < 0 ) {
                        projU = 1 + projU;
                        }
                    if( projU > 1 ) {
                        projU = 2 - projU;
                        }
                    
                    int projX = (int)( projU * ( w - 1 ) );
                    int projY = (int)( projV * ( h - 1 ) );

                    outChannel[ y * w + x ] =
                        baseChannel[ projY * w + projX ];
                    }
                }
            }

        File *outFile = new File( NULL, inArgs[2] );

        FileOutputStream *outStream = new FileOutputStream( outFile );

        TGAImageConverter *converter = new TGAImageConverter();

        converter->formatImage( outImage, outStream );

        delete converter;
        delete outStream;
        delete outFile;

        delete outImage;
        
        delete baseImage;
        }
    

    
    delete loader;
    
    return 0;
    }
    

