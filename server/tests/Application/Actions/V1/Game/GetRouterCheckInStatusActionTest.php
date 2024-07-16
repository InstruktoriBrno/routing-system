<?php
declare(strict_types=1);
namespace Tests\Application\Actions\V1\Game;

use Tests\Application\e2e\EndToEndTest;

class GetRouterCheckInStatusActionTest extends EndToEndTest
{
    public function testAction(): void
    {
        $request = $this->createRequest('GET', '/v1/status/round/4/checkin');
        $response = $this->app->handle($request);

        $expectedJson = <<<'JSON'
{
    "roundId": 4,
    "checkedIn": [
        {
            "router": "B",
            "teamId": "A",
            "card": "A002"
        },
        {
            "router": "E",
            "teamId": "A",
            "card": "A005"
        }
    ],
    "missing": [
        {
            "router": "A",
            "teamId": "A"
        },
        {
            "router": "A",
            "teamId": "B"
        },
        {
            "router": "B",
            "teamId": "B"
        },
        {
            "router": "C",
            "teamId": "A"
        },
        {
            "router": "C",
            "teamId": "B"
        },
        {
            "router": "D",
            "teamId": "A"
        },
        {
            "router": "D",
            "teamId": "B"
        },
        {
            "router": "E",
            "teamId": "B"
        },
        {
            "router": "F",
            "teamId": "A"
        },
        {
            "router": "F",
            "teamId": "B"
        },
        {
            "router": "G",
            "teamId": "A"
        },
        {
            "router": "G",
            "teamId": "B"
        },
        {
            "router": "H",
            "teamId": "A"
        },
        {
            "router": "H",
            "teamId": "B"
        },
        {
            "router": "I",
            "teamId": "A"
        },
        {
            "router": "I",
            "teamId": "B"
        },
        {
            "router": "J",
            "teamId": "A"
        },
        {
            "router": "J",
            "teamId": "B"
        }
    ]
}
JSON;        

        $this->assertEquals(200, $response->getStatusCode());
        $this->assertSame(
            str_replace("\r\n", "\n", $expectedJson),
            (string)$response->getBody()
        );
    }
}
