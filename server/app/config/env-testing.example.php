<?php
declare(strict_types=1);

// An example for env-testing.php located in the same directory as this file.
// The env-testing.php gets included instead of env.php if requested using the constant APP_ENV='testing'.

use Ivory\Connection\ConnectionParameters;

$settings['db'][ConnectionParameters::HOST] = 'localhost';
$settings['db'][ConnectionParameters::USER] = 'user';
$settings['db'][ConnectionParameters::PASSWORD] = 'password';
$settings['db'][ConnectionParameters::DBNAME] = 'db_test';
