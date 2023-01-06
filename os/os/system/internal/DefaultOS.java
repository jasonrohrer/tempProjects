/*
 * Modification History
 *
 * 2001-May-21   Jason Rohrer
 * Created.
 *
 * 2001-May-22   Jason Rohrer
 * Worked on implementation of various methods.
 * Moved os.system.DefaultOS to os.system.internal.DefaultOS.
 * Finished initial implementation.
 * Changed to use an ObjectSelector for opening objects.
 * Changed to implement the modified OS interface.
 *
 * 2001-May-23   Jason Rohrer
 * Changed to use new OS interface.
 * Chagned to implement the new closeObject method.
 */

package os.system.internal;

import os.system.OS;
import os.system.OSAwareObject;
import os.system.internal.FileClassLoader;
import os.system.internal.gui.ObjectSelector;
import os.system.exception.*;

import java.util.Vector;
import java.util.Enumeration;
import java.lang.reflect.Constructor;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;



/**
 * Default implementation of the OS interface.
 *
 * @author Jason Rohrer
 */
public class DefaultOS extends OS {


	
	/**
	 * Constructs a DefaultOS.
	 *
	 * @param inStorageDirectory the directory
	 *   in the external file system where classes
	 *   and objects are stored.
	 */
	public DefaultOS( File inStorageDirectory ) {

		FileClassLoader loader = new FileClassLoader();

		// check that it is a directory
		if( !inStorageDirectory.isDirectory() ) {
			System.out.println(
				"Specified file is not a directory:  "
				+ inStorageDirectory.getAbsolutePath() );
			return;
			}

		File classDirectory = null;
		File objectDirectory = null;

		File files[] = inStorageDirectory.listFiles();

		// find the two directories of interest
		for( int i=0; i<files.length; i++ ) {
			if( files[i].isDirectory() ) {
				if( files[i].getName().equals( "classes" ) ) {
					classDirectory = files[i];
					}
				else if( files[i].getName().equals( "objects" ) ) {
					objectDirectory = files[i];
					}
				}
			}

		if( classDirectory == null ) {
			System.out.println(
				"required directory, \"classes\", not found" );
			return;
			}
		if( objectDirectory == null ) {
			System.out.println(
				"required directory, \"objects\", not found" );
			return;
			}

		// load all the classes from the "classes" directory
		File classFiles[] = classDirectory.listFiles();

		// attempt to load each class
		for( int i=0; i<classFiles.length; i++ ) {
			String name = classFiles[i].getName();
			if( name.endsWith( ".class" ) ) {
				// found a class file
				try {
					Class c = loader.loadClass( classFiles[i] );

					if( isOSAware( c ) ) {
						mOSAwareClasses.addElement( c );
						}
					}
				catch( FileNotFoundException inNotFoundE ) {
					System.out.println( "File not found:  "
										+ name );
					}
				catch( ClassFormatError inFormatE ) {
					System.out.println(
						"File does not contain a valid class:  "
						+ name );
					}
				catch( IOException inIOE ) {
					System.out.println(
						"Error while reading from file:  "
						+ name );
					}
				}
			}

		// don't load objects for now
		
		}

	
	
	// implements the OS inteface
	public OSAwareObject openObject( Class inRequiredInterfaces[],
									 OSAwareObject inOpener )
		throws NoSufficientObjectException,
		OpenAbortedException {

		// gather a collection of sufficient objects.
		// ignore the opener when considering objects
		// to prevent the opener from opening itself.
		OSAwareObject sufficientObjects[] =
			getObjects( inRequiredInterfaces, inOpener );

		if( sufficientObjects.length == 0 ) {
			throw new NoSufficientObjectException();
			}
		else {
			// launch an ObjectSelector to pick an object
			String objectNames[] =
				new String[ sufficientObjects.length ];
			for( int i=0; i<objectNames.length; i++ ) {
				
				objectNames[i] =
					getObjectName( sufficientObjects[i] );
				}
			
			OSAwareObject returnArray[] = new OSAwareObject[1];
			ObjectSelector selector =
				new ObjectSelector( sufficientObjects,
									objectNames,
									null,
									returnArray );
			if( returnArray[0] == null ) {
				throw new OpenAbortedException();
				}
			else {
				// object selected

				// register this opener with the opened object
				int index =
					mInstantiatedObjects.indexOf( returnArray[0] );

				// assume that index is not -1 here
				Vector openers = (Vector)(
					mInstantiatedObjectOpeners.elementAt( index ) );

				openers.addElement( inOpener );

				// return the opened object
				return returnArray[0];
				}
			}
		}


	
	// implements the OS inteface
	public void closeObject( OSAwareObject inObject,
									  OSAwareObject inOpener ) {

		int objectIndex =
			mInstantiatedObjects.indexOf( inObject );

		if( objectIndex == -1 ) {
			// do nothing
			}
		else {
			Vector openers = (Vector)(
				mInstantiatedObjectOpeners.elementAt(
					objectIndex ) );
			
			int openerIndex = openers.indexOf( inOpener );

			if( openerIndex == -1 ) {
				// do nothing
				}
			else {
				// remove the opener from the list
				openers.removeElementAt( openerIndex );
				}
			}
		}

	
	
