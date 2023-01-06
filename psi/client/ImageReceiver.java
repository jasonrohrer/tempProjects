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
import javax.swing.JButton;
import javax.swing.JPanel;
import javax.swing.BoxLayout;
import javax.swing.ImageIcon;

import java.awt.Image;

import java.awt.FlowLayout;
import java.awt.GridLayout;
import java.awt.event.*;


public class ImageReceiver extends SimpleReceiver {




	/**
	 * Constructs a simple receiver and shows its frame.
	 */
	public ImageReceiver() {
		mAppName = "psi.client.ImageReceiver";

		}



	/**
	 * Sets up the frame contents.
	 *
	 * This is called by the constructor, and is
	 * meant to be overridden by subclasses.
	 */
	protected void setupFrame() {
		super.setupFrame();
		for( int i=0; i<4; i++ ) {
			mButtons[i].setText( "" );
			}
		}

	
	
	// overrides getClickIndex of SimpleReceiver
	protected int getClickIndex( int inTestSet[] ) {
		
		// set the buttons to our test set
		for( int i=0; i<4; i++ ) {
			// load the image
			File imageFile = ( mImageFolder.listFiles() )[ inTestSet[i] ];
				
			ImageIcon icon = new ImageIcon( imageFile.getAbsolutePath() );

			// scale the image
			Image iconImage = icon.getImage();
			Image scaledImage = null;

			int maxImageWidth = mButtons[i].getWidth() - 5;
			int maxImageHeight = mButtons[i].getHeight() - 5;
		
		
			float widthFraction = iconImage.getWidth( null ) /
				(float)maxImageWidth;
			float heightFraction = iconImage.getHeight( null ) /
				(float)maxImageHeight;

			float scaleFactor = 1.0f;
		
			if( widthFraction > heightFraction ) {
				scaleFactor = 1.0f / widthFraction;
				}
			else {
				scaleFactor = 1.0f / heightFraction;
				}
			scaledImage = iconImage.getScaledInstance(
				(int)( iconImage.getWidth( null ) * scaleFactor ),
				(int)( iconImage.getHeight( null ) * scaleFactor ),
				Image.SCALE_FAST );

			icon.setImage( scaledImage );
		
			mButtons[i].setIcon( icon );

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


	
	public static void main( String inArgs[] ) {
		ImageReceiver receiverFrame = new ImageReceiver();
		receiverFrame.runReceiver( inArgs );
		
		}

	
	
	}



