This file contains documentation for the various settings files used by 
Primrose.



-- localSearchTrustLevel.ini
-- Range:  decimal values from 0.0 to 1.0
The minimum trust level for other nodes that are allowed to search the this
node's local file share.



-- portRangeHigh.ini
-- Range:   integer values from 1025 to 65536
The high end of the possible port range for incoming TCP connections.



-- portRangeLow.ini
-- Range:   integer values from 1025 to 65536
The low end of the possible port range for incoming TCP connections.



-- secondsBetweenSeedFetches.ini
-- Range:   integer values
How long to wait before asking the trust server for more seed hosts.



-- tcpPort.ini
-- Range:   integer values from 1025 to 65536
The main TCP port to listen on for connections.  This value must not be in
the range described by portRangeLow.ini and portRangeHigh.ini



-- trustServerURL.ini
-- Range:  string values representing valid URLs
The URL of the trust server.



-- udpPort.ini
-- Range:   integer values from 1025 to 65536
The port to receive UDP messages on.  Must be different from tcpPort.ini and
must not be in the range described by portRangeLow.ini and portRangeHigh.ini



-- uploadTrustLevel.ini
-- Range:  decimal values from 0.0 to 1.0
The minimum trust level for other nodes that are allowed download files from
this node's local share.  In other words., the minimum trust level for nodes 
that we will upload to.

