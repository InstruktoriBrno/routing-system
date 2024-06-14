<?php
declare(strict_types=1);

use Monolog\Logger;

date_default_timezone_set('Europe/Prague');

return [
    'displayErrorDetails' => true, // Should be set to false in production
    'logError'            => true,
    'logErrorDetails'     => true,
    'logger' => [
        'name' => 'slim-app',
        'path' => isset($_ENV['docker']) ? 'php://stdout' : __DIR__ . '/../logs/app.log',
        'level' => Logger::DEBUG,
    ],
    'db' => [], // array of parameters for Ivory::setupNewConnection(); must be defined by env.php
];