	// implements the OS inteface
	public OSAwareObject createObject( Class inRequiredInterfaces[],
									   String inInstanceName )
		throws NoSufficientClassException {
		// gather a collection of sufficient classes
		Class sufficientClasses[] = getObjectClasses( inRequiredInterfaces );

		if( sufficientClasses.length == 0 ) {
			throw new NoSufficientClassException();
			}
		else {
			// go through each sufficient class until
			// we find one that is OS aware and construct
			// an instance of it
			for( int i=0; i<sufficientClasses.length; i++ ) {
			
				Class classToUse = sufficientClasses[i];
				try {
					OSAwareObject o = createObject( classToUse,
											 inInstanceName );

					// simply return it, since createObject
					// has already added it to our vector
					return o;
					}
				catch( ObjectCreationException inCreationE ) {
					// creation failed, so move on to the
					// next sufficient class
					}
				}

			// if we made it this far, we failed to construct
			// any of the sufficient classes, so they
			// weren't actually "sufficient"
			throw new NoSufficientClassException();
			}
		}

	

	// implements the OS inteface
	public boolean removeObject( OSAwareObject inObject ) {
		int index = mInstantiatedObjects.indexOf( inObject );
		if( index != -1 ) {		
			mInstantiatedObjects.removeElementAt( index );
			mInstantiatedObjectNames.removeElementAt( index );
			mInstantiatedObjectOpeners.removeElementAt( index );
			return true;
			}
		else {
			return false;
			}
		}

	

	// implements the OS inteface
	public OSAwareObject createObject( Class inClass,
								String inInstanceName  )
		throws ObjectCreationException {


		// the constructor should have only one param,
		// an OS instance
		Class constructorParamTypes[] = new Class[2];
		try {
			constructorParamTypes[0] =
				Class.forName( "os.system.OS" );
			}
		catch( ClassNotFoundException inNotFoundE ) {
			System.out.println(
				"Fatal error:  OS interface class not found." );
			throw new ObjectCreationException(
				"OS interface class not found" );
			}
		try {
			constructorParamTypes[1] =
				Class.forName( "java.lang.String" );
			}
		catch( ClassNotFoundException inNotFoundE ) {
			System.out.println(
				"Fatal error:  String interface class not found." );
			throw new ObjectCreationException(
				"String class not found" );
			}
		
		try {
			Constructor constructor =
				inClass.getConstructor( constructorParamTypes );

			// pass ourselves in as the only param
			Object constructorParams[] = new Object[2];
			constructorParams[0] = this;
			constructorParams[1] = inInstanceName;

			try {
				Object o =
					constructor.newInstance(
						constructorParams );
				// we now have an object instance,
				// so add it to our object vectors and
				// return it
				mInstantiatedObjects.addElement( o );
				mInstantiatedObjectNames.addElement(
					inInstanceName );
				// list of openers, empty so far
				mInstantiatedObjectOpeners.addElement( new Vector() );
				
				return (OSAwareObject)( o );
				}
			catch( Exception inE ) {
				// if construction fails
				throw new ObjectCreationException(
					"constructor invokation failed" );
				}
			}
		catch( NoSuchMethodException inNoMethodE ) {
			// if no sufficient constructor exists
			throw new ObjectCreationException(
				"no sufficient constructor exists" );
			}
		}
	

	
	// implements the OS inteface
	public OSAwareObject[] getObjects( Class inRequiredInterfaces[],
									   OSAwareObject inIgnoreObject ) {
		// gather a collection of sufficient objects
		Vector sufficientObjects = new Vector();

		Enumeration e = mInstantiatedObjects.elements();
		
		while( e.hasMoreElements() ) {
			Object o = e.nextElement();

			// determine whether this object is sufficient
			Class c = o.getClass();
			Class interfaces[] = c.getInterfaces();

			Vector interfaceVector = new Vector();
			for( int i=0; i<interfaces.length; i++ ) {
				interfaceVector.addElement( interfaces[i] );
				}

			boolean oSufficient = true;
			// search for each required interface
			// while we haven't found a counter-example
			for( int i=0; oSufficient && i<inRequiredInterfaces.length;
				 i++ ) {

				oSufficient =
					interfaceVector.contains( inRequiredInterfaces[i] );
				
				}

			if( oSufficient && o != inIgnoreObject ) {
				sufficientObjects.addElement( o );
				}
			}

		OSAwareObject returnArray[] =
			new OSAwareObject[ sufficientObjects.size() ];
		for( int i=0; i<sufficientObjects.size(); i++ ) {
			returnArray[i] =
				(OSAwareObject)( sufficientObjects.elementAt(i) );
			}
		
		return returnArray;
		}

	
	
