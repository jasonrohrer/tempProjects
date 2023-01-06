

use LWP::UserAgent::iTMS_Client;


# search the Store
    
#my $ua = LWP::UserAgent::iTMS_Client->new;
#my $listings = $ua->search( song => 'apples' );
#foreach my $album (@{$listings}) { print $album->{songName} }


my $ua = new LWP::UserAgent::iTMS_Client(
        account_type => 'apple',
        user_id => 'jcr13@cornell.edu',
        password => 'monolith',
        DEBUG => 5,
    );
$ua->login;
