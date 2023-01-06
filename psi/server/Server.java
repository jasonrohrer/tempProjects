/*
 * Modification History
 *
 * 2001-September-26		Jason Rohrer
 * Created.
 *
 * 2001-October-4		Jason Rohrer
 * Changed to avoid duplicate images.
 *
 * 2001-October-5		Jason Rohrer
 * Changed protocol to send correct target to receiver after each trial.
 *
 * 2001-October-19		Jason Rohrer
 * Changed to allow receiver to skip trials.
 * Changed to avoid repeated images.  Has not been finished yet.
 * Finished change to avoid repeats.
 */


package psi.server;

import java.net.*;
import java.io.*;

import java.util.Random;


public class Server {

	static String sAppName = "psi.server.Server";

	static boolean sImageUsed[];

	static Random sRand = new Random( System.currentTimeMillis() );

	static int sNumFiles;
	

	
	/**
	 * Counts the number of remaining unused images.
	 *
	 * @return the number of unused images in the image set.
	 */
	static int countUnusedImages() {
		int count = 0;
		for( int i=0; i<sImageUsed.length; i++ ) {
			if( !( sImageUsed[i] ) ) {
				count++;
				}
			}
		return count;
		}



	/**
	 * Clears the used image flags.
	 */
	static void clearUsedImageMarkers() {
		for( int i=0; i<sImageUsed.length; i++ ) {
			sImageUsed[i] = false;
			}
		}


