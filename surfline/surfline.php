<?php


// jeness beach, NH
$spotName = "Jeness Beach";

$spotURL = "https://services.surfline.com/kbyg/spots/forecasts/wave?spotId=60492e85f79634ecb8e7b0fa&days=5&intervalHours=1&cacheEnabled=true&units[swellHeight]=FT&units[waveHeight]=FT";

$humanURL = "https://www.surfline.com/surf-report/jenness-beach/60492e85f79634ecb8e7b0fa";

$reportToEmails = array( "test@test.com" );



$minHeightToReport = 4;


$useSMTP = true;


$smtpHost = "ssl://mail.server.com";

$smtpPort = "465";

$smtpUsername = "jason@server.com";

$smtpPassword = "secret";



// separate SMTP for mission-critical, transactional emails (like download code
// emails)
// Defaults to being same as the above SMTP, which is used for bulk messages
//  (like a note sent to everyone).
$smtpHostTrans = $smtpHost;

$smtpPortTrans = $smtpPort;

$smtpUsernameTrans = $smtpUsername;

$smtpPasswordTrans = $smtpPassword;





$result = file_get_contents( $spotURL );

$a = json_decode( $result, true );


$waveList = $a[ 'data' ][ 'wave' ];

$report = "";
$anyReported = false;


foreach( $waveList as $wave ) {
    $t = $wave[ 'timestamp' ];

    $date = date('D Y/m/d h:i:s a', $t );
    
    $surf = $wave[ 'surf' ];

    $min = $surf[ 'min' ];
    $max = $surf[ 'max' ];

    if( $min >= $minHeightToReport ) {

        $report = $report . "$date:  min=$min, max=$max\n";

        $anyReported = true;
        }
    }

if( $anyReported ) {
    $fullReport = $humanURL ."\n\n".
        "Waves that are at least $minHeightToReport ft high:\n\n".
        $report;


    foreach( $reportToEmails as $e ) {
        
    
        sl_mail( $e,
                 "$spotName Surf Alert:  Waves at least $minHeightToReport ft",
                 $fullReport );
        }
    }
else {
    echo "No waves that are at least $minHeightToReport ft ".
        "high in next 5 days\n";
    }




function sl_mail( $inEmail,
                  $inSubject,
                  $inBody,
                  // true for transactional emails that should use
                  // a different SMTP
                  $inTrans = false ) {
    
    global $useSMTP, $siteEmailAddress, $siteEmailDomain;

    if( $useSMTP ) {
        require_once "Mail.php";

        global $smtpHost, $smtpPort, $smtpUsername, $smtpPassword;

        $messageID = "<" . uniqid() . "@$siteEmailDomain>";
        
        $headers = array( 'From' => $siteEmailAddress,
                          'To' => $inEmail,
                          'Subject' => $inSubject,
                          'Date' => date( "r" ),
                          'Message-Id' => $messageID );
        $smtp;

        if( $inTrans ) {
            global $smtpHostTrans, $smtpPortTrans,
                $smtpUsernameTrans, $smtpPasswordTrans;

            $smtp = Mail::factory( 'smtp',
                                   array ( 'host' => $smtpHostTrans,
                                           'port' => $smtpPortTrans,
                                           'auth' => true,
                                           'username' => $smtpUsernameTrans,
                                           'password' => $smtpPasswordTrans ) );
            }
        else {
            $smtp = Mail::factory( 'smtp',
                                   array ( 'host' => $smtpHost,
                                           'port' => $smtpPort,
                                           'auth' => true,
                                           'username' => $smtpUsername,
                                           'password' => $smtpPassword ) );
            }
        

        $mail = $smtp->send( $inEmail, $headers, $inBody );


        if( PEAR::isError( $mail ) ) {
            echo( "Email send failed ($inEmail):  " .
                    $mail->getMessage() );
            return false;
            }
        else {
            return true;
            }
        }
    else {
        // raw sendmail
        $mailHeaders = "From: $siteEmailAddress";
        
        return mail( $inEmail,
                     $inSubject,
                     $inBody,
                     $mailHeaders );
        }
    }





?>