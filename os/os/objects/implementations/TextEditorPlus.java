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
 * Changed to use OS's new closeObject method.
 */

package os.objects.implementations;

import os.system.OS;
import os.system.OSAwareObject;
import os.objects.interfaces.TextObject;
import os.system.exception.*;

import javax.swing.JFrame;
import javax.swing.JLabel;
import javax.swing.JButton;
import javax.swing.JPanel;
import javax.swing.JTextArea;
import javax.swing.BoxLayout;
import javax.swing.JScrollPane;

import javax.swing.event.CaretListener;
import javax.swing.event.CaretEvent;

import java.awt.event.ActionListener;
import java.awt.event.ActionEvent;
import java.awt.FlowLayout;
import java.awt.GridBagLayout;
import java.awt.GridBagConstraints;

import java.util.Vector;



/**
 * An OS-aware text editor that can open other TextObjects.
 *
 * @author Jason Rohrer
 */
public class TextEditorPlus
	extends JFrame implements TextObject, OSAwareObject {

	
	OS mOS;

	String mLastText;

	String mNoOpenedText = "(none)";
	JLabel mOpenedLabel = new JLabel( "opened:  " );
	JLabel mOpenedNameLabel = new JLabel( mNoOpenedText );
	
	JButton mOpenButton = new JButton( "open" );
	JButton mCloseButton = new JButton( "close" );

	JPanel mButtonPanel = new JPanel();
	JPanel mTopPanel = new JPanel();
	JPanel mOpenedPanel = new JPanel();
	
	
	JTextArea mTextArea = new JTextArea();
	JScrollPane mScrollPane = new JScrollPane();


	ActionListener mListener = new OpenListener();

	JLabel mMessageLabel = new JLabel( " " );
	JPanel mMessagePanel = new JPanel();


	TextObject mCurrentTextObject = null;

	LocalTextListener mLocalTextListener = new LocalTextListener();

	boolean mOpenedChanging = false;
	
	/**
	 * An OS-aware constructor.  Creates and displays
	 * a <code>SimpleTextEditor</code>.
	 *
	 * @param inOS the <code>OS</code> that this editor
	 *   is in.
	 * @param inName the human-readable OS name of this object.
	 */
	public TextEditorPlus( OS inOS, String inName ) {
		super( inName + " <text editor plus>" );
		
		mOS = inOS;
		
		setSize( 400, 500 );
		

		//getContentPane().setLayout(
		//	new BoxLayout( getContentPane(), BoxLayout.Y_AXIS ) );

		mButtonPanel.setLayout( new FlowLayout( FlowLayout.RIGHT ) );
		mButtonPanel.add( mOpenButton );
		mButtonPanel.add( mCloseButton );
		
		mOpenedPanel.setLayout( new FlowLayout( FlowLayout.LEFT ) );
		mOpenedPanel.add( mOpenedLabel );
		mOpenedPanel.add( mOpenedNameLabel );

		mTopPanel.setLayout(
			new BoxLayout( mTopPanel, BoxLayout.X_AXIS ) );
		mTopPanel.add( mOpenedPanel );
		mTopPanel.add( mButtonPanel );

		mMessagePanel.setLayout( new FlowLayout( FlowLayout.CENTER ) );
		mMessagePanel.add( mMessageLabel );

		mScrollPane.getViewport().setView( mTextArea );



		getContentPane().setLayout( new GridBagLayout() );
		GridBagConstraints gbc = new GridBagConstraints();

		gbc.anchor = GridBagConstraints.NORTHWEST;
		gbc.gridwidth = 1;		
		gbc.gridheight = 1;
		
		gbc.fill = GridBagConstraints.HORIZONTAL;
		gbc.gridx = 0;
		gbc.gridy = 0;
		gbc.weightx = 1.0;
		gbc.weighty = 0;
		getContentPane().add( mTopPanel, gbc );

		gbc.gridx = 0;
		gbc.gridy = 1;
		gbc.weightx = 1.0;
		gbc.weighty = 1.0;
		gbc.fill = GridBagConstraints.BOTH;
		getContentPane().add( mScrollPane, gbc );

		gbc.gridx = 0;
		gbc.gridy = 2;
		gbc.weightx = 1.0;
		gbc.weighty = 0;
		gbc.fill = GridBagConstraints.HORIZONTAL;
		getContentPane().add( mMessagePanel, gbc );
		
		mOpenButton.addActionListener( mListener );
		mCloseButton.addActionListener( mListener );
		
		mLastText = mTextArea.getText();

		mTextArea.addCaretListener( mLocalTextListener );
		
		setVisible( true );
		}


	
	// implements the TextObject interface 
	public String getText() {
		return mTextArea.getText();
		}

	

	// implements the TextObject interface 
	public void setText( String inText ) {
		mTextArea.setText( inText );
		//mOS.objectChanged( this );
		}
	

	private class OpenListener implements ActionListener {

		// implements ActionListener.actionPerformed
		public void actionPerformed( ActionEvent inActionEvent ) {
			if( inActionEvent.getSource() == mOpenButton ) {
				Class requiredInterfaces[] = new Class[1];
				try {
					requiredInterfaces[0] =
						Class.forName( "os.objects.interfaces.TextObject" );
					}
				catch( ClassNotFoundException inNotFoundE ) {
					mMessageLabel.setText(
						"fatal error:  TextObject class not found" );
					return;
					}
			
				try {
					TextObject oldObject = mCurrentTextObject;
				
					mCurrentTextObject  =
						(TextObject)(
							mOS.openObject( requiredInterfaces,
											TextEditorPlus.this ) );

					// close the old object
					mOS.closeObject( oldObject, TextEditorPlus.this );
				
					// don't send these
					// text changes to the opened object
					mOpenedChanging = true;
					setText( mCurrentTextObject.getText() );
					mOpenedChanging = false;

					// delete any old messages
					mMessageLabel.setText( " " );

					// set the opened name text
					String name = mOS.getObjectName( mCurrentTextObject );
					mOpenedNameLabel.setText( name );

					mOS.objectChanged( TextEditorPlus.this );
					}
				catch( NoSufficientObjectException inNoObjectE ) {
					mMessageLabel.setText(
						"no TextObjects exist in the system" );
					}
				catch( OpenAbortedException inAbortedE ) {
					// do nothing if aborted
					}
				}
			else if( inActionEvent.getSource() == mCloseButton ) {
				if( mCurrentTextObject != null ) {
					mOS.closeObject( mCurrentTextObject,
									 TextEditorPlus.this );
					mCurrentTextObject = null;
					mOpenedNameLabel.setText( mNoOpenedText );
					}
				}
			}
		}   



	private class LocalTextListener implements CaretListener {

		public void caretUpdate( CaretEvent inCaretE ) {
			//System.out.println( "caret event" );
			if( ! mTextArea.getText().equals( mLastText ) ) {
				// text has changed
				
				// first, send the change to any open object,
				// but only if the change wasn't caused by the
				// open object
				if( mCurrentTextObject != null && !mOpenedChanging ) {
					
					// make sure the changed wasn't caused
					// by the opened object changing
					// (since we shouldn't send these
					// changes back to the opened object)
					if( !mOpenedChanging ) {
						// set the text of the opened object
						mOpenedChanging = true;
						mCurrentTextObject.setText( mTextArea.getText() );
						mOpenedChanging = false;
						}
					}

				// tell the OS that we've changed
				mOS.objectChanged( TextEditorPlus.this );
				}
			}
		}



	// implements OSAwareObject interface
	public void openObjectChanged( OSAwareObject inObject ) {
		// make sure the changed wasn't caused by us
		// changing the open object
		if( !mOpenedChanging ) {
			mOpenedChanging = true;
			setText( mCurrentTextObject.getText() );
			mOpenedChanging = false;
			}
		}


	
	}

