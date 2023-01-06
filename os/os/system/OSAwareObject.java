/*
 * Modification History
 *
 * 2001-May-23   Jason Rohrer
 * Created.
 */

package os.system;



/**
 * Interface for an object that is OSAware.
 * <p>
 * Along with implementing this interface, an
 * <code>OSAwareObject</code> must also provide a
 * constructor that takes
 * an <code>OS</code> and a <code>String</code> as its
 * only parameters.  The <code>String</code> parameter
 * is the human-readable name of the object.
 *
 * @author Jason Rohrer
 */
public interface OSAwareObject {


	
	/**
	 * Tells this object that one of its open
	 * objects has changed.
	 *
	 * @param inObject the object that has changed.
	 */
	public void openObjectChanged( OSAwareObject inObject );

	
	
	}

