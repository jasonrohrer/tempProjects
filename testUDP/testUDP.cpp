/*
 * Modification History
 *
 * 2004-November-2   Jason Rohrer
 * Fixed bug in printing remote port number.
 */


#include <sys/types.h>
#include <sys/socket.h>

#include <netinet/in.h>

#include <arpa/inet.h>

#include "minorGems/util/stringUtils.h"


/*
 * Based on information from:
 * "A Crash Course in UNIX TCP/IP Socket Programming"
 * by John Selbie
 * http://www.fortunecity.com/skyscraper/arpanet/6/cc.htm
 */


/*
  Cannot believe that this works.
  
  This solves the p2p firewall problem in many cases.

  If a node sends out a UDP message and then waits for a response, the
  NAT leaves a port open for quite a while so that it can route the
  response to the appropriate internal address... currently measuring how long
  it leaves it open.... not excellent:

  Less than a minute.... more than 55 seconds... probably 60 seconds.

  So, we can use IP:port combos that *we* receive searches from to send
  our searches directly back, even to nodes behind NATs and firewalls.
  When we get results, we can then initiate UDP-based DL requests back
  through the open port on the NAT... if the user starts the DL soon enough.  

  In my tests, UDP messages seem to be dropped if they have payloads that
  are larger than 1464 bytes in size.  This seems to be true on local subnets
  as well as across the internet and through NAT routers.

  However, I have read elsewhere that 576 bytes is the minimum max
  transmittable unit (MTU) for IP packet routers, and that some nodes on
  the internet may only be able to handle UDP packets with payloads that are
  at most 512 bytes in size (plus headers to hit the 576 byte limit).
  Any IP router that is to-spec must be able to handle UDP messages with
  payloads that are at least 512 bytes.

  Probably best to stick with 512-byte messages.

  --Each search result can be sent back as a separate UDP message.
  --What about file chunk sizes?  Maybe if chunk just comes with the the
    20-byte SHA1 hash and a 32-bit chunk number, we can devote the remaining
    488 bytes to the payload... But what about signatures?

  Signature follows payload in a UDP message:
  -- 64-bit User ID (unique for each user)
  -- 64-bit Nonce that is never reused (starts at 0 and counts up)
  -- Signature = SHA1( Nonce | Password | MessagePayload )

  Signature uses up 8 + 8 + 20 = 36 bytes
  

  Thus, message payload is 476 bytes.

  For a file chunk message, this gives us 452 bytes of file data.

  Do not encode the size of the payload in the UDP message itself, because
  this can be computed from the size of the UDP message (-36 signature bytes
  and for a file chunk, -24 chunk ID bytes).
 */




int main( int inNumArgs, char **inArgs ) {

    int sendPort = 112004;
    int receivePort = 112008;
    int testMessageLength = 600;


    // read ports from args, if it is correct, else leave default port value
    if( inNumArgs > 1 ) {
        sscanf( inArgs[1], "%d", &sendPort );
        }
    if( inNumArgs > 2 ) {
        sscanf( inArgs[2], "%d", &receivePort );
        }
    if( inNumArgs > 3 ) {
        sscanf( inArgs[3], "%d", &testMessageLength );
        }
    
    
    char *sendToAddress = NULL;
    
    if( inNumArgs > 4 ) {
        sendToAddress = inArgs[4];
        }

    // create a UDP socket 
    int sock;
    sock = socket( AF_INET, SOCK_DGRAM, 0 );


    // bind to receivePort    
    struct sockaddr_in bindAddress;
    
    bindAddress.sin_family = AF_INET;
    
    bindAddress.sin_port = htons( receivePort );
    
    bindAddress.sin_addr.s_addr = INADDR_ANY;
    
    bind( sock, (struct sockaddr *)&bindAddress, sizeof(bindAddress) );


    int sentNumber = 0;
    
    if( sendToAddress != NULL ) {

        // send the first message


        // pack the address into a sockaddr_in structure
        struct sockaddr_in toAddress;
        
        toAddress.sin_family = AF_INET;
        
        // htons for network byte order
        toAddress.sin_port = htons( sendPort ); 
        
        toAddress.sin_addr.s_addr = inet_addr( sendToAddress );
         

        
        //char *message = autoSprintf( "Hello message #%d", sentNumber );
        //int messageLength = strlen( message ) + 1;

        int messageLength = testMessageLength;
        char *message = new char[ messageLength ];
        
        printf( "Sending first message\n" );
        
        sendto( sock, message, messageLength, 0,
                (struct sockaddr *)( &toAddress ),
                sizeof( toAddress ) );

        delete [] message;

        sentNumber++;
        }


    // now receive messages and respond to them

    while( true ) {

        struct sockaddr_in fromAddress;

        char receiveBuffer[2*testMessageLength];

        int result;

        unsigned int fromAddressLength = sizeof( fromAddress );

        int numReceived = recvfrom( sock, receiveBuffer,
                                    2*testMessageLength, 0,
                                    (struct sockaddr *)( &fromAddress),
                                    &fromAddressLength);

        printf( "Got %d byte message from %s:%d\n",
                numReceived,
                inet_ntoa( fromAddress.sin_addr ),
                ntohs( fromAddress.sin_port ) );


        printf( "Sending response\n" );

        //char *message = autoSprintf( "Hello message #%d", sentNumber );
        //int messageLength = strlen( message ) + 1;

        int messageLength = testMessageLength;
        char *message = new char[ messageLength ];

        
        sendto( sock, message, messageLength, 0,
                (struct sockaddr *)( &fromAddress ),
                sizeof( fromAddress ) );

        delete [] message;

        sentNumber++;


        // send extra copy to same address to see if it gets through
        message = autoSprintf( "(double) Hello message #%d", sentNumber );
                
 
        printf( "Sending double response\n" );
        
        sendto( sock, message, messageLength, 0,
                (struct sockaddr *)( &fromAddress ),
                sizeof( fromAddress ) );

        delete [] message;

        sentNumber++;

        
        }

    return 0;
    }
