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

    public function testSampleDataRoundSpecLib(): void
    {
        // $roundSpecLibData = $this->getDb()
        //     ->query('SELECT id, name FROM round_spec_lib WHERE NOT is_deleted ORDER BY id');
        // foreach ($roundSpecLibData as $tuple) {
        //     $request = $this->createRequest('GET', "/v1/game/round/1/validate-setup");
        //     $response = $this->app->handle($request);

        //     $msgPrefix = "round spec #{$tuple->id}: `{$tuple->name}`";
    
        //     $this->assertEquals(200, $response->getStatusCode(), "$msgPrefix: status code");
        //     $this->assertSame('OK' . PHP_EOL, (string)$response->getBody(), "$msgPrefix: response body");
        // }

        $roundSpecLib = $this->getDb()
            ->query('SELECT name, spec FROM round_spec_lib WHERE NOT is_deleted')
            ->assoc('name', 'spec');
        foreach ($roundSpecLib as $name => $spec) {
            assert($spec instanceof Json);
            $this->app = $this->createAppInstance(); // reset mocks
            $this->setupGameRoundMock(1, $spec->getValue());
            $request = $this->createRequest('GET', '/v1/game/round/1/validate-setup');
            $response = $this->app->handle($request);
    
            $this->assertEquals(200, $response->getStatusCode(), "round spec `$name`: status code");
            $this->assertSame('OK' . PHP_EOL, (string)$response->getBody(), "round spec `$name`: response body");
        }
    }

    public function testInvalidSchema(): void
    {
        $spec = new stdClass();
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

    // TODO: test higher-level validations: validate router keys, links, packet types + properties, etc.
}
