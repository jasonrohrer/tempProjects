/**
 * Modification History
 *
 * 2003-January-10   Jason Rohrer
 * Created.
 */



import javax.swing.JFrame;
import javax.swing.JPanel;
import javax.swing.JLabel;

import javax.swing.JScrollPane;
import javax.swing.JList;

import javax.swing.JTextField;


import java.awt.BorderLayout;
import java.awt.FlowLayout;

import javax.swing.event.ListSelectionEvent;
import javax.swing.event.ListSelectionListener;


import java.util.Vector;

import java.net.Socket;
import java.io.IOException;
import java.net.UnknownHostException;

import java.io.InputStream;
import java.io.OutputStream;



/**
 * The main frame for hyperlit.
 *
 * @author Jason Rohrer
 */
class LinkLauncher extends JFrame implements Runnable {
        
    private String mLinkOpenerAddress = "localhost";
    private int mLinkOpenerPort = 9578;
    
    private String mBrowserCommandName = "opera";
    
    
    private JList mLinkList;
    private JLabel mListLabel = new JLabel( "Currently Active Links:" );

    private JTextField mBrowserField = new JTextField( "netscape", 20 );
    private JLabel mBrowserFieldLabel = new JLabel( "browser command:" );
    
        
    
    public static void main( String [] inArgs ) {

        new LinkLauncher();
        }


    
    /**
     * Constructs and displays a link launcher frame.
     */
    public LinkLauncher() {

        super( "Link Launcher" );

        getContentPane().setLayout( new BorderLayout() );

        getContentPane().add( mListLabel, BorderLayout.NORTH );

        
        mLinkList = new JList();
        
        JScrollPane linkListScrollPane = new JScrollPane( mLinkList );
        
        getContentPane().add( linkListScrollPane, BorderLayout.CENTER );


        JPanel browserFieldPanel = new JPanel(
            new FlowLayout( FlowLayout.CENTER ) );

        browserFieldPanel.add( mBrowserFieldLabel );
        browserFieldPanel.add( mBrowserField );

        getContentPane().add( browserFieldPanel, BorderLayout.SOUTH );


        
        ListListener listListener = new ListListener();
        mLinkList.addListSelectionListener( listListener );


        Link testLinkA = new Link( "test", "http://www.test.com" );
        Link testLinkB = new Link( "test2", "http://www.test2.com" );

        Vector linkVector = new Vector();
        linkVector.add( testLinkA );
        linkVector.add( testLinkB );

        setLinks( linkVector );

        Thread runningThread = new Thread( this );
        runningThread.start();
        

        setSize( 400, 500 );

        setVisible( true );
        }
    

    
    private InputStream mSockIn = null;
    private OutputStream mSockOut = null;


    
    // implements Runnable.run()
    public void run() {
        try {
            Socket sock = new Socket( mLinkOpenerAddress, mLinkOpenerPort );

            mSockIn = sock.getInputStream();
            mSockOut = sock.getOutputStream();


            // repeatedly read a link list
            while( true ) {

                // first, the number of links
                int numLinks = readInt( mSockIn );

                Vector linkVector = new Vector();

                // read title and URL for each link
                for( int i=0; i<numLinks; i++ ) {

                    String linkTitle = readString( mSockIn );
                    String linkURL = readString( mSockIn );

                    linkVector.add( new Link( linkTitle, linkURL ) );
                    }

                // replace the displayed links
                setLinks( linkVector );
                }
            
            }
        catch( UnknownHostException inE ) {
            System.out.println( "Unable to lookup hostname: " +
                                mLinkOpenerAddress );
            
            }
        catch( IOException inE ) {
            System.out.println( "A connection error occurred." );
            }
        System.exit( 0 );
        }

    

    private byte mIntByteArray[] = new byte[ 4 ]; 

    

    /**
     * Reads an integer in big-endian, 32-bit format.
     *
     * @param inStream the stream to read from.
     *
     * @return the integer read.
     *
     * @throws IOException if reading from the stream fails.
     */
    protected int readInt( InputStream inStream )
        throws IOException {

        // read as big endian
        
        inStream.read( mIntByteArray );

        return mIntByteArray[0] << 24 | mIntByteArray[1] << 16 |
               mIntByteArray[2] << 8  | mIntByteArray[3];  
        }



