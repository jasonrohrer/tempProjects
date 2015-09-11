<?php

include_once( "common.php" );


mu_checkReferrer();
mu_clearPasswordCookie();

header('Location: http://meetup.com/homespun' );

?>