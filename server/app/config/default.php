<?php
declare(strict_types=1);

use App\Console\AwardPointsCommand;
use App\Console\CloneRoundCommand;
use App\Console\GetStatusCommand;
use App\Console\ListGamesCommand;
use App\Console\ListRoundsCommand;
use App\Console\PauseGameCommand;
use App\Console\ResumeGameCommand;
use App\Console\SetupGameCommand;
use App\Console\StartGameCommand;
use App\Console\TailEventsCommand;

date_default_timezone_set('Europe/Prague');

return [
    'displayErrorDetails' => true, // Should be set to false in production
    'logError'            => true,
    'logErrorDetails'     => true,
    'app-logger' => [
        'name' => 'app',
        'path' => isset($_ENV['docker']) ? 'php://stdout' : __DIR__ . '/../../logs/app.log',
        'level' => \Monolog\Level::Debug,
    ],
    'gateway-client-logger' => [
        'name' => 'gateway.client',
        'path' => isset($_ENV['docker']) ? 'php://stdout' : __DIR__ . '/../../logs/gateway.client.log',
        'level' => \Monolog\Level::Debug,
    ],
    'db' => [], // array of parameters for Ivory::setupNewConnection(); must be defined by env.php
    'gateway' => [ // parameters for gateway client
        'mock' => false,    // setting to TRUE mocks the client rather than reaching a real one
        // URI of the gateway root. E.g., appending "/v1/status" to this should result in the complete status endpoint URL.
        'base_uri' => null, // must be defined by env.php
    ],
    'server' => [ // parameters for the server
        'auth' => true,     // setting to TRUE requires clients to authenticate for select endpoints
        // URI of the server root. E.g., appending "/v1/status" to this should result in the complete status endpoint URL.
        // Sent to the gateway for logging router events. If null or unspecified, gateway will automatically infer from source
        // IP address starting the game.
        'base_uri' => null, // optionally define in env.php
    ],
    'commands' => [
        GetStatusCommand::class,
        ListGamesCommand::class,
        ListRoundsCommand::class,
        CloneRoundCommand::class,
        PauseGameCommand::class,
        ResumeGameCommand::class,
        SetupGameCommand::class,
        StartGameCommand::class,
        AwardPointsCommand::class,
        TailEventsCommand::class,
    ],
];