    /**
     * Writes an integer in big-endian, 32-bit format.
     *
     * @param inStream the stream to write to.
     * @param inInt the integer to write.
     *
     * @throws IOException if writing to the stream fails.
     */
    protected void writeInt( OutputStream inStream, int inInt )
        throws IOException {

        // write as big endian

        mIntByteArray[0] = (byte)( inInt >> 24 );
        mIntByteArray[1] = (byte)( inInt >> 16 );
        mIntByteArray[2] = (byte)( inInt >> 8 );
        mIntByteArray[3] = (byte)( inInt );

        inStream.write( mIntByteArray );
        }



    // URLs are limited to 255 characters
    // and link titles are likely to be even shorter
    private int mStringBufferLength = 300;
    private byte mStringBuffer[] = new byte[ mStringBufferLength ];

    
    
    /**
     * Reads a \0-terminated string.
     *
     * @param inStream the stream to read from.
     *
     * @return the string read.
     *
     * @throws IOException if reading from the stream fails.
     */
    protected String readString( InputStream inStream )
        throws IOException {

        int index = 0;

        byte readByte = (byte)( inStream.read() );

        while( readByte != '\0' && index < mStringBufferLength  ) {
            mStringBuffer[ index ] = readByte;
            index++;
            readByte = (byte)( inStream.read() );
            }

        // index is now one beyond the last byte in the string
        int length = index;

        return new String( mStringBuffer, 0, index );
        }


    
    /**
     * Writes a \0-terminated string.
     *
     * @param inStream the stream to write to.
     * @param inString the string to write
     *
     * @throws IOException if reading from the stream fails.
     */
    protected void writeString( OutputStream inStream, String inString )
        throws IOException {

        byte stringBytes[] = inString.getBytes();

        int numBytes = stringBytes.length;

        for( int i=0; i<numBytes; i++ ) {
            inStream.write( (int)( stringBytes[i] ) );
            }

        // write termination
        inStream.write( (int)( '\0' ) );        
        }

    

    /**
     * Opens a URL in the remote browser.
     *
     * @param inURL the URL to open.
     */
    protected void openURL( String inURL ) {

        System.out.println( "Opening " + inURL );

        try {
            mBrowserCommandName = mBrowserField.getText();
            
            writeString( mSockOut, mBrowserCommandName );
            writeString( mSockOut, inURL );
            }
        catch( IOException inE ) {
            System.out.println( "Failed to write link open command to " +
                                mLinkOpenerAddress + ":" + mLinkOpenerPort );
            System.exit( 0 );
            }
        }



    /**
     * Sets the list of links to display.
     *
     * @param inLinkVector a vector of Link objects.
     */
    protected void setLinks( Vector inLinkVector ) {

        mLinkList.setListData( inLinkVector );
        }
    


    /**
     * A link object.
     *
     * @author Jason Rohrer.
     */
    protected class Link {

        public String mName;
        public String mURL;


        
        /**
         * Constructs a link.
         *
         * @param inName the title of the link.
         * @param inURL the link's URL.
         */
        public Link( String inName, String inURL ) {
            mName = inName;
            mURL = inURL;
            }


        
        // overrides Object.toString
        public String toString() {
            return mName;
            }

        

        }


    
    protected boolean mIgnoreListChange = false;
    protected int currentlySelectedIndex = -1;
    
    
            
    

    /**
     * A listener for mLinkList.
     *
     * @author Jason Rohrer.
     */
    protected class ListListener implements ListSelectionListener {


        
        // implements ListSelectionListener.valueChanged
        public void valueChanged( ListSelectionEvent inEvent ) {

            if( mIgnoreListChange ) {
                return;
                }
                        
            Object selected = mLinkList.getSelectedValue();
            int selectedIndex = mLinkList.getSelectedIndex();
            
            if( selected != null
                && selectedIndex != currentlySelectedIndex ) {

                Link currentLink = (Link)selected;

                openURL( currentLink.mURL );

                currentlySelectedIndex = selectedIndex;
                }
            
            }


        
        }
    



    }
