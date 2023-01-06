#include "minorGems/network/Socket.h"



/**
 * Starts the stream network.
 *
 * @param inPort the port to listen for connections on.
 */
void startNetwork( int inPort );



void stopNetwork();



/**
 * Executes one step of stream network operations.
 *
 * This includes:
 * --Receiving available data from our data source.
 * --Accepting incoming connections.
 * --Sending data chunks to clients that need them.
 *
 * @param inTimeLimitInMilliseconds the maximum number of milliseconds
 *   that this network step should take.
 */
void executeNetworkStep( unsigned long inTimeLimitInMilliseconds );



/**
 * Adds a socket to our list of clients.
 *
 * @param inSocket the socket to add.
 *   Will be destroyed by this layer.
 *
 * @return the index of this new socket in our list.
 */
int addClientSocket( Socket *inSocket );



/**
 * Sets the socket to receive data from.
 * This call may either add a new socket that this layer does not know about
 * or change to a different known socket as a source.
 *
 * Note that calling setSourceSocket with our current source socket as an
 * argument re-initiates the stream from that source.
 *
 * @param inSocket the socket to receive from.
 *   Will be destroyed by this layer eventually.
 */
void setSourceSocket( Socket *inSocket );



/**
 * Moves on to the next source channel.
 */
void nextChannel();

