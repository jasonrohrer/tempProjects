/*
 * Modification History
 *
 * 2001-May-22   Jason Rohrer
 * Created.
 * Switched to a GridBag layout.
 * Made several cosmetic changes.
 */

package os.system.internal.gui;

import os.system.OS;
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

import java.awt.event.ActionListener;
import java.awt.event.ActionEvent;
import java.awt.FlowLayout;
import java.awt.GridBagLayout;
import java.awt.GridBagConstraints;

import java.util.Vector;
import java.util.Enumeration;
import java.lang.reflect.Constructor;

import java.io.File;
import java.io.FileInputStream;

/**
 * Default implementation of the OS interface.
 *
 * @author Jason Rohrer
 */
public class ObjectCreator extends JFrame {

	OS mOS;

	Class mClasses[];
	String mClassNames[];
	
	JList mClassList = new JList();
	JScrollPane mScrollPane = new JScrollPane();
	
	JLabel mInstanceNameLabel = new JLabel( "instance name " );
	JTextField mInstanceNameField = new JTextField( 30 );
	
	
	JButton mCreateButton = new JButton( "create" );

	JLabel mResponseLabel = new JLabel( " " );

	JPanel mInstanceNamePanel = new JPanel();
	JPanel mButtonPanel = new JPanel();
	JPanel mResponsePanel = new JPanel();

	
	ActionListener mListener = new CreatorListener();
	
	/**
	 * Constructs an object creator and displays it.
	 *
	 * @param inOS the OS to create objects in.
	 */
	public ObjectCreator( OS inOS) {
		super( "object creator" );

		setSize( 600, 400 );
		
		mOS = inOS;


		mClasses = mOS.getAllObjectClasses();
		mClassNames = new String[ mClasses.length ];
		for( int i=0; i<mClassNames.length; i++ ) {
			mClassNames[i] = mClasses[i].getName();
			}
		
		mClassList = new JList( mClassNames );
		mClassList.getSelectionModel().setSelectionMode(
			ListSelectionModel.SINGLE_SELECTION );
		
		mScrollPane.getViewport().setView( mClassList );
		
		mInstanceNameField.setEditable( true );

		mButtonPanel.setLayout( new FlowLayout( FlowLayout.CENTER ) );
		mButtonPanel.add( mCreateButton );
		
		mResponsePanel.setLayout( new FlowLayout( FlowLayout.CENTER ) );
		mResponsePanel.add( mResponseLabel );

		mInstanceNamePanel.setLayout( new FlowLayout( FlowLayout.CENTER ) );
		mInstanceNamePanel.add( mInstanceNameLabel );
		mInstanceNamePanel.add( mInstanceNameField );



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
		getContentPane().add( mInstanceNamePanel, gbc );

		gbc.fill = GridBagConstraints.HORIZONTAL;
		gbc.gridx = 0;
		gbc.gridy = 2;
		gbc.weightx = 1.0;
		gbc.weighty = 0;
		getContentPane().add( mButtonPanel, gbc );

		gbc.fill = GridBagConstraints.HORIZONTAL;
		gbc.gridx = 0;
		gbc.gridy = 3;
		gbc.weightx = 1.0;
		gbc.weighty = 0;
		getContentPane().add( mResponsePanel, gbc );

		
		mCreateButton.addActionListener( mListener );

		setVisible( true );
		}
		

	
	private class CreatorListener implements ActionListener {

		// implements ActionListener.actionPerformed
		public void actionPerformed( ActionEvent inActionEvent ) {
			String instanceName = mInstanceNameField.getText();

			if( instanceName.equals( "" ) ) {
				mResponseLabel.setText( "invalid instance name" );
				return;
				}
			else {
				int index = mClassList.getSelectedIndex();
				if( index != -1 ) {
					Class c = mClasses[ index ];
					try {
						mOS.createObject( c, instanceName );

						// creation was successful
						// clear error message
						mResponseLabel.setText( " " );
						}
					catch( ObjectCreationException inCreationE ) {
						mResponseLabel.setText(
							"error:  object creation failed:  " +
							inCreationE.getMessage() );
						}
					}
				else {
					mResponseLabel.setText(
						"class not selected" );
					}
				}
			
			}
		}

	
	
	}

