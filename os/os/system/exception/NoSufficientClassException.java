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
 * An exception thrown when no class implementing
 * a set of desired interfaces is found.
 *
 * @author Jason Rohrer
 */
public class NoSufficientClassException extends Exception {



	/**
	 * Constructs an <code>NoSufficientClassException</code> with
	 * no specified detail message.
	 */
	public NoSufficientClassException() {
		super();
		}


	
	/**
	 * Constructs an <code>NoSufficientClassException</code> with
	 * a specified detail message.
	 *
	 * @param inMessage a detail message.
	 */
	public NoSufficientClassException( String inMessage ) {
		super( inMessage );
		}

	
	
	}

