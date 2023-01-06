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
 * An exception thrown when an open operation
 * is aborted before an object is selected.
 *
 * @author Jason Rohrer
 */
public class OpenAbortedException extends Exception {


	
	/**
	 * Constructs an <code>OpenAbortedException</code> with
	 * no specified detail message.
	 */
	public OpenAbortedException() {
		super();
		}


	
	/**
	 * Constructs an <code>OpenAbortedException</code> with
	 * a specified detail message.
	 *
	 * @param inMessage a detail message.
	 */
	public OpenAbortedException( String inMessage ) {
		super( inMessage );
		}

	
	
	}

