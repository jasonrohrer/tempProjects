/*
 * Modification History
 *
 * 2001-May-22   Jason Rohrer
 * Created.
 *
 * 2001-May-23   Jason Rohrer
 * Fixed a typo.
 * Changed to use OSAwareObjects.
 */

package os.system.internal.gui;

import os.system.OS;
import os.system.OSAwareObject;
import os.system.exception.*;

import javax.swing.JList;
import javax.swing.JFrame;
import javax.swing.JLabel;
import javax.swing.JTextField;
import javax.swing.JButton;
import javax.swing.JPanel;
import javax.swing.BoxLayout;
import javax.swing.ListSelectionModel;
import javax.swing.JScrollPane;
import javax.swing.JDialog;

import java.awt.event.ActionListener;
import java.awt.event.ActionEvent;
import java.awt.FlowLayout;
import java.awt.Frame;
import java.awt.GridBagLayout;
import java.awt.GridBagConstraints;

import java.util.Vector;
import java.util.Enumeration;
import java.lang.reflect.Constructor;

import java.io.File;
import java.io.FileInputStream;

/**
 * Dialog for object selection.
 *
 * @author Jason Rohrer
 */
public class ObjectSelector extends JDialog {


	JList mObjectList = new JList();
	JScrollPane mScrollPane = new JScrollPane();

	JButton mOpenButton = new JButton( "select" );
	JButton mCancelButton = new JButton( "cancel" );
	
	JLabel mResponseLabel = new JLabel( " " );

	JPanel mButtonPanel = new JPanel();
	JPanel mResponsePanel = new JPanel();

	
	ActionListener mListener = new SelectListener();

	// the array to put the selected object in
	OSAwareObject mOutObject[];

	// the Boolean to put the status in
	Boolean mStatus;


	OSAwareObject mObjects[];
	String mObjectNames[];
	
	/**
	 * Constructs an object selector and displays it.
	 * Blocks until an object is selected.
	 *
	 * @param inObjects the list of objects to display
	 *   for selection.
	 * @param inObjectNames the human-readable
	 *   names of the objects.
	 * @param inParent the parent frame to block.
	 * @param outObject <code>Object</code> array
	 *   of size 1 in which selected object will
	 *   be returned.  <code>null</code> will be
	 *   inserted into the array if selection is
	 *   aborted.
	 */
	public ObjectSelector( OSAwareObject inObjects[],
						   String inObjectNames[],
						   Frame inParent,
						   OSAwareObject outObject[] ) {
		super( inParent, "object selector", true );
		
		mOutObject = outObject;

		mObjects = inObjects;
		mObjectNames = inObjectNames;
		
		setSize( 500, 400 );

		
		mObjectList = new JList( mObjectNames );
		mObjectList.getSelectionModel().setSelectionMode(
			ListSelectionModel.SINGLE_SELECTION );
		
		mScrollPane.getViewport().setView( mObjectList );

		mButtonPanel.setLayout( new FlowLayout( FlowLayout.CENTER ) );
		mButtonPanel.add( mOpenButton );
		mButtonPanel.add( mCancelButton );
		
		mResponsePanel.setLayout( new FlowLayout( FlowLayout.CENTER ) );
		mResponsePanel.add( mResponseLabel );


		getContentPane().setLayout( new GridBagLayout() );
		GridBagConstraints gbc = new GridBagConstraints();

		gbc.anchor = GridBagConstraints.NORTHWEST;
		gbc.gridwidth = 1;		
		gbc.gridheight = 1;

		gbc.gridx = 0;
		gbc.gridy = 0;
		gbc.weightx = 1.0;
		gbc.weighty = 1.0;
		gbc.fill = GridBagConstraints.BOTH;
		getContentPane().add( mScrollPane, gbc );

		gbc.fill = GridBagConstraints.HORIZONTAL;
		gbc.gridx = 0;
		gbc.gridy = 1;
		gbc.weightx = 1.0;
		gbc.weighty = 0;
		getContentPane().add( mButtonPanel, gbc );

		gbc.fill = GridBagConstraints.HORIZONTAL;
		gbc.gridx = 0;
		gbc.gridy = 2;
		gbc.weightx = 1.0;
		gbc.weighty = 0;
		getContentPane().add( mResponsePanel, gbc );
				
		mOpenButton.addActionListener( mListener );
		mCancelButton.addActionListener( mListener );

		setVisible( true );
		}
	

	
	private class SelectListener implements ActionListener {

		// implements ActionListener.actionPerformed
		public void actionPerformed( ActionEvent inActionEvent ) {
			if( inActionEvent.getSource() == mOpenButton ) {
				int index = mObjectList.getSelectedIndex();
				if( index != -1 ) {
					mOutObject[0] =
						mObjects[ mObjectList.getSelectedIndex() ];
					// unblock the parent frame
					setVisible( false );
					}
				}
			else if( inActionEvent.getSource() == mCancelButton ) {
				// aborted
				mOutObject[0] = null;
				// unblock the parent frame
				setVisible( false );
				}
			}
		}

	
	
	}

