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
import javax.swing.ImageIcon;

import java.awt.Image;

import java.awt.FlowLayout;
import java.awt.event.*;


public class ImageTransmitter extends SimpleTransmitter {

	ImageIcon mIcon;
	
	/**
	 * Constructs a simple transmitter and displays its frame.
	 */
	public ImageTransmitter() {
		mAppName = "psi.client.ImageTransmitter";
		}



	/**
	 * Sets up the frame contents.
	 *
	 * This is called by the constructor, and is
	 * meant to be overridden by subclasses.
	 */
	protected void setupFrame() {
		super.setupFrame();
		mLabel.setText( "" );
		}
	

	
	// overrides displayTarget of SimpleTransmitter
	protected void displayTarget( int inTarget ) {
		// load the image
		File imageFile = ( mImageFolder.listFiles() )[ inTarget ];
				
		mIcon = new ImageIcon( imageFile.getAbsolutePath() );

		// scale the image
		Image iconImage = mIcon.getImage();
		Image scaledImage = null;

		int maxImageWidth = getWidth() - 50;
		int maxImageHeight = getHeight() - 50;
		
		
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

		mIcon.setImage( scaledImage );
		
		mLabel.setIcon( mIcon );
		
		}

	
	
		
	public static void main( String inArgs[] ) {
		ImageTransmitter t = new ImageTransmitter();
		t.runTransmitter( inArgs );
		}

	
	
	}



