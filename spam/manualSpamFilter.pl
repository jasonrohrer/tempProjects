#!/usr/bin/perl -w


#### settings

# where mail is kept
my $mailDirectory = "~/mail";

# skip any files that contain one of these strings in their name
my $skipFileNamesThatContainA = ".sylpheed";
my $skipFileNamesThatContainB = "spam";

#spam moved here
my $spamFolder = "$mailDirectory/spam";

# file used to track the timestamp of the last filter run
# used to detect which messages are new
my $lastCheckFile = "~/.lastSpamFilterRun";

# command to pipe messages into
# should print score/threshold  (example:  4.3/5.0) as a result
my $spamCheckerCommand = "spamc -c";

# alternal command for stand-alone spamassassin
# this is actually not supported by the current code below
#my $spamCheckerCommand = "spamassassin"


#### end of settings



# all regular files that are newer than our marker file
my $newMailFiles = `find $mailDirectory -type f -newer $lastCheckFile`;

#print "New mail files:\n" . $newMailFiles;


my @newMailList = split( /\n/, $newMailFiles );

foreach my $fileName ( @newMailList ) {
    if( ! ($fileName =~ m/$skipFileNamesThatContainA/) and
        ! ($fileName =~ m/$skipFileNamesThatContainB/) ) {
        
        print "Checking file $fileName\n";
        my $result = `$spamCheckerCommand < "$fileName"`;

        (my $score, my $threshold) = split( /\//, $result );
        
        chomp $score;
        chomp $threshold;

        print "   score: $score/$threshold   ";

        if( $score >= $threshold ) {
            print "spam\n";

            `mv "$fileName" $spamFolder`;
        }
        else {
            print "not spam\n";
        }
    }
}



`touch $lastCheckFile`;
