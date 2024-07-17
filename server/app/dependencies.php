<?php
declare(strict_types=1);

use App\Application\Settings\SettingsInterface;
use DI\ContainerBuilder;
use GuzzleHttp\Handler\MockHandler;
use GuzzleHttp\Psr7\Response;
use Ivory\Connection\IConnection;
use Ivory\Ivory;
use Monolog\Handler\StreamHandler;
use Monolog\Logger;
use Psr\Log\LoggerInterface;

return function (ContainerBuilder $containerBuilder) {
    $containerBuilder->addDefinitions([
        LoggerInterface::class => function (SettingsInterface $settings): LoggerInterface {
            $cfg = $settings->get('app-logger');
            $logger = new Logger($cfg['name']);
            $logger->pushHandler(new StreamHandler($cfg['path'], $cfg['level']));
            return $logger;
        },

        IConnection::class => function (SettingsInterface $settings): IConnection {
            return Ivory::setupNewConnection($settings->get('db'));
        },

        \GuzzleHttp\HandlerStack::class => function (SettingsInterface $settings): \GuzzleHttp\HandlerStack {
            if (!empty($settings->get('gateway')['mock'])) {
                $handler = new MockHandler([
                    new Response(200, ['Content-Type' => 'application/json'], '{"res":"<mocked response>"}'),
                    new Response(200, ['Content-Type' => 'application/json'], '{"res":"<mocked response 2>"}'),
                    new Response(200, ['Content-Type' => 'application/json'], '{"res":"<mocked response 3>"}'),
                    new Response(200, ['Content-Type' => 'application/json'], '{"res":"<mocked response 4>"}'),
                    new Response(200, ['Content-Type' => 'application/json'], '{"res":"<mocked response 5>"}'),
                ]);
            } else {
                $handler = null; // let Guzzle select the handler automatically
            }
    
            return \GuzzleHttp\HandlerStack::create($handler);
        },

        // TODO: refactor: encapsulate the gateway client in a class
        \GuzzleHttp\Client::class => function (SettingsInterface $settings, \GuzzleHttp\HandlerStack $handlerStack): \GuzzleHttp\Client {
            return new \GuzzleHttp\Client([
                'base_uri' => $settings->get('gateway')['base_uri'],
                'handler' => $handlerStack,
            ]);
        },
    ]);
};
