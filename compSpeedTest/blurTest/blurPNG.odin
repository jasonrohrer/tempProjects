package blurPNG

import stbi "vendor:stb/image"
import "core:fmt"

main :: proc() {
	w : i32
	h : i32
	c : i32
	
	imageData : [^]byte =  stbi.load( "testIn.png", &w, &h, &c, 0 )

	if imageData != nil {
		fmt.printf( "PNG file loaded with dimensions %vx%v and %v channels\n",
					 w, h, c )

		numColorVals : = w * h * c;
		
		blurData : = make( []byte, numColorVals )
		
		defer delete( blurData );

		valSums : = make( []i32, c )

		defer delete( valSums );
		
		boxR : i32 = 40;

		for y: i32 =0; y<h; y += 1 {
			boxStartY : = y - boxR;
			boxEndY : = y + boxR;
			if boxStartY < 0 {
				boxStartY = 0;
			}
			if boxEndY > h-1 {
				boxEndY = h-1;
			}
			boxH : = 1 + boxEndY - boxStartY;

			for x: i32 =0; x<w; x += 1 {
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

				for chan : i32 =0; chan < c; chan += 1 {
					valSums[chan] = 0;
				}

				for bY:=boxStartY; bY <= boxEndY; bY += 1 {
					for bX:=boxStartX; bX <= boxEndX; bX += 1 {
						for chan : i32 =0; chan < c; chan += 1 {
							valSums[chan] += cast(i32)imageData[ bY * w * c + bX * c + chan ];
						}
					}
				}
				for chan : i32 = 0; chan < c; chan += 1 {
					blurData[ y * w * c + x * c + chan ] = cast(byte) ( valSums[chan] / boxPixels );
				}
			}
		}
		

		stbi.write_png( "testBlurred_odin.png", w, h, c,
						raw_data( blurData ), w * c )
		
		stbi.image_free( imageData )
	}
}
