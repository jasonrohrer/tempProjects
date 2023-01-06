/*
 * Modification History
 *
 * 2001-May-21   Jason Rohrer
 * Created.
 *
 * 2001-May-22   Jason Rohrer
 * Added a method for getting all classes.
 * Added an implemented method for checking if a class is
 * OS-aware.
 * Changed to be an abstract class instead of an interface.
 * Added a method for getting an object's name.
 * Changed the definition of an OS-aware class.
 *
 * 2001-May-23   Jason Rohrer
 * Changed to use OSAware objects.
 * Changed to have an objectChanged method.
 * Changed to have an opener parameter for the open method.
 * Added an ignoreObject parameter to the getObjects method.
 * Added a closeObject method.
 */

package os.system;

import os.system.exception.*;

import java.lang.reflect.Constructor;

/**
 * Interface for the OS.
 *
 * @author Jason Rohrer
 */
public abstract class OS {


	
	/**
	 * Opens an <code>Object</code> instance from in the system
	 * that implements a set of interfaces.
	 * <P>
	 * <code>OS</code> handles selecting the object.  Note that
	 * this may involve prompting the user to select an object
	 * graphically, so this call may block.
	 *
	 * @param inRequiredInterfaces an array of <code>Class</code>es
	 *   representing the required interfaces.
	 *   Note that all classes in the array
	 *   must be interfaces (in other words, invoking
	 *   <code>isInterface()</code> on the class must return true).
	 * @param inOpener the object that is trying to open
	 *   an object.
	 *
	 * @return an <code>OSAwareObject</code> that implements
	 *   the specified interfaces.
	 *
	 * @exception NoSufficientObjectException if no object in the
	 *   system supports all of the required interfaces.
	 * @exception OpenAbortedException if the open operation is aborted
	 *   without selecting an object.
	 */
	public abstract OSAwareObject openObject(
		Class inRequiredInterfaces[], OSAwareObject inOpener )
		throws NoSufficientObjectException,
			   OpenAbortedException;

	

	/**
	 * Closes a connection to an object instance in the system.
	 * <p>
	 * Note that this does not remove the object from the
	 * system.
	 *
	 * @param inObject the object to close.
	 * @param inOpener the object that currently has inObject
	 *   open.  <code>inOpener</code> will stop receiving
	 *   change notification for <code>inObject</code> after
	 *   the close operation complets.
	 */
	public abstract void closeObject( OSAwareObject inObject,
									  OSAwareObject inOpener );
	

	
	/**
	 * Gets a new <code>Object</code> instance that implements a set of
	 * interfaces.
	 * <P>
	 * <code>OS</code> selects which class type to instantiate, though
	 * this decision will not involve the user, which differs
	 * from {@link #openObject}.
	 *
	 * @param inRequiredInterfaces an array of <code>Class</code>es
	 *   representing the required interfaces.
	 *   Note that all classes in the array
	 *   must be interfaces (in other words, invoking
	 *   <code>isInterface()</code> on the class must return true).
	 * @param inInstanceName the human-readable name to
	 *   bind to this object instance.
	 *
	 * @return a new <code>OSAwareObject</code> instance
	 *   that implements the specified interfaces.
	 *
	 * @exception NoSufficientClassException if no class in the
	 *   system supports all of the required interfaces.
	 */
	public abstract OSAwareObject createObject(
		Class inRequiredInterfaces[],
		String inInstanceName )
		throws NoSufficientClassException;

	

