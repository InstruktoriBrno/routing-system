<?php
declare(strict_types=1);
namespace Tests\Application\Actions\V1\Game;

use Tests\Application\e2e\EndToEndTest;

class ScoreE2ETest extends EndToEndTest
{
    public function testTeamScore(): void
    {
        $request = $this->createRequest('GET', '/v1/score/round/4/team/A');
        $response = $this->app->handle($request);
        $this->assertEquals(200, $response->getStatusCode());
        $teamAScore = json_decode((string)$response->getBody());
        $this->assertSame(4, $teamAScore->roundId);
        $this->assertSame('1. round', $teamAScore->roundName);
        $this->assertSame('A', $teamAScore->teamId);
        $this->assertSame(0, $teamAScore->overallScore);
        $this->assertSame(0, $teamAScore->adHocScore);

        $request = $this->createRequestWithBody('POST', '/v1/game/round/4/router/B', <<<'JSON'
{
    "routerMac": "ab:cd:ef:ab:cd:ef",
    "source": "online",
    "events": [
        { "time": 15, "card": "A020", "bearer": "fe:d3:4c:aa:72:11:23", "score": 10 },
        { "time": 15, "card": "A020", "bearer": "fe:d3:4c:aa:72:11:23", "score": 10 },
        { "time": 20, "card": "A020", "bearer": "fe:d3:4c:aa:72:11:23", "score": 10 },
        { "time": 15, "card": "B020", "bearer": "fe:d3:4c:aa:72:11:24", "score": 10 },
        { "time": 99, "card": "A022", "bearer": "fe:d3:4c:aa:72:11:25" }
    ]
}
JSON
        );
        $response = $this->app->handle($request);
        $this->assertJsonStringEqualsJsonString('{"insertCnt": 4}', (string)$response->getBody());
        $this->assertEquals(200, $response->getStatusCode());
    
        $request = $this->createRequestWithBody('POST', '/v1/game/round/4/router/D', <<<'JSON'
{
    "routerMac": "ab:cd:ef:ab:cd:e9",
    "source": "online",
    "events": [
        { "time": 40, "card": "A022", "bearer": "fe:d3:4c:aa:72:11:24", "score": 30 }
    ]
}
JSON
        );
        $response = $this->app->handle($request);
        $this->assertJsonStringEqualsJsonString('{"insertCnt": 1}', (string)$response->getBody());
        $this->assertEquals(200, $response->getStatusCode());
            
        $request = $this->createRequestWithBody('POST', '/v1/game/round/4/router/J', <<<'JSON'
{
    "routerMac": "ab:cd:ef:ab:cd:a9",
    "source": "offline_box",
    "events": [
        { "time": 570, "card": "B028", "bearer": "fe:d3:4c:aa:72:11:28", "score": 90 }
    ]
}
JSON
        );
        $response = $this->app->handle($request);
        $this->assertJsonStringEqualsJsonString('{"insertCnt": 1}', (string)$response->getBody());
        $this->assertEquals(200, $response->getStatusCode());
            
        $request = $this->createRequestWithBody('POST', '/v1/game/round/4/team/A', <<<'JSON'
{
    "score": 10,
    "reason": "malfunctioning router"
}
JSON
        );
        $response = $this->app->handle($request);
        $this->assertEquals('[]', (string)$response->getBody());
        $this->assertEquals(200, $response->getStatusCode());

        $request = $this->createRequestWithBody('POST', '/v1/game/round/4/team/B', <<<'JSON'
{
    "score": 4,
    "reason": "malfunctioning router"
}
JSON
        );
        $response = $this->app->handle($request);
        $this->assertEquals('[]', (string)$response->getBody());
        $this->assertEquals(200, $response->getStatusCode());

        $request = $this->createRequestWithBody('POST', '/v1/game/round/4/team/A', <<<'JSON'
{
    "score": -5,
    "reason": "correction"
}
JSON
        );
        $response = $this->app->handle($request);
        $this->assertEquals('[]', (string)$response->getBody());
        $this->assertEquals(200, $response->getStatusCode());

        $request = $this->createRequest('GET', '/v1/score/round/4/team/A');
        $response = $this->app->handle($request);
        $this->assertEquals(200, $response->getStatusCode());
        $teamAScore = json_decode((string)$response->getBody());

        $this->assertSame(4, $teamAScore->roundId);
        $this->assertSame('1. round', $teamAScore->roundName);
        $this->assertSame('A', $teamAScore->teamId);
        $this->assertSame(55, $teamAScore->overallScore);
        $this->assertSame(5, $teamAScore->adHocScore);
        $this->assertSame(['return'], array_keys(get_object_vars($teamAScore->perPacketType)));
        $this->assertSame(50, $teamAScore->perPacketType->return->thisTeamScore);
        $this->assertSame(100, $teamAScore->perPacketType->return->bestTeamScore);
    }
}
