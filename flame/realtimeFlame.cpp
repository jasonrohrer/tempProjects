/*
 * Modification History
 *
 * 2003-July-29   Jason Rohrer
 * Created.
 *
 * 2003-July-31   Jason Rohrer
 * Improved coloring methods to match (or exceed) Apophysis.
 *
 * 2003-August-3   Jason Rohrer
 * Changed to use best color accumulation method, added other coefficient sets.
 * Added support for arbitrary gradients.
 *
 * 2003-August-5   Jason Rohrer
 * Added x and y image offsets.  Added additional coeff sets.
 */



#include "minorGems/graphics/ScreenGraphics.h"
#include "minorGems/graphics/GraphicBuffer.h"

#include "minorGems/util/random/StdRandomSource.h"
#include "minorGems/system/Thread.h"



#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <math.h>


char running = true;

void catch_int(int sig_num) {
    if( running ) {
        printf( "Quiting...\n" );
        running = false;
        signal( SIGINT, catch_int);
        }
    else {
        exit(1);
        }
    }



int numGradientPoints;
double *gradientPointPositions;
double **gradientPointColors;



// maps a [0,1] gradient index and a color number into a [0,1] color value
// color numbers are 0=red, 1=gree, 2=blue
double getColor( double inGradientIndex, int inColorNumber ) {
    // linear blend
    int closestPointAbove;
    int closestPointBelow;
    char foundPoints = false;
    for( int i=0; !foundPoints && i<numGradientPoints-1; i++ ) {
        if( gradientPointPositions[i] <= inGradientIndex &&
            gradientPointPositions[i+1] >= inGradientIndex ) {
            closestPointAbove = i+1;
            closestPointBelow = i;
            foundPoints = true;
            }
        }

    double positionAbove = gradientPointPositions[ closestPointAbove ];
    double positionBelow = gradientPointPositions[ closestPointBelow ];
    double spread = positionAbove - positionBelow;
    
    double colorAbove =
        gradientPointColors[ closestPointAbove ][inColorNumber];
    double colorBelow =
        gradientPointColors[ closestPointBelow ][inColorNumber];

    if( spread == 0 ) {
        return colorAbove;
        }


    return ( ( inGradientIndex - positionBelow ) * colorAbove +
             (  positionAbove - inGradientIndex ) * colorBelow
             ) /  spread;
    
    }


// maps a [0,1] gradient index into a [0,1] red color
double getRed( double inGradientIndex ) {

    return getColor( inGradientIndex, 0 );
    
    /*
    // yellow if index = 0.5
    double pixelRed = 1.0;
    
    // greener for < 0.5 index
    if( inGradientIndex < 0.5 ) {
        pixelRed -= 2 * ( 0.5 - inGradientIndex );
        }

    if( inGradientIndex > 0.25 && inGradientIndex < 0.35 ) {
        // purple stripe in middle
        pixelRed = 1.0;
        }
    return pixelRed;
    */
    }



// maps a [0,1] color index into an arGb color
double getGreen( double inGradientIndex ) {

    return getColor( inGradientIndex, 1 );

    /*
    // yellow if index = 0
    double pixelGreen = 1.0;

    // redder for > 0.5 index
    if( inGradientIndex > 0.5 ) {
        pixelGreen += 2 * ( 0.5 - inGradientIndex );
        }

    if( inGradientIndex > 0.25 && inGradientIndex < 0.35 ) {
        // purple stripe in middle
        pixelGreen = 0;
        }
    return pixelGreen;
    */
    }



// maps a [0,1] color index into an argB color
double getBlue( double inGradientIndex ) {

    return getColor( inGradientIndex, 2 );

    /*
    // yellow if index = 0.5
    double pixelBlue = 0;
    
    if( inGradientIndex > 0.25 && inGradientIndex < 0.35 ) {
        // purple stripe in middle
        pixelBlue = 1.0;
        }
    return pixelBlue;
    */
    }



// maps a [0,1] gradient index into an argb color
unsigned long getScreenColor( double inGradientIndex, double inBrightness ) {

    
    int red = (int)( getColor( inGradientIndex, 0 ) * inBrightness * 255 );
    int green = (int)( getColor( inGradientIndex, 1 ) * inBrightness * 255 );
    int blue = (int)( getColor( inGradientIndex, 2 ) * inBrightness * 255);
    
            
    return
        red << 16 | green << 8 | blue;
    }






