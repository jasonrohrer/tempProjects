/*
 * Modification History
 *
 * 2001-May-22   Jason Rohrer
 * Created.
 * Changed to include listener support.
 *
 * 2001-May-23   Jason Rohrer
 * Added an interface for change listeners.
 * Removed the change listener interfaces,
 * since this functionality is now handled by the OS.
 * Changed to extend the OSAwareObject interface.
 */

package os.objects.interfaces;

import os.system.OSAwareObject;

import javax.swing.event.ChangeListener;

/**
 * Interface for an object that contains text.
 *
 * @author Jason Rohrer
 */
public interface TextObject extends OSAwareObject {


	
	/**
	 * Gets the text contained by this object.
	 *
	 * @return a <code>String</code> containing this object's text.
	 */
	public String getText();


	
	/**
	 * Sets the text contained by this object.
	 *
	 * @param inText a <code>String</code> containing the text
	 *   to replace this object's text with.
	 */
	public void setText( String inText );


		
	}

