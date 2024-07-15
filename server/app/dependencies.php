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
use Monolog\Processor\UidProcessor;
use Psr\Container\ContainerInterface;
use Psr\Log\LoggerInterface;

return function (ContainerBuilder $containerBuilder) {
    $containerBuilder->addDefinitions([
        LoggerInterface::class => function (ContainerInterface $c) {
            $settings = $c->get(SettingsInterface::class);

            $loggerSettings = $settings->get('logger');
            $logger = new Logger($loggerSettings['name']);

            $processor = new UidProcessor();
            $logger->pushProcessor($processor);

            $handler = new StreamHandler($loggerSettings['path'], $loggerSettings['level']);
            $logger->pushHandler($handler);

            return $logger;
        },

        IConnection::class => function (ContainerInterface $c) {
            // TODO: cache
            $settings = $c->get(SettingsInterface::class);
            return Ivory::setupNewConnection($settings->get('db'));
        },

        \GuzzleHttp\HandlerStack::class => function (ContainerInterface $c) {
            $cfg = $c->get(SettingsInterface::class)->get('gateway');
            if (!empty($cfg['mock'])) {
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
        \GuzzleHttp\Client::class => function (ContainerInterface $c) {
            $cfg = $c->get(SettingsInterface::class)->get('gateway');
            $handlerStack = $c->get(\GuzzleHttp\HandlerStack::class);
            return new \GuzzleHttp\Client([
                'base_uri' => $cfg['base_uri'],
                'handler' => $handlerStack,
            ]);
        },
    ]);
};