int main() {

    signal( SIGINT, catch_int);
    
    int width = 640;
    int height = 480;

    int halfWidth = width/2;
    int halfHeight = height/2;

    double xOffset = -2.5;
    double yOffset = -1.7;
    double scale = 130;
    
    
    int stepsPerRedraw = 100000;
    //int stepsPerRedraw = 1;
    int startSkipSteps = 20;
    //int startSkipSteps = 0;

    
    int numPixels = width * height;

    unsigned long *pixels = new unsigned long[numPixels];

    unsigned long *pixelHitCounts = new unsigned long[numPixels];
    double *pixelRedCounts = new double[numPixels];
    double *pixelGreenCounts = new double[numPixels];
    double *pixelBlueCounts = new double[numPixels];

    // a color index for each pixel
    // modified based on which functions "touch" the pixel
    // if funtion 0 lands on the pixel, the index is decremented,
    // and if function 1 lands on the pixel, the index is incremented.
    double *pixelColorIndex = new double[numPixels];

    
    int i;
    // fill screen with black
    for( i=0; i<numPixels; i++ ) {
        pixels[i] = 0;
        pixelHitCounts[i] = 0;

        pixelColorIndex[i] = 0;
        pixelRedCounts[i] = 0;
        pixelGreenCounts[i] = 0;
        pixelBlueCounts[i] = 0;        
        }

    StdRandomSource *randSource = new StdRandomSource();
    
    // setup gradient
    numGradientPoints = 10;
    gradientPointPositions = new double[numGradientPoints];
    

    gradientPointColors = new double*[numGradientPoints];
    for( i=0; i<numGradientPoints; i++ ) {
        // evenly spaced points
        gradientPointPositions[i] = i * (1.0 / ( numGradientPoints - 1 ) ); 
        
        gradientPointColors[i] = new double[3];
        gradientPointColors[i][0] = randSource->getRandomDouble();
        gradientPointColors[i][1] = randSource->getRandomDouble();
        gradientPointColors[i][2] = randSource->getRandomDouble();
        }


    // draw gradient bar across top
    for( int y=0; y<10; y++ ) {
        for( int x=0; x<width; x++ ) {
            pixels[ y * width + x ] =
                getScreenColor( (double)x / (double)width, 1 );
            }
        }
    

    
    
    ScreenGraphics *screen = new ScreenGraphics( width, height );


    GraphicBuffer *screenBuffer = new GraphicBuffer(
        pixels, width, height );
    
    screen->swapBuffers( screenBuffer );

    


    // setup tranasforms
    int numXforms = 4;

    /*
    double xformColor[2] = { 1, 0 };
    double coeffs[2][6] =
        { { -0.789744, 0.362598, -0.27486, -0.66451, -0.767223,  -0.26784 },
          { 0.536387, 0.416911, -0.336487,  0.427679, 0.292662, -0.533659 } };
    */
    /*
    double coeffs[2][6] =
        { { 0.604277, -0.797912, 0.716041, 0.601121, -0.177195, -0.575022 },
          {-0.0342957, -0.281929, 0.323805, 0.0318931, 0.657963, 0.238455 } };
    */
    /*
    // wing        
    double xformColor[3] = { 1, .5, 0 };
    double coeffs[3][6] =
        { { 0.646257, 1.3588, -0.283822, 0.00255696, -0.211129, -3.12319 },
          { 0.977672, 0.381126, -0.552303, 0.465366, -1.33674, -0.0590757 },
          { 0.256568,  -0.334069, 0.0805013, 0.480298, 1.49009, 0.366954 } };
    */
    // waves
    /*
    double xformColor[4] = { 1, .666, .333, 0 };
    double coeffs[4][6] =
        { { 0.49, 0.08, -0.08, 0.49, 0.70, 1.35 },
          { -0.48, 0.15, -0.15, -0.48, 2.78, 3.34 },
          { 0.48, 0.15, -0.15, 0.48, 1.66, 1.05 },
          { 0.43, 0.31, -0.43,  0.54, 2.43, 1.43 } };
    */
    // waves mistake (nice find!)
    double xformColor[4] = { 1, .666, .333, 0 };
    double coeffs[4][6] =
        { { 0.49, -0.08, 0.08, 0.49, 0.70, 1.35 },
          { -0.48, -0.15, 0.15, -0.48, 2.78, 3.34 },
          { 0.48, -0.15, 0.15, 0.48, 1.66, 1.05 },
          { 0.43, -0.43, 0.31, 0.54, 2.43, 1.43 } };
    
    /*
    // triangle gasket
    double coeffs[3][6] =
        { { .5, 0, 0, .5, -.25, -.25 },
          { .5, 0, 0, .5, -.25, +.25},
          { .5, 0, 0, .5, +.25, -.25} };
    */
    // square gasket
    /*
    double coeffs[4][6] =
        { { .45, 0, 0, .45, -.25, -.25 },
          { .45, 0, 0, .45, -.25, +.25},
          { .45, 0, 0, .45, +.25, -.25},
          { .45, 0, 0, .45, +.25, +.25} };
    
    double xformColor[4] = { 1, .66, .33, 0 };
    */

    /*
    printf( "Coeffs:\n{ " );
    for( i=0; i<2; i++ ) {
        printf( "{ " );
        for( int j=0; j<6; j++ ) {
            coeffs[i][j] = randSource->getRandomDouble() * 2 - 1;
            printf( "%f", coeffs[i][j] );
            if( j < 5 ) {
                printf( "," );
                }
            printf( " " );
            }
        printf( "},\n" );
        }
    printf( "};" );
    */
    
    double x = randSource->getRandomDouble() * 2 - 1;
    double y = randSource->getRandomDouble() * 2 - 1;
    double c = randSource->getRandomDouble();

    
    int stepCount = 0;
    while( running ) {
        if( stepCount % 1000000 == 0 ) {
            printf( "Step %d\n", stepCount );
            }
        int f = randSource->getRandomBoundedInt( 0, numXforms-1 );
        
        // From Erik Reckase's UltraFractal flame coloring:
        //
        // Draves coefficients are in the order :
        // x' = ax + cy + e    [a c b d e f]
        // y' = bx + dy + f
        
        double newX = coeffs[f][0] * x + coeffs[f][2] * y + coeffs[f][4];
        double newY = coeffs[f][1] * x + coeffs[f][3] * y + coeffs[f][5];
        double newC;

        // averaging method
        newC = c + xformColor[f];
        newC = newC / 2;
        
        
        // no average method
        //newC = xformColor[f];
        
        
        x = newX;
        y = newY;
        c = newC;

        if( stepCount >= startSkipSteps ) {
            int pixX = (int)( ( x + xOffset ) * scale + halfWidth );
            int pixY = height - (int)( ( y + yOffset ) * scale + halfHeight );

             
            
            if( pixY < height && pixY >=0 && pixX < width && pixX >=0 ) {
                int pixelIndex = pixY * width + pixX;

                pixelHitCounts[ pixelIndex ] ++;

                int hitCount = pixelHitCounts[ pixelIndex ];

                double logHitCount = log( hitCount );

                double pixelValue = ( logHitCount / 10 );

                if( pixelValue > 1 ) {
                    pixelValue = 1;
                    }
                
                pixelColorIndex[ pixelIndex ] += c;


                /*
                // pixel is average of all c values that hit it
                // take average over all hits
                double pixelColor = pixelColorIndex[ pixelIndex ] /
                    (double)( pixelHitCounts[ pixelIndex ] );

                pixels[ pixelIndex ] =
                    getScreenColor( pixelColor, pixelValue );
                */
                
                                
                // or pixel is average of all RGB values that hit it
                // this matches Draves' approach
                pixelRedCounts[ pixelIndex ] += getRed( c );
                pixelGreenCounts[ pixelIndex ] += getGreen( c );
                pixelBlueCounts[ pixelIndex ] += getBlue( c );


                double pixelRed = pixelRedCounts[ pixelIndex ] /
                    (double)( pixelHitCounts[ pixelIndex ] );
                double pixelGreen = pixelGreenCounts[ pixelIndex ] /
                    (double)( pixelHitCounts[ pixelIndex ] );
                double pixelBlue = pixelBlueCounts[ pixelIndex ] /
                    (double)( pixelHitCounts[ pixelIndex ] );
                

                int red = (int)( pixelRed * pixelValue * 255 );
                int green = (int)( pixelGreen * pixelValue * 255 );
                int blue = (int)( pixelBlue * pixelValue * 255 );

                pixels[ pixelIndex ] = red << 16 | green << 8 | blue;
                
                }
            }

        
        stepCount++;

        if( stepCount % stepsPerRedraw == 0 ) {
            screen->swapBuffers( screenBuffer );
            }
        }
    
    

    delete screenBuffer;
    delete screen;
    delete randSource;
    delete [] pixels;
    delete [] pixelHitCounts;
    delete [] pixelColorIndex;


    for( i=0; i<numGradientPoints; i++ ) {
        delete [] gradientPointColors[i];
        }
    delete [] gradientPointColors;
    delete [] gradientPointPositions;
    
    
    printf( "Done.\n" );
    }
