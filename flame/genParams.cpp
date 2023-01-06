// generates a parameter file for flame version 1.7 (hpi-flame)


#include <stdio.h>

#include "minorGems/util/random/StdRandomSource.h"


int main( int inNumArgs, char **inArgs ) {

    int numFlames = 100;
    int paletteNumber = 20;
    int qualitySetting = 1;

    
    char *outputFileName = "generated.flame";

    if( inNumArgs > 1 ) {
        outputFileName = inArgs[1];
        }

    FILE *outputFile = fopen( outputFileName, "w" );

    if( outputFile == NULL ) {
        printf( "Failed to open output file %s for writing\n",
                outputFileName );

        return 0;
        }


    StdRandomSource *randSource = new StdRandomSource();

    
    fprintf( outputFile, "<test>\n" );
    

    for( int i=0; i<numFlames; i++ ) {

        fprintf( outputFile,
                 "<flame imageNumber=\"%d\" time=\"0\" palette=\"%d\" size=\"640 480\" "
                 "center=\"0 0\" "
                 "scale=\"240\" zoom=\"0\" oversample=\"1\" filter=\"0\" "
                 "quality=\"%d\" batches=\"10\" background=\"0 0 0\" "
                 "brightness=\"4\" gamma=\"4\" vibrancy=\"1\" "
                 "hue=\"0.22851\">\n\n",
                 i,
                 randSource->getRandomBoundedInt( 0, paletteNumber ),
                 qualitySetting );


        int numXforms = randSource->getRandomBoundedInt( 1, 6 );

        double *weights = new double[ numXforms ];

        int j;

        // generate a normalized weight vector
        double weightSum = 0;
        for( j=0; j<numXforms; j++ ) {
            weights[j] = randSource->getRandomDouble();
            weightSum += weights[j];
            }
        for( j=0; j<numXforms; j++ ) {
            weights[j] = weights[j] / weightSum;
            }


        for( j=0; j<numXforms; j++ ) {

            fprintf( outputFile,
                     "<xform weight=\"%f\" color=\"%f\" var1=\"%d\" "
                     "coefs=\"%f %f %f %f %f %f\"/>\n\n",
                     weights[j],
                     randSource->getRandomDouble(),
                     randSource->getRandomBoundedInt(0, 5),
                     randSource->getRandomDouble() * 2 - 1,
                     randSource->getRandomDouble() * 2 - 1,
                     randSource->getRandomDouble() * 2 - 1,
                     randSource->getRandomDouble() * 2 - 1,
                     randSource->getRandomDouble() * 2 - 1,
                     randSource->getRandomDouble() * 2 - 1);
            }

        delete [] weights;

        fprintf( outputFile, "</flame>\n\n\n\n" );        
        }
    

    fprintf( outputFile, "</test>\n" );
    
    return 0;
    }
