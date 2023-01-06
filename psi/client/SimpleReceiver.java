/*
 * Modification History
 *
 * 2001-September-26		Jason Rohrer
 * Created.
 *
 * 2001-October-5		Jason Rohrer
 * Changed to use the new protocol which shows the correct
 * answer to the receiver after each trial.
 *
 * 2001-October-19		Jason Rohrer
 * Changed to support the protocol for skipping trials.
 */


package psi.client;

import java.net.*;
import java.io.*;

import javax.swing.JFrame;
import javax.swing.JButton;
import javax.swing.JPanel;
import javax.swing.BoxLayout;


import java.awt.FlowLayout;
import java.awt.BorderLayout;
import java.awt.GridLayout;
import java.awt.event.*;

import java.awt.Color;


public class SimpleReceiver extends JFrame {

	protected String mAppName = "psi.client.SimpleReceiver";

	
	protected JButton mButtons[] = new JButton[4];

	protected final int mLastClick[] = new int[1];

	// for signalling when clicks have occurred
	protected final Semaphore mSemaphore = new Semaphore();

	protected File mImageFolder;


	
	/**
	 * Constructs a simple receiver and shows its frame.
	 */
	public SimpleReceiver() {
		super( "Receiver" );

		setupFrame();
		}



	/**
	 * Sets up the frame contents.
	 *
	 * This is called by the constructor, and is
	 * meant to be overridden by subclasses.
	 */
	protected void setupFrame() {
		//getContentPane().setLayout( new FlowLayout( FlowLayout.CENTER ) );
		//getContentPane().setLayout( new BoxLayout( getContentPane(),
		//										   BoxLayout.X_AXIS ) );
		JPanel mainButtonPanel = new JPanel();
		mainButtonPanel.setLayout( new GridLayout( 2, 2 ) );

		JPanel skipButtonPanel = new JPanel();
		skipButtonPanel.setLayout( new FlowLayout( FlowLayout.CENTER ) );
		
		getContentPane().setLayout( new BorderLayout() );

		getContentPane().add( mainButtonPanel, BorderLayout.CENTER );
		getContentPane().add( skipButtonPanel, BorderLayout.SOUTH );
		
		
		for( int i=0; i<4; i++ ) {
			//JPanel panel = new JPanel( new FlowLayout( FlowLayout.CENTER ) );
			//getContentPane().add( panel );
			
			mButtons[i] = new JButton( "." );
			mainButtonPanel.add( mButtons[i] );

			final int finalI = i;
			
			mButtons[i].addActionListener(
				new ActionListener() {
						public void actionPerformed( ActionEvent inE ) {
							mLastClick[0] = finalI;
							mSemaphore.wakeUp();
							}
						} );
			}

		JButton skipButton = new JButton( "Skip" );
		skipButtonPanel.add( skipButton );

		// set last click to -1 for skip
		skipButton.addActionListener(
			new ActionListener() {
					public void actionPerformed( ActionEvent inE ) {
						mLastClick[0] = -1;
						mSemaphore.wakeUp();
						}
					} );
		
		
		setSize( 400, 200 );
		}

	

	/**
	 * Used to signal that button clicks have occurred.
	 */
	protected class Semaphore {
		
		public synchronized void waitFor()
			throws InterruptedException {
			
			this.wait();
			}

		
		public synchronized void wakeUp() {
			this.notify();
			}
		
		}
	

	
	/**
	 * Gets the index of the button that the user clicks.
	 * Waits for the user to click a button.
	 *
	 * @param inTestSet an integer array indexing the
	 *   4 element test set to be displayed.
	 * @return the index that the user clicked.
	 */
	protected int getClickIndex( int inTestSet[] ) {
		
		// set the buttons to our test set
		for( int i=0; i<4; i++ ) {
			mButtons[i].setText( "" + inTestSet[i] );
			}
		try {
			mSemaphore.waitFor();
			return mLastClick[0];
			}
		catch( InterruptedException inE ) {
			System.out.println( "Interrupted while waiting for user click." );
			return 0;
			}
		}



	/**
	 * Shows the correct target to the user.
	 * Waits for the user to click a button before returning.
	 *
	 * @param inTarget the index of the button to show as the target.
	 */
	protected void showCorrectTarget( int inTarget ) {

		// make the target button green
		Color oldBackground = mButtons[ inTarget].getBackground();
		mButtons[ inTarget ].setBackground(
			new Color( 0, 1.0f, 0 ) );

		// now wait for the user to click before proceeding
		try {
			mSemaphore.waitFor();
			}
		catch( InterruptedException inE ) {
			System.out.println( "Interrupted while waiting for user click." );
			}

		// restore the button color
		mButtons[ inTarget ].setBackground( oldBackground );
		}
	


	/**
	 * Runs the receiver.
	 *
	 * @param inArgs an array of command line arguments.
	 */
	protected void runReceiver( String inArgs[] ) {
		
		if( inArgs.length != 3 ) {
			usage( mAppName );
			}

		// parse our input arguments
		String address = inArgs[0];
		int port = 5000;
		mImageFolder = new File( inArgs[2] );
		
		try{
			port = Integer.parseInt( inArgs[1] );
			}
		catch( NumberFormatException e ) {
			System.out.println( "port_number must be a positive integer: " +
								inArgs[0] );
			usage( mAppName );
			}

		Socket sock = null;
		try {
			System.out.println( "Connecting to server  "
								+ address + ":" + port + " ..." );
			sock = new Socket( address, port );
			}
		catch( IOException inE ) {
			System.out.println( "Connection to server failed:  "
								+ address + ":" + port );
			usage( mAppName );
			}

		try {
			System.out.println( "Connection established." );
			DataOutputStream output =
				new DataOutputStream( sock.getOutputStream() );
			DataInputStream input =
				new DataInputStream( sock.getInputStream() );

			System.out.println( "Sending greeting..." );
			output.writeUTF( "receiver" );


			// now participate in trials

			// display our frame
			setVisible( true );
			
			while( true ) {
				// read the test set from the server
				int testSet[] = new int[4];
				for( int i=0; i<4; i++ ) {
					testSet[i] = input.readInt();
					}

				// wait for the user's click
				int clickIndex = getClickIndex( testSet );
				int prediction;
				if( clickIndex == -1 ) {
					prediction = -1;
					}
				else {
					prediction =
						testSet[ clickIndex ];
					}
				
				// send the prediction
				output.writeInt( prediction );

				// read the correct target and display it
				int target = input.readInt();
				int targetButton = 0;

				// find the index to display as the target
				for( int i=0; i<4; i++ ) {
					if( testSet[i] == target ) {
						targetButton = i;
						}
					}
				
				showCorrectTarget( targetButton );

				
				// send a ready signal
				output.writeInt( 1 );
				}
			
			}
		catch( IOException inE ) {
			System.out.println( "Connection to server failed." );
			System.exit( 1 );
			}

		}

	
	
	public static void main( String inArgs[] ) {
		SimpleReceiver receiverFrame = new SimpleReceiver();
		receiverFrame.runReceiver( inArgs );
		
		}



	/**
	 * Prints a usage message and exits.
	 *
	 * @param inAppName the name of this application.
	 */
	protected void usage( String inAppName ) {
		System.out.println( "Usage:" );
		System.out.println( "\t" + inAppName +
							" server_address server_port_number "
							+ "image_folder" );
		System.out.println( "Example:" );
		System.out.println( "\t" + inAppName +
							" psi.mydomain.com 5000 images" );

		System.exit( 1 );
		}

	
	
	}



