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
    ],
];
