
#define STB_IMAGE_IMPLEMENTATION

#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "stb_image.h"
#include "stb_image_write.h"
#include <stdio.h>



int main() {

	int w;
	int h;
	int c;
	

	const char *fileName = "testIn.png";
	const char *fileNameOut = "testBlurred_c.png";
	
	unsigned char *imageData = stbi_load( fileName, &w, &h, &c, 0 );

	if( imageData != NULL ) {
        int numColorVals = w * h * c;

        unsigned char *blurData = malloc( numColorVals );
        int *valSums = malloc( c );

        int boxR = 40;

        int x, y, chan;
        
        
        printf( "File %s loaded with dimensions %dx%d and %d channels\n", fileName, w, h, c );
        
		for( y=0; y<h; y++ ) {
			int boxStartY = y - boxR;
			int boxEndY = y + boxR;
            int boxH;
            
			if( boxStartY < 0 ) {
				boxStartY = 0;
                }
			if( boxEndY > h-1 ) {
				boxEndY = h-1;
                }
            
			boxH = 1 + boxEndY - boxStartY;

			for( x=0; x < w; x++ ) {
                int boxStartX = x - boxR;
                int boxEndX = x + boxR;
                int boxW;
                int boxPixels;
                int bY, bX;
                
                if( boxStartX < 0 ) {
                    boxStartX = 0;
                    }
                if( boxEndX > w-1 ) {
                    boxEndX = w-1;
                    }
				
                boxW = 1 + boxEndX - boxStartX;
                  
                boxPixels = boxH * boxW;

                for( chan=0; chan < c; chan++ ) {
                    valSums[chan] = 0;
                    }

                for( bY=boxStartY; bY<=boxEndY; bY++ ) {
                    for( bX=boxStartX; bX<=boxEndX; bX++ ) {
                        for( chan=0; chan < c; chan++ ) {
                            valSums[chan] += imageData[ bY * w * c + bX * c + chan ];
                            }
                        }
                    }
                for( chan=0; chan < c; chan++ ) {
                    blurData[ y * w * c + x * c + chan ] =
                        (unsigned char)( valSums[chan] / boxPixels );
                    }
                }
            }
		
		stbi_write_png( fileNameOut, w, h, c, blurData, w * c );
		
		stbi_image_free( imageData );
        
        }
	else {
		printf( "File %s load failed\n", fileName );
        }

    }
