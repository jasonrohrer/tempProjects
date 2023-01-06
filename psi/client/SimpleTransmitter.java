/*
 * Modification History
 *
 * 2001-September-26		Jason Rohrer
 * Created.
 */


package psi.client;

import java.net.*;
import java.io.*;

import javax.swing.JFrame;
import javax.swing.JLabel;

import java.awt.FlowLayout;
import java.awt.event.*;


public class SimpleTransmitter extends JFrame {

	protected String mAppName = "psi.client.SimpleTransmitter";

	protected JLabel mLabel = new JLabel( "." );

	protected File mImageFolder;
	

	
	/**
	 * Constructs a simple transmitter and displays its frame.
	 */
	public SimpleTransmitter() {
		super( "Transmitter" );
		setupFrame();
		}



	/**
	 * Sets up the frame contents.
	 *
	 * This is called by the constructor, and is
	 * meant to be overridden by subclasses.
	 */
	protected void setupFrame() {
		getContentPane().setLayout( new FlowLayout( FlowLayout.CENTER ) );
		getContentPane().add( mLabel );

		setSize( 400, 200 );
		}
	

	
	/**
	 * Displays a target.
	 *
	 * @param inTarget the index of the target to display.
	 */
	protected void displayTarget( int inTarget ) {
		mLabel.setText( "" + inTarget );		}


	
	/**
	 * Runs the transmitter.
	 *
	 * @param inArgs an array of command line arguments.
	 */
	protected void runTransmitter( String inArgs[] ) {
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
			output.writeUTF( "transmitter" );


			// now participate in trials

			// display our frame
			setVisible( true );
			
			while( true ) {
				// simply display the target for each trial
				displayTarget( input.readInt() );
				}
			
			}
		catch( IOException inE ) {
			System.out.println( "Connection to server failed." );
			System.exit( 1 );
			}

		}

	
		
	public static void main( String inArgs[] ) {
		SimpleTransmitter t = new SimpleTransmitter();
		t.runTransmitter( inArgs );
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



