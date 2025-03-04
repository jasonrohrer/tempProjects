#import "stb_image";
#import "stb_image_write";
#import "Basic";


testF :: ( x : int ) -> int {
	return x;
}

main :: () {

	w : s32;
	h : s32;
	c : s32;
	

	// z : = testF( 5 );

	fileName : = "testIn.png";
	fileNameOut : = "testBlurred.png";
	
	imageData : *u8 = stbi_load( fileName.data, *w, *h, *c, 0 );

	if imageData != null {
		print( "File % loaded with dimensions %x% and % channels\n", fileName, w, h, c );

		numColorVals : = w * h * c;
		
		blurData : = NewArray( numColorVals, u8 );
		defer array_free( blurData );
		
		valSums : = NewArray( c, int );
		defer array_free( valSums );
		
		boxR : = 40;

		for y: 0 .. h-1 {
			boxStartY : = y - boxR;
			boxEndY : = y + boxR;
			if boxStartY < 0 {
				boxStartY = 0;
			}
			if boxEndY > h-1 {
				boxEndY = h-1;
			}
			boxH : = 1 + boxEndY - boxStartY;

			for x: 0 .. w-1 {
				boxStartX : = x - boxR;
				boxEndX : = x + boxR;
				if boxStartX < 0 {
					boxStartX = 0;
				}
				if boxEndX > w-1 {
					boxEndX = w-1;
				}
				
				boxW : = 1 + boxEndX - boxStartX;
			

				boxPixels : = boxH * boxW;

				for chan : 0 .. c - 1 {
					valSums[chan] = 0;
				}

				for bY : boxStartY .. boxEndY {
					for bX : boxStartX .. boxEndX {
						for chan : 0 .. c - 1 {
							valSums[chan] += imageData[ bY * w * c + bX * c + chan ];
						}
					}
				}
				for chan : 0 .. c - 1 {
					blurData[ y * w * c + x * c + chan ] = cast(u8) ( valSums[chan] / boxPixels );
					//blurData[ y * w * c + x * c + chan ] = imageData[ y * w * c + x * c + chan ];
				}
			}
		}
		
		stbi_write_png( fileNameOut.data, w, h, c, blurData.data, w * c );
		
		stbi_image_free( imageData );

	}
	else {
		print( "File % load failed\n", fileName );
	}

}