	/**
	 * Removes an <code>Object</code> from the system.
	 * <P>
	 * Note that <code>OS</code> only removes the object
	 * from its own internal tables.  The object is
	 * not actually destroyed by this call, though <code>OS</code>
	 * will no longer be aware of it.
	 *
	 * @param inObject the object to remove from <code>OS</code>.
	 *
	 * @return <code>true</code> iff the object was removed
	 *   successfully.
	 */
	public abstract boolean removeObject( OSAwareObject inObject );
	

	
	/**
	 * Gets a new <code>Object</code> instance of a specified
	 * <code>Class</code>.
	 * <P>
	 * Note tha<code>OS</code> selects which class type to instantiate, though
	 * this decision will not involve the user, which differs
	 * from {@link #openObject}.
	 *
	 * @param inClass the <code>Class</code> to create an
	 *   instance of.  Note that the class must be
	 *   <code>OS</code>-aware (it must have a constructor that
	 *   takes an <code>OS</code> and <code>String</code>as its
	 *   only arguments.
	 * @param inInstanceName the human-readable name to
	 *   bind to this object instance.
	 *
	 * @return a new <code>OSAwareObject</code> instance of the
	 *   specified class.
	 *
	 * @exception ObjectCreationException if object creation fails
	 *   (for example, if the specified class is not
	 *   <code>OS</code>-aware).
	 */
	public abstract OSAwareObject createObject(
		Class inClass,
		String inInstanceName )
		throws ObjectCreationException;
	

	
	/**
	 * Gets all <code>OSAwareObject</code> instances in the
	 * system that implement a set of interfaces.
	 * <P>
	 * Note that in most common programming situations, this
	 * method call should be avoided, since opening an object
	 * from the system without user interaction can be
	 * dangerous.  Using {@link #openObject} is recommended.
	 *
	 * @param inRequiredInterfaces an array of <code>Class</code>es
	 *   representing the required interfaces.
	 *   Note that all classes in the array
	 *   must be interfaces (in other words, invoking
	 *   <code>isInterface()</code> on the class must return true).
	 * @param inIgnoreObject an object to ignore when looking
	 *   for sufficient objects.  Passing in <code>null</code>
	 *   will cause all objects to be considered.
	 *
	 * @return an array of objects that implement the specified
	 *   interfaces.  The returned array will contain no elements
	 *   if no sufficient objects are found.
	 */
	public abstract OSAwareObject[] getObjects(
		Class inRequiredInterfaces[], OSAwareObject inIgnoreObject );
	

	
	/**
	 * Gets all OS-aware classes in the system that implement
	 * a set of interfaces.
	 *
	 * @param inRequiredInterfaces an array of <code>Class</code>es
	 *   representing the required interfaces.
	 *   Note that all classes in the array
	 *   must be interfaces (in other words, invoking
	 *   <code>isInterface()</code> on the class must return true).
	 *
	 * @return an array of <code>Class</code>es that implement the
	 *   specified interfaces.  The returned array will contain no
	 *   elements if no sufficient classes are found.
	 *   All classes returned are object classes that can
	 *   be instantiated (not interfaces).
	 */
	public abstract Class[] getObjectClasses(
		Class inRequiredInterfaces[] );



	/**
	 * Gets all OS-aware object classes in the system.
	 *
	 * All classes returned are object classes that
	 * can be instantiated (not interfaces).
	 *
	 * @return an array all classes in the system.
	 */
	public abstract Class[] getAllObjectClasses();



	/**
	 * Gets the human-readable name of an object.
	 *
	 * @param inObject the object to get a name for.
	 *
	 * @return the name of the object, or <code>null</code>
	 *   if the object is not in OS.
	 */
	public abstract String getObjectName( OSAwareObject inObject );
	


	/**
	 * Tells <code>OS</code> that an object has changed.
	 *
	 * Must be called by OS-aware objects every time they
	 * change so that the change is registered by
	 * objects that have opened them.
	 *
	 * @param inObject the object that has changed.
	 */
	public abstract void objectChanged( OSAwareObject inObject );

	
	
	/**
	 * Checks whether a class is OS aware
	 * (in other words, whether the class has
	 * a constructor taking only an <code>OS</code> and
	 * a <code>String</code> argument ).
	 *
	 * @param inClass the class to check.
	 *
	 * @return <code>true</code> iff the class is OS aware.
	 */
	protected boolean isOSAware( Class inClass ) {
		Constructor constructors[] = inClass.getConstructors();


		// first, look for the OSAwareObject interface
		Class osAwareClass;
		try {
			osAwareClass = Class.forName( "os.system.OSAwareObject" );
			}
		catch( ClassNotFoundException inNotFoundE ) {
			System.out.println(
				"Fatal error:  OSAwareObject interface class not found." );
			return false;
			}

		Class interfaces[] = inClass.getInterfaces();
		boolean hasInterface = false;
		for( int i=0; i<interfaces.length && !hasInterface; i++ ) {
			if( interfaces[i] == osAwareClass ) {
				hasInterface = true;
				}
			}

		if( !hasInterface ) {
			return false;
			}
		
		// else continue, and check for proper constructor
		
		Class osClass;
		Class stringClass;
		try {
			osClass = Class.forName( "os.system.OS" );
			}
		catch( ClassNotFoundException inNotFoundE ) {
			System.out.println(
				"Fatal error:  OS interface class not found." );
			return false;
			}
		try {
			stringClass = Class.forName( "java.lang.String" );
			}
		catch( ClassNotFoundException inNotFoundE ) {
			System.out.println(
				"Fatal error:  String class not found." );
			return false;
			}
		
		// search for a good constructor
		for( int i=0; i<constructors.length; i++ ) {

			Class paramTypes[] = constructors[i].getParameterTypes();
			
			if( paramTypes.length == 2 ) {

				if( paramTypes[0] == osClass &&
					paramTypes[1] == stringClass ) {
					
					// found a valid constructor
					return true;
					}
				}

			}

		// not found
		return false;
		}

	
	
	}

