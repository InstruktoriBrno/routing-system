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

    public function testInvalidDuration(): void
    {
        $spec = new stdClass();
        $spec->duration = 0;
        $spec->routers = new stdClass();
        $spec->links = [];
        $spec->packets = new stdClass();
        $spec->events = [];

        $this->setupGameRoundMock(1, $spec);

        $request = $this->createRequest('GET', '/v1/game/round/1/validate-setup');
        $response = $this->app->handle($request);

        $this->assertEquals(200, $response->getStatusCode());
        $this->assertStringContainsStringIgnoringCase('JSON schema', (string)$response->getBody());
        $this->assertStringContainsStringIgnoringCase('duration', (string)$response->getBody());
    }

    public function testInvalidRouterId(): void
    {
        $spec = new stdClass();
        $spec->duration = 300;
        $spec->routers = new stdClass();
        $spec->routers->{'AB'} = new stdClass();
        $spec->routers->{'AB'}->mac = ['12:34:56:78:09:ae']; // wrong router ID
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

    public function testInvalidLink(): void
    {
        $spec = new stdClass();
        $spec->duration = 300;
        $spec->routers = new stdClass();
        $spec->routers->{'A'} = new stdClass();
        $spec->routers->{'A'}->mac = ['12:34:56:78:09:ae'];
        $spec->links = ['A'];
        $spec->packets = new stdClass();
        $spec->events = [];

        $this->setupGameRoundMock(1, $spec);

        $request = $this->createRequest('GET', '/v1/game/round/1/validate-setup');
        $response = $this->app->handle($request);

        $this->assertEquals(200, $response->getStatusCode());
        $this->assertStringContainsStringIgnoringCase('JSON schema', (string)$response->getBody());
        $this->assertStringContainsStringIgnoringCase('links', (string)$response->getBody());
    }

    public function testPacketPropertiesByType(): void
    {
        $spec = new stdClass();
        $spec->duration = 300;
        $spec->routers = new stdClass();
        $spec->links = [];
        $spec->packets = new stdClass();
        $spec->packets->{'007'} = new stdClass();
        $spec->packets->{'007'}->type = 'priority';
        $spec->packets->{'007'}->releaseTime = 10;
        $spec->packets->{'007'}->source = 'A'; // missing destination, which is required for "priority"
        $spec->packets->{'007'}->pointsPerMinuteLeft = 5;
        $spec->events = [];

        $this->setupGameRoundMock(1, $spec);

        $request = $this->createRequest('GET', '/v1/game/round/1/validate-setup');
        $response = $this->app->handle($request);

        $this->assertEquals(200, $response->getStatusCode());
        $this->assertStringContainsStringIgnoringCase('JSON schema', (string)$response->getBody());
        $this->assertStringContainsStringIgnoringCase('packets', (string)$response->getBody());
    }
}
