<?php
declare(strict_types=1);
namespace Tests\Application\Actions\V1\Game;

use Ivory\Value\Json;
use stdClass;
use Tests\Application\e2e\EndToEndTest;

class ValidateRoundSpecActionTest extends EndToEndTest
{
    public function testMinimalistic(): void
    {
        $spec = new stdClass();
        $spec->roundId = 1;
        $spec->roundName = 'test';
        $spec->duration = 300;
        $spec->routers = new stdClass();
        $spec->links = [];
        $spec->packets = new stdClass();
        $spec->events = [];

        $this->setupGameRoundMock(1, $spec);

        $request = $this->createRequest('GET', '/v1/game/round/1/validate-setup');
        $response = $this->app->handle($request);

        $this->assertEquals(200, $response->getStatusCode());
        $this->assertSame('OK' . PHP_EOL, (string)$response->getBody());
    }

    public function testInvalidSchema(): void
    {
        $spec = new stdClass();
        $spec->roundId = 1;
        $spec->roundName = 'test';
        $spec->duration = 300;
        $spec->links = [];
        $spec->packets = new stdClass();
        $spec->events = [];

        $this->setupGameRoundMock(1, $spec);

        $request = $this->createRequest('GET', '/v1/game/round/1/validate-setup');
        $response = $this->app->handle($request);

        $this->assertEquals(200, $response->getStatusCode());
        $this->assertStringContainsStringIgnoringCase('JSON schema', (string)$response->getBody());
        $this->assertStringContainsStringIgnoringCase('routers', (string)$response->getBody());
    }

    // TODO: test higher-level validations: validate router keys, links, etc.
}