	/**
	 * Gets the random index of an unused image and marks it as used.
	 *
	 * @return the index of an unused image, or -1 if no unused
	 *   images remain.
	 */
	static int getRandomUnusedImageIndex() {
		if( countUnusedImages() < 1 ) {
			return -1;
			}
		
		int unusedIndex = -1;
		while( true ) {
			unusedIndex = sRand.nextInt( sNumFiles );
			if( !( sImageUsed[unusedIndex] ) ) {
				sImageUsed[unusedIndex] = true;
				return unusedIndex;
				}
			}
		}	

	
	public static void main( String inArgs[] ) {

		if( inArgs.length != 3 ) {
			usage( sAppName );
			}

		// parse our input arguments
		int port = 5000;
		File imageFolder = new File( inArgs[1] );
		int numTrials = 10;
		
		try{
			port = Integer.parseInt( inArgs[0] );
			}
		catch( NumberFormatException e ) {
			System.out.println( "port_number must be a positive integer: " +
								inArgs[0] );
			usage( sAppName );
			}

		try{
			numTrials = Integer.parseInt( inArgs[2] );
			}
		catch( NumberFormatException e ) {
			System.out.println( "num_trials must be a positive integer: " +
								inArgs[2] );
			usage( sAppName );
			}

		// take stock of our image folder
		sNumFiles = ( imageFolder.listFiles() ).length;

		if( sNumFiles < 4 ) {
			// not enough images
			System.out.println(
				"not enough images in the image folder (minimum=4): " +
				sNumFiles );
			usage( sAppName );
			}

		sImageUsed = new boolean[sNumFiles];
		
		// create a server socket on our port w/ a queue of 2
		ServerSocket serverSock = null;
		try {
			serverSock = new ServerSocket( port, 2 );
			}
		catch( IOException inE ) {
			System.out.println( "Error binding to port " + port );
			usage( sAppName );
			}
		
		System.out.println( "Listening for connections on port " + port );

		
		// accept two connections
		Socket sockA;
		Socket sockB;
		try {
			sockA = serverSock.accept();
			System.out.println( "Connection A received... "
								+ "listening for second connection" );
			sockB = serverSock.accept();
			System.out.println( "Connection B received. " );
			}
		catch( IOException inE ) {
			System.out.println(
				"Server socket failed while waiting for connections." );
			return;
			}

		
		int hitCount = 0;
		int skipCount = 0;
		int trialCount = 0;
		
		// figure out which socket belongs to which client
		try {
			DataOutputStream outputA =
				new DataOutputStream( sockA.getOutputStream() );
			DataInputStream inputA =
				new DataInputStream( sockA.getInputStream() );
			
			DataOutputStream outputB =
				new DataOutputStream( sockB.getOutputStream() );
			DataInputStream inputB =
				new DataInputStream( sockB.getInputStream() );

			System.out.println(
				"Waiting for greetings from connected clients." );
			String messageA = inputA.readUTF();
			String messageB = inputB.readUTF();
			System.out.println( "Both greetings received." );

			
			// stream names corresponding to client type
			DataOutputStream receiverOut = null;
			DataInputStream receiverIn = null;
			DataOutputStream transmitterOut = null;
			DataInputStream transmitterIn = null;
			
			if( messageA.equals( "receiver" ) ) {
				receiverOut = outputA;
				receiverIn = inputA;
				}
			if( messageB.equals( "receiver" ) ) {
				receiverOut = outputB;
			 	receiverIn = inputB;
				}
			if( messageA.equals( "transmitter" ) ) {
				transmitterOut = outputA;
				transmitterIn = inputA;
				}
			if( messageB.equals( "transmitter" ) ) {
				transmitterOut = outputB;
			 	transmitterIn = inputB;
				}

			if( receiverOut == null || transmitterOut == null ) {
				System.out.println( "Bad greeting messages received." );
				System.exit( 1 );
				}

			// we have the clients properly identified

			// seed our random generator
			Random rand = new Random( System.currentTimeMillis() );
			
			
			// start running trials
			

			
			for( int i=0; i<numTrials; i++ ) {
				int numUnusedImages = countUnusedImages();
				if( numUnusedImages < 4 ) {
					// not enough images left for another set
					// reset
					clearUsedImageMarkers();
					}
				
				// pick a file number and send it to the transmitter
				int targetFile = getRandomUnusedImageIndex();
				
				transmitterOut.writeInt( targetFile );

				// now we need to pick 3 more integers and mix
				// them up with our target integer

				int testSet[] = new int[4];

				// first, pick a spot for our target integer
				int targetSpot = sRand.nextInt( 4 );
				testSet[ targetSpot ] = targetFile;

				// fill in the other 3 spots with random file numbers
				for( int j=0; j<4; j++ ) {
					if( j != targetSpot ) {
						testSet[j] = getRandomUnusedImageIndex();
						}
					}

				for( int j=0; j<4; j++ ) {
					receiverOut.writeInt( testSet[j] );
					}

				// now read the prediction
				int prediction = receiverIn.readInt();

				// now send the correct target
				// to the receiver
				receiverOut.writeInt( targetFile );

				// wait for the reciever's "continue" signal
				receiverIn.readInt();
				
				
				System.out.print( "Test set: " );
				for( int j=0; j<4; j++ ) {
					System.out.print( testSet[j] +"\t" );
					}
				System.out.print( "target: " + targetFile );

				System.out.print( "\tprediction: " + prediction );

				if( prediction == targetFile ) {
					System.out.println( "\thit" );
					hitCount++;
					}
				else if( prediction == -1 ) {
					System.out.println( "\tskip" );
					skipCount++;
					}
				else {
					System.out.println( "\tmiss" );
					}

				trialCount++;
				}

			// done with trials
			System.out.println( "Total Hit ratio = " +
								(float)hitCount / (float)trialCount );
			System.out.println( "Hit ratio (skipped trials ignored) = " +
								(float)hitCount /
								(float)( trialCount - skipCount ) );
			
			}
		catch( IOException inE ) {
			System.out.println( "Connection to one of the clients failed." );

			// print stats here too
			System.out.println( "Total Hit ratio = " +
								(float)hitCount / (float)trialCount );
			System.out.println( "Hit ratio (skipped trials ignored) = " +
								(float)hitCount /
								(float)( trialCount - skipCount ) );
			
			System.exit( 1 );
			}
		}



	/**
	 * Prints a usage message and exits.
	 *
	 * @param inAppName the name of this application.
	 */
	protected static void usage( String inAppName ) {
		System.out.println( "Usage:" );
		System.out.println( "\t" + inAppName +
							" port_number image_folder num_trials" );
		System.out.println( "Example:" );
		System.out.println( "\t" + inAppName + " 5000 images 100" );

		System.exit( 1 );
		}

	
	
	}



