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
 * An exception thrown when no object implementing
 * a set of desired interfaces is found.
 *
 * @author Jason Rohrer
 */
public class NoSufficientObjectException extends Exception {



	/**
	 * Constructs an <code>NoSufficientObjectException</code> with
	 * no specified detail message.
	 */
	public NoSufficientObjectException() {
		super();
		}


	
	/**
	 * Constructs an <code>NoSufficientObjectException</code> with
	 * a specified detail message.
	 *
	 * @param inMessage a detail message.
	 */
	public NoSufficientObjectException( String inMessage ) {
		super( inMessage );
		}

	
	
	}

