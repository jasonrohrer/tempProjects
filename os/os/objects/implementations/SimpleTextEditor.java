/*
 * Modification History
 *
 * 2001-May-22   Jason Rohrer
 * Created.
 *
 * 2001-May-23   Jason Rohrer
 * Changed so that text events are handled.
 * Changed to implement new OSAwareObject interface.
 * Changed to allow OS to handle object change events.
 *
 */

package os.objects.implementations;

import os.system.OS;
import os.system.OSAwareObject;
import os.objects.interfaces.TextObject;


import javax.swing.JFrame;
import javax.swing.JTextArea;
import javax.swing.BoxLayout;
import javax.swing.JScrollPane;

import javax.swing.event.ChangeListener;
import javax.swing.event.ChangeEvent;
import javax.swing.event.CaretListener;
import javax.swing.event.CaretEvent;

import java.awt.event.ActionListener;

import java.util.Vector;



/**
 * An OS-aware text editor.
 *
 * @author Jason Rohrer
 */
public class SimpleTextEditor
	extends JFrame implements TextObject, OSAwareObject {

	
	OS mOS;

	String mLastText;
	
	JTextArea mTextArea = new JTextArea();
	JScrollPane mScrollPane = new JScrollPane();

	TextListener mTextListener = new TextListener();
	
	
	/**
	 * An OS-aware constructor.  Creates and displays
	 * a <code>SimpleTextEditor</code>.
	 *
	 * @param inOS the <code>OS</code> that this editor
	 *   is in.
	 * @param inName the human-readable OS name of this object.
	 */
	public SimpleTextEditor( OS inOS, String inName ) {
		super( inName + " <simple text editor>" );

		mOS = inOS;
		
		setSize( 400, 500 );
		

		getContentPane().setLayout(
			new BoxLayout( getContentPane(), BoxLayout.Y_AXIS ) );

		mScrollPane.getViewport().setView( mTextArea );
		
		getContentPane().add( mScrollPane );

		mLastText = mTextArea.getText();

		mTextArea.addCaretListener( mTextListener );
		
		setVisible( true );
		}


	
	// implements the TextObject interface 
	public String getText() {
		return mTextArea.getText();
		}

	

	/**
	 * True if the text is currently being set
	 * by the setText method, in which case
	 * we shouldn't be telling the OS about the
	 * change twice.
	 */
	private boolean mBeingSet = false;


	
	// implements the TextObject interface 
	public void setText( String inText ) {
		mBeingSet = true;

		mTextArea.setText( inText );
		mOS.objectChanged( this );
		
		mBeingSet = false;
		}



	private class TextListener implements CaretListener {

		public void caretUpdate( CaretEvent inCaretE ) {
			//System.out.println( "caret event" );
			if( ! mTextArea.getText().equals( mLastText ) ) {
				// text has changed

				mLastText = mTextArea.getText();

				if( !mBeingSet ) {
					// tell the OS that we've changed
					mOS.objectChanged( SimpleTextEditor.this );
					}
				}
			}
		}


	
	// implements OSAwareObject interface
	public void openObjectChanged( OSAwareObject inObject ) {
		// do nothing, since we don't support open objects
		}

	
	
	}

