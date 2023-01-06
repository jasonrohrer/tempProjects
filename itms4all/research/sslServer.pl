use IO::Socket::SSL;

# https port is 443, the port iTunes connects on for ssl


# construction is not working correctly
my $server = IO::Socket::SSL->new(    Listen    => 5,
                                      LocalAddr => 'localhost',
                                      LocalPort => 443 );

$server->listen();

my $sslSocket = $server->accept();

my $buffer;

while( read( $sslSocket, $buffer, 2560 ) ) {
    print $buffer;
}



