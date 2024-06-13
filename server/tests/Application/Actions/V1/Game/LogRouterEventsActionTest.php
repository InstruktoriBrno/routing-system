<?php
declare(strict_types=1);

namespace Tests\Application\Actions\V1\Game;

use App\Application\Handlers\HttpErrorHandler;
use App\Domain\DomainException\DomainRecordNotFoundException;
use App\Domain\Game\GameRound;
use App\Domain\Game\GameRoundRepository;
use DI\Container;
use Prophecy\Argument;
use Prophecy\Prophecy\ObjectProphecy;
use Slim\App;
use Slim\Middleware\ErrorMiddleware;
use Tests\TestCase;

class LogRouterEventsActionTest extends TestCase
{
    private App $app;

    protected function setUp(): void
    {
        parent::setUp();

        $this->app = $this->createAppInstance();

        $callableResolver = $this->app->getCallableResolver();
        $responseFactory = $this->app->getResponseFactory();

        $errorHandler = new HttpErrorHandler($callableResolver, $responseFactory);
        $errorMiddleware = new ErrorMiddleware($callableResolver, $responseFactory, true, false, false);
        $errorMiddleware->setDefaultErrorHandler($errorHandler);

        $this->app->add($errorMiddleware);
    }

    private function mockGameRoundRepository(ObjectProphecy $gameRoundRepositoryProphecy): void
    {
        $container = $this->app->getContainer();
        assert($container instanceof Container);
        $container->set(GameRoundRepository::class, $gameRoundRepositoryProphecy->reveal());
    }

    public function testAction(): void
    {
        $gameRoundRepositoryProphecy = $this->prophesize(GameRoundRepository::class);
        $gameRoundRepositoryProphecy
            ->findByApiIdent(1)
            ->willReturn(new GameRound(1, 1, 'Test game', '{}', 1, 'pwd'))
            ->shouldBeCalledOnce();
        $gameRoundRepositoryProphecy
            ->logRouterEvents(1, 'C', 'fe:d3:4c:aa:72:11', 'online', Argument::any())
            ->shouldBeCalledOnce();
        $this->mockGameRoundRepository($gameRoundRepositoryProphecy);

        $request = $this->createRequest('POST', '/v1/game/round/1/router/C')
            ->withParsedBody((object)[
                'routerMac' => 'fe:d3:4c:aa:72:11',
                'source' => 'online',
                'events' => [
                    (object)['time' => 15, 'card' => 'A015', 'bearer' => 'fe:d3:4c:aa:72:11:23'],
                    (object)['time' => 25, 'card' => 'Z999', 'bearer' => 'fe:de:33:ab:cd:ef:00', 'score' => 10],
                ],
            ]);
        $response = $this->app->handle($request);

        $this->assertEquals(200, $response->getStatusCode());
    }

    public function testEmptyEvents(): void
    {
        $gameRoundRepositoryProphecy = $this->prophesize(GameRoundRepository::class);
        $gameRoundRepositoryProphecy
            ->findByApiIdent(1)
            ->willReturn(new GameRound(1, 1, 'Test game', '{}', 1, 'pwd'))
            ->shouldBeCalledOnce();
        $gameRoundRepositoryProphecy
            ->logRouterEvents(1, 'C', 'fe:d3:4c:aa:72:11', 'online', Argument::any())
            ->shouldBeCalledOnce();
        $this->mockGameRoundRepository($gameRoundRepositoryProphecy);

        $request = $this->createRequest('POST', '/v1/game/round/1/router/C')
            ->withParsedBody((object)[
                'routerMac' => 'fe:d3:4c:aa:72:11',
                'source' => 'online',
                'events' => [],
            ]);
        $response = $this->app->handle($request);

        $this->assertEquals(200, $response->getStatusCode(), 'Response body: ' . $response->getBody()->getContents());
    }

    public function testNonExistentRound(): void
    {
        $gameRoundRepositoryProphecy = $this->prophesize(GameRoundRepository::class);
        $gameRoundRepositoryProphecy
            ->findByApiIdent(2)
            ->willThrow(new DomainRecordNotFoundException('No game round `2` found'))
            ->shouldBeCalledOnce();
        $this->mockGameRoundRepository($gameRoundRepositoryProphecy);

        $request = $this->createRequest('POST', '/v1/game/round/2/router/C')
            ->withParsedBody((object)[
                'routerMac' => 'fe:d3:4c:aa:72:11',
                'source' => 'online',
                'events' => [
                    (object)['time' => 15, 'card' => 'A015', 'bearer' => 'fe:d3:4c:aa:72:11:23'],
                ],
            ]);
        $response = $this->app->handle($request);

        $this->assertEquals(404, $response->getStatusCode());
    }

    public function testInvalidEvent(): void
    {
        $gameRoundRepositoryProphecy = $this->prophesize(GameRoundRepository::class);
        $this->mockGameRoundRepository($gameRoundRepositoryProphecy);

        $request = $this->createRequest('POST', '/v1/game/round/1/router/C')
            ->withParsedBody((object)[
                'routerMac' => 'fe:d3:4c:aa:72:11',
                'source' => 'online',
                'events' => [
                    (object)['time' => 15, 'card' => 'A015', 'bearer' => 'fe:d3:4c:aa:72:11:23'],
                    (object)['time' => 'foo', 'card' => 'Z999', 'bearer' => 'fe:de:33:ab:cd:ef:00', 'score' => 10],
                ],
            ]);
        $response = $this->app->handle($request);

        $this->assertEquals(400, $response->getStatusCode());
    }
}
