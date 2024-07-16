<?php
declare(strict_types=1);
namespace Tests;

use App\Application\Handlers\HttpErrorHandler;
use App\Application\Settings\SettingsInterface;
use App\Domain\Game\GameRound;
use App\Domain\Game\GameRoundRepository;
use DI\Container;
use DI\ContainerBuilder;
use Exception;
use PHPUnit\Framework\TestCase as PHPUnit_TestCase;
use Prophecy\PhpUnit\ProphecyTrait;
use Prophecy\Prophecy\ObjectProphecy;
use Psr\Http\Message\ServerRequestInterface as Request;
use Slim\App;
use Slim\Factory\AppFactory;
use Slim\Middleware\ErrorMiddleware;
use Slim\Psr7\Factory\StreamFactory;
use Slim\Psr7\Headers;
use Slim\Psr7\Request as SlimRequest;
use Slim\Psr7\Uri;
use stdClass;

class TestCase extends PHPUnit_TestCase
{
    use ProphecyTrait;

    protected App $app;

    protected function setUp(): void
    {
        parent::setUp();

        $this->app = $this->createAppInstance();
    }

    /**
     * @return App
     * @throws Exception
     */
    protected function createAppInstance(): App
    {
        // Instantiate PHP-DI ContainerBuilder
        $containerBuilder = new ContainerBuilder();

        // Container intentionally not compiled for tests.

        // Set up settings
        $settings = require __DIR__ . '/../app/settings.php';
        $settings($containerBuilder);

        // Set up dependencies
        $dependencies = require __DIR__ . '/../app/dependencies.php';
        $dependencies($containerBuilder);

        // Build PHP-DI Container instance
        $container = $containerBuilder->build();

        // Instantiate the app
        AppFactory::setContainer($container);
        $app = AppFactory::create();

        // Register middleware
        $middleware = require __DIR__ . '/../app/middleware.php';
        $middleware($app, $container->get(SettingsInterface::class));

        // Register routes
        $routes = require __DIR__ . '/../app/routes.php';
        $routes($app);

        $callableResolver = $app->getCallableResolver();
        $responseFactory = $app->getResponseFactory();

        $errorHandler = new HttpErrorHandler($callableResolver, $responseFactory);
        $errorMiddleware = new ErrorMiddleware($callableResolver, $responseFactory, true, false, false);
        $errorMiddleware->setDefaultErrorHandler($errorHandler);

        $app->add($errorMiddleware);

        return $app;
    }

    /**
     * @param string $method
     * @param string $path
     * @param array  $headers
     * @param array  $cookies
     * @param array  $serverParams
     * @return Request
     */
    protected function createRequest(
        string $method,
        string $path,
        array $headers = ['HTTP_ACCEPT' => 'application/json'],
        array $cookies = [],
        array $serverParams = []
    ): Request {
        $uri = new Uri('', '', 80, $path);
        $handle = fopen('php://temp', 'w+');
        $stream = (new StreamFactory())->createStreamFromResource($handle);

        $h = new Headers();
        foreach ($headers as $name => $value) {
            $h->addHeader($name, $value);
        }

        return new SlimRequest($method, $uri, $h, $cookies, $serverParams, $stream);
    }

    protected function createRequestWithBody(
        string $method,
        string $path,
        string $body,
        array $headers = ['HTTP_ACCEPT' => 'application/json'],
        array $cookies = [],
        array $serverParams = []
    ): Request {
        $stream = (new StreamFactory())->createStream($body);
        return $this->createRequest($method, $path, $headers, $cookies, $serverParams)
            ->withBody($stream);
    }

    protected function setupGameRoundMock(int $roundId, stdClass $spec): ObjectProphecy
    {
        $gameRoundRepositoryProphecy = $this->prophesize(GameRoundRepository::class);
        $gameRoundRepositoryProphecy
            ->findByApiIdent(1)
            ->willReturn(new GameRound($roundId, 1, 'Test game', $spec, $roundId, 'pwd'))
            ->shouldBeCalledOnce();

        $this->mockGameRoundRepository($gameRoundRepositoryProphecy);

        return $gameRoundRepositoryProphecy;
    }

    protected function mockGameRoundRepository(ObjectProphecy $gameRoundRepositoryProphecy): ObjectProphecy
    {
        $container = $this->app->getContainer();
        assert($container instanceof Container);
        $container->set(GameRoundRepository::class, $gameRoundRepositoryProphecy->reveal());

        return $gameRoundRepositoryProphecy;
    }
}
