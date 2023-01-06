/*
 * Modification History
 *
 * 2002-January-9   Jason Rohrer
 * Created.
 *
 * 2002-January-11   Jason Rohrer
 * Added chord display.
 */


import javax.swing.JFrame;
import javax.swing.JTextArea;
import javax.swing.JPanel;
import javax.swing.JLabel;

import java.awt.event.KeyListener;
import java.awt.event.KeyEvent;

import java.awt.Color;

import javax.swing.BoxLayout;
import javax.swing.SwingUtilities;

import java.util.ArrayList;

public class ChordKeyFrame extends JFrame {

	char keyChars[] = { 'a', 's', 'd', 'f', 'j', 'k', 'l', ';' };
	
	int numChords = 256;
	
	char chords[] = new char[ numChords ];


	
	private void initChords() {
		int i;
		for( i=0; i<numChords; i++ ) {
			chords[i] = (char)i;
			}

		// bubble sort
		for( int a=0; a<numChords; a++ ) {
			for( int b=0; b<numChords-1; b++ ) {

				int numThis = getNumBitsOn( chords[ b ] );
				int numNext = getNumBitsOn( chords[ b+1 ] );

				if( numThis < numNext ) {
					char temp = chords[ b ];
					chords[ b ] = chords[ b+1 ];
					chords[ b+1 ] = temp;
					}
				}
			}
		}	


	
	private int getNumBitsOn( char inChar ) {
		int numBits = 0;
		for( int i=0; i<8; i++ ) {
			if( 1 == ( ( inChar >> i ) & 1 ) ) {
				numBits++;
				}
			}
		
		return numBits;
		}

	
	
	JTextArea mTextArea = new JTextArea();
	JPanel mBottomPanel = new JPanel();

	JPanel mChordDisplayPanels[] = new JPanel[8];
	
	
	public ChordKeyFrame() {
		super( "ChordKey Tester" );

		initChords();

		JPanel chordPanel = new JPanel();
		chordPanel.setLayout(
			new BoxLayout( chordPanel, BoxLayout.X_AXIS ) );

		for( int i=0; i<8; i++ ) {
			mChordDisplayPanels[i] = new JPanel();
			mChordDisplayPanels[i].add( new JLabel( "" + keyChars[i] ) );

			mChordDisplayPanels[i].setBackground( Color.white );
			
			chordPanel.add( mChordDisplayPanels[i] );
			}

		
		getContentPane().setLayout(
			new BoxLayout( getContentPane(), BoxLayout.Y_AXIS ) );

		mTextArea.setEditable( false );

		getContentPane().add( chordPanel );
		getContentPane().add( mTextArea );

		mBottomPanel.add( new JLabel( "Test" ) );
		getContentPane().add( mBottomPanel );
		
		setSize( 500, 500 );
		setVisible( true );

		addKeyListener( new ChordKeyListener() );
		}


	
	protected class ChordKeyListener implements KeyListener {

		ArrayList mCurrentKeyList = new ArrayList();
		
		long lastTime = System.currentTimeMillis();
		boolean buildingChord = false;
		
		
		public void keyPressed( KeyEvent inEvent ) {
			
			char keyChar = inEvent.getKeyChar();

			Character charWrapper = new Character( keyChar );

			if( !buildingChord ) {
				buildingChord = true;
			
				mCurrentKeyList.add( charWrapper );

				lastTime = System.currentTimeMillis();

				Thread dumpThread = new Thread() {
						public void run() {
							try {
								sleep( 100 );
								}
							catch( InterruptedException inException ) {
								}

							SwingUtilities.invokeLater(
								new Runnable() {
										public void run() {
											dumpChord();
											}
										}
							);
							
							buildingChord = false;
							}
						};
				dumpThread.start();				
				}
			else {
				mCurrentKeyList.add( charWrapper );
				}
			/*
				long thisTime = System.currentTimeMillis();

				if( thisTime - lastTime < 100 ) {

					mCurrentKeyList.add( charWrapper );

					lastTime = thisTime;
					}
				else {
					// too much time passed, dump the chord
					dumpChord();
					buildingChord = false;
					}
				}
			*/
			}


		
		public void keyReleased( KeyEvent inEvent ) {
			
			}


		
		protected void dumpChord() {	
			
			char bitChar = 0;
			/*
			if( mCurrentKeyList.size() == 1 ) {
				bitChar = ( (Character)(
					mCurrentKeyList.get(0) ) ).charValue();
				}
			else {
				}
			*/
			
			for( int i=0; i<8; i++ ) {
				if( mCurrentKeyList.contains(
					new Character( keyChars[i] ) ) ) {

					bitChar = (char)( bitChar | ( 1 << i ) );
					}
				
				}
			
			for( int i=0; i<8; i++ ) {
				if( ( bitChar >> i & 1 ) == 1 ) {
					mChordDisplayPanels[i].setBackground( Color.red );
					}
				else {
					mChordDisplayPanels[i].setBackground( Color.white );
					}
				}

			
				/*
			if( mCurrentKeyList.contains( new Character( 'a' ) )
				&& mCurrentKeyList.contains( new Character( 's' ) ) ) {
				bitChar = 't';
				}
			else {
				bitChar = 'y';
				}
			*/		
				
			
			String currentString = mTextArea.getText();

			mTextArea.setText(
				currentString.concat( String.valueOf( bitChar ) ) );
			
			mCurrentKeyList.clear();

			}



		public void keyTyped( KeyEvent inEvent ) {

			}



		}


	
	public static void main( String inArgs[] ) {
		new ChordKeyFrame();
		
		}


	
	}
