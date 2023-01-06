#include "DataSource.h"


/**
 * Plays an mpeg stream from a data source and relays the stream to clients
 * that connect on a server port.
 *
 * @param inSource the data source.
 *   Destroyed by caller.
 * @param inServerPortNumber the port number to listen on for client
 *   connections.
 */ 
void playFromDataSource( DataSource *inSource, int inServerPortNumber );

