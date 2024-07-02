<?php
declare(strict_types=1);
namespace Tests\Application\Actions\V1\Game;

use App\Domain\DomainException\DomainRecordNotFoundException;
use App\Domain\Game\GameRoundRepository;
use Prophecy\Argument;
use stdClass;
use Tests\TestCase;

class LogRouterEventsActionTest extends TestCase
{
    public function testAction(): void
    {
        $this->setupGameRoundMock(1, new stdClass())
            ->logRouterEvents(1, 'C', 'fe:d3:4c:aa:72:11', 'online', Argument::any())
            ->shouldBeCalledOnce();

        $request = $this->createRequestWithBody('POST', '/v1/game/round/1/router/C',
            <<<'JSON'
            {
                "routerMac": "fe:d3:4c:aa:72:11",
                "source": "online",
                "events": [
                    { "time": 15, "card": "A015", "bearer": "fe:d3:4c:aa:72:11:23" },
                    { "time": 25, "card": "Z999", "bearer": "fe:de:33:ab:cd:ef:00", "score": 10 }
                ]
            }
            JSON
        );
        $response = $this->app->handle($request);

        $this->assertEquals(200, $response->getStatusCode());
    }

    public function testEmptyEvents(): void
    {
        $this->setupGameRoundMock(1, new stdClass())
            ->logRouterEvents(1, 'C', 'fe:d3:4c:aa:72:11', 'online', Argument::any())
            ->shouldBeCalledOnce();

        $request = $this->createRequestWithBody('POST', '/v1/game/round/1/router/C',
            <<<'JSON'
            {
                "routerMac": "fe:d3:4c:aa:72:11",
                "source": "online",
                "events": []
            }
            JSON
        );
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

        $request = $this->createRequestWithBody('POST', '/v1/game/round/2/router/C', 
            <<<'JSON'
            {
                "routerMac": "fe:d3:4c:aa:72:11",
                "source": "online",
                "events": [
                    { "time": 15, "card": "A015", "bearer": "fe:d3:4c:aa:72:11:23" }
                ]
            }
            JSON
        );
        $response = $this->app->handle($request);

        $this->assertEquals(404, $response->getStatusCode());
    }

    public function testInvalidEvent(): void
    {
        $gameRoundRepositoryProphecy = $this->prophesize(GameRoundRepository::class);
        $this->mockGameRoundRepository($gameRoundRepositoryProphecy);

        $request = $this->createRequestWithBody('POST', '/v1/game/round/1/router/C',
            <<<'JSON'
            {
                "routerMac": "fe:d3:4c:aa:72:11",
                "source": "online",
                "events": [
                    { "time": 15, "card": "A015", "bearer": "fe:d3:4c:aa:72:11:23" },
                    { "time": "foo", "card": "Z999", "bearer": "fe:de:33:ab:cd:ef:00", "score": 10 }
                ]
            }
            JSON
        );
        $response = $this->app->handle($request);

        $this->assertEquals(400, $response->getStatusCode());
    }
}
