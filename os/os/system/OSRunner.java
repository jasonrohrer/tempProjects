/*
 * Modification History
 *
 * 2001-May-22   Jason Rohrer
 * Created.
 */

package os.system;

import os.system.OS;
import os.system.internal.gui.ObjectCreator;

import os.system.internal.DefaultOS;

import java.io.File;

/**
 * Class that launches an OS.
 *
 * @author Jason Rohrer
 */
public class OSRunner {


	
	/**
	 * Main method.
	 *
	 * @param inArgs an array of command line arguments.
	 */
	public static void main( String inArgs[] ) {
		if( inArgs.length != 1 ) {
			usage( "OS" );
			}
		File storageDirectory = new File( inArgs[0] );

		OS os = new DefaultOS( storageDirectory );
		
		ObjectCreator creator = new ObjectCreator( os );
		}


	
	/**
	 * Prints a usage message and exits.
	 *
	 * @param inAppName the name of the application.
	 */
	public static void usage( String inAppName ) {
	
		System.out.println( "usage:" );
		System.out.println( inAppName + " storage_directory" );
			
		System.out.println( "example:" );
		System.out.println( "\t" + inAppName + " root" );
			
		System.exit( 1 );
		}


	
	}

