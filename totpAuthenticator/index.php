<?php

include( "Base32.php" );
include( "Hotp.php" );
include( "Totp.php" );

include( "otpAuthSecret.php" );

global $otpAuthSecret;


echo "OTP Auth token:<br>";

$token = generateTokenSimple( $otpAuthSecret );

echo "$token";



?>