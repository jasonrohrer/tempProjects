/*
 * Modification History
 *
 * 2001-May-21   Jason Rohrer
 * Created.
 *
 * 2001-May-22   Jason Rohrer
 * Added explicit constructors. 
 */

package os.system.exception;


/**
 * An exception thrown when object creation fails
 * (for example, if the requested class is not
 * <code>OS</code>-aware).
 *
 * @author Jason Rohrer
 */
public class ObjectCreationException extends Exception {


	
	/**
	 * Constructs an <code>ObjectCreationException</code> with
	 * no specified detail message.
	 */
	public ObjectCreationException() {
		super();
		}


	
	/**
	 * Constructs an <code>ObjectCreationException</code> with
	 * a specified detail message.
	 *
	 * @param inMessage a detail message.
	 */
	public ObjectCreationException( String inMessage ) {
		super( inMessage );
		}


	
	}

