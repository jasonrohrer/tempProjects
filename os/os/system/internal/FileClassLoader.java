/*
 * Modification History
 *
 * 2001-May-22   Jason Rohrer
 * Created.
 */

package os.system.internal;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;



/**
 * Class loader that can load classes from a
 * specified file
 *
 * @author Jason Rohrer
 */
public class FileClassLoader extends ClassLoader {


	
	/**
	 * Loads a class from file.
	 *
	 * @param inFile the file to read the class from.
	 *
	 * @return the class read from inFile.
	 *
	 * @exception FileNotFoundException if the file is
	 *   not found.
	 * @exception ClassFormatError if the file does
	 *   not contain proper class data.
	 * @exception IOException if reading from the file fails.
	 */
	Class loadClass( File inFile )
		throws FileNotFoundException, ClassFormatError,
			   IOException {

		FileInputStream stream =
			new FileInputStream( inFile );

		// read the entire file into memory
		long numToRead = inFile.length();
		byte fileData[] = new byte[ (int)numToRead ];
		stream.read( fileData );

		stream.close();
		
		return defineClass( null, fileData, 0, (int)numToRead );
		}


	
	}
