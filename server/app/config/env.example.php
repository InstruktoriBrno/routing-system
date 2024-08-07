<?php
declare(strict_types=1);

// An example for env.php located in the same directory as this file.
// The env.php gets included if no APP_ENV is defined (such as for testing).

use Ivory\Connection\ConnectionParameters;

$settings['db'][ConnectionParameters::HOST] = 'localhost';
$settings['db'][ConnectionParameters::USER] = 'user';
$settings['db'][ConnectionParameters::PASSWORD] = 'password';
$settings['db'][ConnectionParameters::DBNAME] = 'db_prod';

$settings['gateway']['base_uri'] = 'http://1.2.3.4:5678';

$settings['server']['base_uri'] = 'http://1.2.3.4:8000';
