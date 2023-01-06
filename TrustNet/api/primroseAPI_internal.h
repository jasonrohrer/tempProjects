/*
 * Modification History
 *
 * 2004-November-11   Jason Rohrer
 * Created.
 */



/**
 * Internal functions used in the Primrose API.
 *
 * All calls are thread-safe.
 *
 * All function parameters must be destroyed by caller.
 * All string parameters must be \0-terminated.
 *
 * @author Jason Rohrer.
 */



#include "Primrose/api/primroseAPI.h"

#include "minorGems/network/Socket.h"



/**
 * Checks the signature of a message and gets the sender ID.
 *
 * @param inTextHeader the ASCII header portion of the message.
 * @param inBinaryBlock the optional binary block in the message, or NULL
 *   if the message has no binary block.
 * @param inBinaryBlockLength the size of the binary block.  Must be 0
 *   if inBinaryBlock is NULL.
 * @param inSignatureBlock the binary signature block.
 * @param inSignatureBlockLength the size of the signature block.
 *
 * @return the sender ID, or NULL if the signature is invalid.
 */
struct UserID *primrose_internalCheckSignatureAndGetSenderID(
    char *inTextHeader,
    unsigned char *inBinaryBlock, unsigned long inBinaryBlockLength,
    unsigned char *inSignatureBlock, unsigned long inSignatureBlockLength );



/**
 * Checks the signature of a message and gets the sender ID.
 *
 * @param inTextHeader the ASCII header portion of the message.
 * @param inBinaryBlock the optional binary block in the message, or NULL
 *   if the message has no binary block.
 * @param inBinaryBlockLength the size of the binary block.  Must be 0
 *   if inBinaryBlock is NULL.
 * @param inSignatureTextBlock the text signature block.
 *
 * @return the sender ID, or NULL if the signature is invalid.
 */
struct UserID *primrose_internalCheckSignatureAndGetSenderID(
    char *inTextHeader,
    unsigned char *inBinaryBlock, unsigned long inBinaryBlockLength,
    char *inSignatureTextBlock );



/**
 * Gets our signature block for a given message (signs it with our local key).
 *
 * @param inTextHeader the ASCII header portion of the message.
 * @param inBinaryBlock the optional binary block in the message, or NULL
 *   if the message has no binary block.
 * @param inBinaryBlockLength the size of the binary block.  Must be 0
 *   if inBinaryBlock is NULL.
 * @param inReceiverID the ID of the receiver.
 * @param outBlockLength pointer to where the length of the signature block
 *   should be returned.
 *
 * @return the binary signature block, or NULL if signing fails.
 */ 
unsigned char *primrose_internalGetBinarySignatureBlock(
    char *inTextHeader,
    unsigned char *inBinaryBlock, unsigned long inBinaryBlockLength,
    struct UserID *inReceiverID,
    unsigned long *outBlockLength );



/**
 * Gets our signature block for a given message (signs it with our local key).
 *
 * @param inTextHeader the ASCII header portion of the message.
 * @param inBinaryBlock the optional binary block in the message, or NULL
 *   if the message has no binary block.
 * @param inBinaryBlockLength the size of the binary block.  Must be 0
 *   if inBinaryBlock is NULL.
 * @param inReceiverID the ID of the receiver.
 *
 * @return the signature block as ASCII text, or NULL if signing fails.
 */ 
char *primrose_internalGetTextSignatureBlock(
    char *inTextHeader,
    unsigned char *inBinaryBlock, unsigned long inBinaryBlockLength,
    struct UserID *inReceiverID );



/**
 * Gets a socket connection to a node that may be firewalled.
 *
 * @param inIPAddress the IP address of the node.
 *   Must be destroyed by caller.
 * @param inTCPPort the TCP port of the node.
 * @param inUDPPort the UDP port of the node.
 * @param inFirewallStatus true if the node is firewalled, or false otherwise.
 * @param inUserID the ID of the user that we are trying to connect to.
 *   Must be destroyed by caller.
 */
Socket *primrose_internalGetConnection( char *inIPAddress,
                                        unsigned short inTCPPort,
                                        unsigned short inUDPPort,
                                        char inFirewallStatus,
                                        struct UserID *inUserID );



/**
 * Reads a block of data from a socket.
 *
 * @param inSocket the socket to read from.
 *   Must be destroyed by caller.
 * @param inLength the length to read, or -1 to read up to (and including
 *   the first \0 character.
 *
 * @return the block data, or NULL if reading fails.
 *   Length of block will be either inLength (if inLength >= 0) or
 *   determined by the position of the first \0 character.
 *   Must be destroyed by caller.
 */
unsigned char *primrose_internalReadBlock( Socket *inSocket,
                                           long inLength );
