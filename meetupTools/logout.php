<?php

include_once( "common.php" );


mu_checkReferrer();
mu_clearPasswordCookie();

echo "Logged out";

?>