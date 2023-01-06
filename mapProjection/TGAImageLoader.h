/*
 * Modification History
 *
 * 2001-September-23    Jason Rohrer
 * Created. 
 *
 * 2002-June-17    Jason Rohrer
 * Fixed to handle NULL paths. 
 */


#ifndef TGA_IMAGE_LOADER_INCLUDED
#define TGA_IMAGE_LOADER_INCLUDED 


#include "minorGems/graphics/Image.h"
#include "minorGems/graphics/converters/TGAImageConverter.h"


#include "minorGems/io/file/File.h"
#include "minorGems/io/file/FileInputStream.h"
#include "minorGems/io/file/Path.h"

#include <string.h>


/**
 * Class that can load a series of TGA images from a folder.
 *
 * @author Jason Rohrer 
 */
class TGAImageLoader : protected TGAImageConverter { 

	public:

		/**
		 * Constructs an image loader.
		 *
		 * @param inFolderPath the path to the folder from
		 *   which images will be loaded, or NULL to use the current
         *   working folder.
		 *   Will be destroyed when this class is destroyed, if non-NULL;
		 */
		TGAImageLoader( Path *inFolderPath );


		
		~TGAImageLoader();



		/**
		 * Loads a TGA image from this loader's folder.
		 *
		 * @param inFileName the name of the file to load.
		 *   Must be destroyed by caller if not const.
		 *
		 * @return the loaded image, or NULL if loading
		 *   the image fails.
		 */
		Image *loadImage( char *inFileName );


		
	protected:
		Path *mFolderPath;
		
	};



inline TGAImageLoader::TGAImageLoader( Path *inFolderPath )
	: mFolderPath( inFolderPath ) {
	
	}


		
inline TGAImageLoader::~TGAImageLoader() {
    if( mFolderPath != NULL ) {
        delete mFolderPath;
        }
	}



inline Image *TGAImageLoader::loadImage( char *inFileName ) {

	// we need to copy the path since it will be
	// destroyed when the file is destroyed

    Path *copiedPath = NULL;
    if( mFolderPath != NULL ) {
        copiedPath = mFolderPath->copy();
        }

	File *file = new File( copiedPath,
						   inFileName, strlen( inFileName ) ); 
			
	// read image in
	FileInputStream *stream = new FileInputStream( file );

	Image *returnImage = deformatImage( stream );
	
	delete stream;
	delete file;

	if( returnImage == NULL ) {
		// print an error message about the failed load
		
		char *pathString = mFolderPath->getPathStringTerminated();
		
		printf( "Failed to load image file: %s%s\n",
				pathString, inFileName );
		}
	
	return returnImage;
	}


#endif
