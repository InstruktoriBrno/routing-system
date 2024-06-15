<?php
declare(strict_types=1);
namespace Tests\Application\Actions\V1\Game;

use Tests\Application\e2e\EndToEndTest;

class ScoreE2ETest extends EndToEndTest
{
    public function testAwardAdHocPoints(): void
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

        // TODO: send a router event with score for team B

        $request = $this->createRequest('POST', '/v1/game/round/4/team/A')
            ->withParsedBody((object)[
                'score' => 10,
                'reason' => 'malfunctioning router',
            ]);
        $response = $this->app->handle($request);
        $this->assertEquals(200, $response->getStatusCode());

        $request = $this->createRequest('POST', '/v1/game/round/4/team/B')
            ->withParsedBody((object)[
                'score' => 4,
                'reason' => 'malfunctioning router',
            ]);
        $response = $this->app->handle($request);
        $this->assertEquals(200, $response->getStatusCode());

        $request = $this->createRequest('POST', '/v1/game/round/4/team/A')
            ->withParsedBody((object)[
                'score' => -5,
                'reason' => 'correction',
            ]);
        $response = $this->app->handle($request);
        $this->assertEquals(200, $response->getStatusCode());

        $request = $this->createRequest('GET', '/v1/score/round/4/team/A');
        $response = $this->app->handle($request);
        $this->assertEquals(200, $response->getStatusCode());
        $teamAScore = json_decode((string)$response->getBody());
        $this->assertSame(4, $teamAScore->roundId);
        $this->assertSame('1. round', $teamAScore->roundName);
        $this->assertSame('A', $teamAScore->teamId);
        $this->assertSame(5, $teamAScore->overallScore);
        $this->assertSame(5, $teamAScore->adHocScore);
    }
}