	// implements the OS inteface
	public Class[] getObjectClasses( Class inRequiredInterfaces[] ) {
		// gather a collection of sufficient classes
		Vector sufficientClasses = new Vector();

		Enumeration e = mOSAwareClasses.elements();
		
		while( e.hasMoreElements() ) {
			Class c = (Class)( e.nextElement() );

			// skip any interface classes
			if( ! c.isInterface() ) {
				// determine whether this class is sufficient
				Class interfaces[] = c.getInterfaces();

				Vector interfaceVector = new Vector();
				for( int i=0; i<interfaces.length; i++ ) {
					interfaceVector.addElement( interfaces[i] );
					}

				boolean cSufficient = true;
				// search for each required interface
				// while we haven't found a counter-example
				for( int i=0;
					 cSufficient && i<inRequiredInterfaces.length;
					 i++ ) {

					cSufficient =
						interfaceVector.contains(
							inRequiredInterfaces[i] );
					}

				if( cSufficient ) {
					sufficientClasses.addElement( c );
					}
				}
			}

		// extract into an array
		Class returnArray[] = new Class[ sufficientClasses.size() ];
		for( int i=0; i<sufficientClasses.size(); i++ ) {
			returnArray[i] =
				(Class)( sufficientClasses.elementAt( i ) );
			}
		
		return returnArray;
		}

	

	// implements the OS interface
	public Class[] getAllObjectClasses() {
		Vector objectClasses = new Vector();
		for( int i=0; i<mOSAwareClasses.size(); i++ ) {
			Class c = (Class)( mOSAwareClasses.elementAt( i ) );
			if( ! c.isInterface() ) {
				objectClasses.add( c );
				}
			}

		// extract into an array
		Class returnArray[] = new Class[ objectClasses.size() ];
		for( int i=0; i<objectClasses.size(); i++ ) {
			returnArray[i] =
				(Class)( objectClasses.elementAt( i ) );
			}
		
		return returnArray;
		}



	// implements the OS inteface
	public String getObjectName( OSAwareObject inObject ) {
		int index = mInstantiatedObjects.indexOf( inObject );

		if( index == -1 ) {
			return null;
			}
		else {
			return (String)(
				mInstantiatedObjectNames.elementAt( index ) );
			}
		}


	
	// implements the OS inteface
	public void objectChanged( OSAwareObject inObject ) {
		int index = mInstantiatedObjects.indexOf( inObject );

		if( index == -1 ) {
			// do nothing
			}
		else {
			Vector openers = (Vector)(
				mInstantiatedObjectOpeners.elementAt( index ) );
			// fire the change to each opener
			for( int i=0; i<openers.size(); i++ ) {
				OSAwareObject o =
					(OSAwareObject)( openers.elementAt( i ) );

				o.openObjectChanged( inObject );				
				}
			}
		}

	
	
	/**
	 * A vector of objects instantiated in the system.
	 */
	private Vector mInstantiatedObjects = new Vector();


	
	/**
	 * A vector of object names.
	 */
	private Vector mInstantiatedObjectNames = new Vector();


	
	/**
	 * A vector of vectors containing objects that
	 * currently have the corresponding object open.
	 */
	private Vector mInstantiatedObjectOpeners = new Vector();

	
	
	/**
	 * A vector of classes in the system.
	 */
	private Vector mOSAwareClasses = new Vector();



	}

