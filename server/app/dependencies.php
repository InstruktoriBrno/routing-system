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
use Tuupola\Middleware\HttpBasicAuthentication;

return function (ContainerBuilder $containerBuilder) {
    $containerBuilder->addDefinitions([
        LoggerInterface::class => function (ContainerInterface $c): LoggerInterface {
            $settings = $c->get(SettingsInterface::class);

            $loggerSettings = $settings->get('logger');
            $logger = new Logger($loggerSettings['name']);

            $processor = new UidProcessor();
            $logger->pushProcessor($processor);

            $handler = new StreamHandler($loggerSettings['path'], $loggerSettings['level']);
            $logger->pushHandler($handler);

            return $logger;
        },

        IConnection::class => function (ContainerInterface $c): IConnection {
            $settings = $c->get(SettingsInterface::class);
            return Ivory::setupNewConnection($settings->get('db'));
        },

        \GuzzleHttp\HandlerStack::class => function (ContainerInterface $c): \GuzzleHttp\HandlerStack {
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
        \GuzzleHttp\Client::class => function (ContainerInterface $c): \GuzzleHttp\Client {
            $cfg = $c->get(SettingsInterface::class)->get('gateway');
            $handlerStack = $c->get(\GuzzleHttp\HandlerStack::class);
            return new \GuzzleHttp\Client([
                'base_uri' => $cfg['base_uri'],
                'handler' => $handlerStack,
            ]);
        },

        HttpBasicAuthentication::class => function (ContainerInterface $c): HttpBasicAuthentication {
            return new HttpBasicAuthentication([
                'path' => [
                    '/v1/game/',
                ],
                'secure' => false, // use HTTP Basic Auth over HTTP (i.e., not HTTPS)
                'authenticator' => function ($arguments) use ($c): bool {
                    $db = $c->get(IConnection::class);
                    assert($db instanceof IConnection);

                    $username = $arguments['user'];
                    $password = $arguments['password'];

                    $roundIdent = filter_var($username, FILTER_VALIDATE_INT);
                    if ($roundIdent === false) {
                        return false;
                    }

                    $apiPassword = $db->querySingleValue(
                        'SELECT api_password FROM game_round WHERE api_ident = %i',
                        $roundIdent
                    );
                    return ($apiPassword === $password);
                    // TODO: only accept for /round/<username>, not any others! It's a shame HttpBasicAutentication authenticator is not given the whole request.
                    //       How: 1. use an authenticator object implementing __invoke($arguments)
                    //            2. stack the first middleware to read $request, and store it in the authenticator object
                    //            3. let this middleware do the job using the authenticator object
                    //       Also, define rules telling whether should authenticate, based on the request path.
                    //       EVEN BETTER, only pass a callable object as a rule, then shouldAuthenticate will call it, first, with the request. As a side effect, it will keep the request, and pass it on to the authenticator.
                },
            ]);
        },
    ]);
};
