<?php
declare(strict_types=1);

// An example for env.php located in the same directory as this file.
// The env.php gets included if no APP_ENV is defined (such as for testing).

use Ivory\Connection\ConnectionParameters;

$settings['db'][ConnectionParameters::HOST] = 'localhost';
$settings['db'][ConnectionParameters::USER] = 'user';
$settings['db'][ConnectionParameters::PASSWORD] = 'password';
$settings['db'][ConnectionParameters::DBNAME] = 'db_prod';

// URI of the gateway root. E.g., appending "/v1/status" to this should result in the complete status endpoint URL.
$settings['gateway']['base_uri'] = 'http://1.2.3.4:5678';

// URI of the server root. E.g., appending "/v1/status" to this should result in the complete status endpoint URL.
// Sent to the gateway for logging router events. If null or unspecified, gateway will automatically infer from source
// IP address starting the game.
$settings['server']['base_uri'] = 'http://1.2.3.4:8000';
