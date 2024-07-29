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

    public function testInvalidPacketCardNum(): void
    {
        $spec = new stdClass();
        $spec->duration = 300;
        $spec->routers = new stdClass();
        $spec->routers->{'A'} = new stdClass();
        $spec->routers->{'A'}->mac = ['12:34:56:78:09:ae'];
        $spec->links = [];
        $spec->packets = new stdClass();
        $spec->packets->{'00'} = (object)['type' => 'locator', 'releaseTime' => 0, 'source' => 'A'];
        $spec->events = [];

        $this->setupGameRoundMock(1, $spec);

        $request = $this->createRequest('GET', '/v1/game/round/1/validate-setup');
        $response = $this->app->handle($request);

        $this->assertEquals(200, $response->getStatusCode());
        $this->assertStringContainsStringIgnoringCase('JSON schema', (string)$response->getBody());
        $this->assertStringContainsStringIgnoringCase('packets', (string)$response->getBody());
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

    public function testVariousErrors(): void
    {
        $spec = new stdClass();
        $spec->duration = 300;
        $spec->routers = (object)[
            'A' => (object)['mac' => ['12:34:56:78:09:ab']],
            'B' => (object)['mac' => ['12:34:56:78:09:ac', '12:34:56:78:09:ad']],
            'C' => (object)['mac' => ['12:34:56:78:09:ad']], // duplicate MAC
            'Z' => (object)['mac' => []], // no MAC addresses
        ];
        $spec->links = [
            'AB',
            'AC',
            'AD', // non-existent router "D"
            'ABB', // wrong link id
        ];
        $spec->packets = (object)[
            '000' => (object)['type' => 'locator', 'releaseTime' => 0, 'source' => 'A'], // "000" reserved for "admin" packets
            '001' => (object)['type' => 'tcp', 'releaseTime' => 30, 'source' => 'A', 'destination' => 'B', 'points' => 20],
            '002' => (object)['type' => 'tcp', 'releaseTime' => 30, 'source' => 'A', 'destination' => 'D', 'points' => 20], // non-existent destination "D"
            '003' => (object)['type' => 'tcp', 'releaseTime' => 30, 'source' => 'D', 'destination' => 'C', 'points' => 20], // non-existent source "D"
            '004' => (object)['type' => 'tcp', 'releaseTime' => 300, 'source' => 'A', 'destination' => 'C', 'points' => 20], // releaseTime after round end
            '010' => (object)['type' => 'chat', 'releaseTime' => 60, 'source' => 'B', 'destination' => 'C', 'points' => 10, 'roundTripCount' => 2, 'messages' => ['a', 'b', 'c', 'd']],
            '011' => (object)['type' => 'chat', 'releaseTime' => 60, 'source' => 'B', 'destination' => 'C', 'points' => 10, 'roundTripCount' => 2, 'messages' => ['a', 'b', 'c']], // not enough messages
            '012' => (object)['type' => 'tcp', 'releaseTime' => 30, 'source' => 'A', 'destination' => 'A', 'points' => 5], // source same as destination
        ];
        $spec->events = [
            (object)['type' => 'linkdown', 'time' => 50, 'link' => 'BA'], // OK despite link written reversely
            (object)['type' => 'linkdown', 'time' => 50, 'link' => 'AB'], // already down
            (object)['type' => 'linkdown', 'time' => 50, 'link' => 'BC'], // not-existent link
            (object)['type' => 'linkdown', 'time' => 300, 'link' => 'AC'], // past the round duration
            (object)['type' => 'linkup', 'time' => 30, 'link' => 'AB'], // already up
            (object)['type' => 'linkup', 'time' => 60, 'link' => 'AB'],
        ];

        $this->setupGameRoundMock(1, $spec);

        $request = $this->createRequest('GET', '/v1/game/round/1/validate-setup');
        $response = $this->app->handle($request);

        $this->assertEquals(200, $response->getStatusCode());

        $responseBody = (string)$response->getBody();
        
        $this->assertStringNotContainsStringIgnoringCase('routers["A"]', $responseBody);
        $this->assertStringNotContainsStringIgnoringCase('routers["B"]', $responseBody);
        $this->assertMatchesRegularExpression('/routers\\["C"\\].*mac/i', $responseBody);
        $this->assertMatchesRegularExpression('/routers\\["Z"\\].*mac/i', $responseBody);
        
        $this->assertStringNotContainsStringIgnoringCase('links[0]', $responseBody);
        $this->assertStringNotContainsStringIgnoringCase('links[1]', $responseBody);
        $this->assertMatchesRegularExpression('/links\\[2\\].*router "D"/i', $responseBody);
        $this->assertStringContainsStringIgnoringCase('links[3]', $responseBody);
        
        $this->assertMatchesRegularExpression('/packets\\["000"\\].*admin/i', $responseBody);
        $this->assertStringNotContainsStringIgnoringCase('packets["001"]', $responseBody);
        $this->assertMatchesRegularExpression('/packets\\["002"\\].*destination.*router "D"/i', $responseBody);
        $this->assertMatchesRegularExpression('/packets\\["003"\\].*source.*router "D"/i', $responseBody);
        $this->assertMatchesRegularExpression('/packets\\["004"\\].*releaseTime/i', $responseBody);
        $this->assertStringNotContainsStringIgnoringCase('packets["010"]', $responseBody);
        $this->assertMatchesRegularExpression('/packets\\["011"\\].*messages/i', $responseBody);
        $this->assertMatchesRegularExpression('/packets\\["012"\\].*source.*destination/i', $responseBody);

        $this->assertStringNotContainsStringIgnoringCase('events[0]', $responseBody);
        $this->assertMatchesRegularExpression('/events\\[1\\].*already down/i', $responseBody);
        $this->assertMatchesRegularExpression('/events\\[2\\].*non-existent/i', $responseBody);
        $this->assertMatchesRegularExpression('/events\\[3\\].*duration/i', $responseBody);
        $this->assertMatchesRegularExpression('/events\\[4\\].*already up/i', $responseBody);
        $this->assertStringNotContainsStringIgnoringCase('events[5]', $responseBody);
    }
}
