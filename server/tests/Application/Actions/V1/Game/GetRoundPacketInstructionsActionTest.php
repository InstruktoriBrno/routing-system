<?php
declare(strict_types=1);
namespace Tests\Application\Actions\V1\Game;

use Tests\Application\e2e\EndToEndTest;

class GetRoundPacketInstructionsActionTest extends EndToEndTest
{
    public function testSimple(): void
    {
        $request = $this->createRequest('GET', '/v1/game/round/1/instructions/packets');
        $response = $this->app->handle($request);

        $this->assertEquals(200, $response->getStatusCode());
        $this->assertSame(
            <<<'TXT'
            001: A, 0:00, locator
            002: B, 0:00, locator
            003: C, 0:00, locator
            011: A, 0:30, return
            010: A, 1:00, return
            030: B, 1:00, return
            012: A, 1:30, return
            020: B, 3:00, return

            TXT,
            (string)$response->getBody()
        );
    }
    
    public function testComplex(): void
    {
        $request = $this->createRequest('GET', '/v1/game/round/4/instructions/packets');
        $response = $this->app->handle($request);

        $this->assertEquals(200, $response->getStatusCode());
        $this->assertSame(
            <<<'TXT'
            001: A, 0:00, locator
            002: B, 0:00, locator
            003: C, 0:00, locator
            004: D, 0:00, locator
            005: E, 0:00, locator
            006: F, 0:00, locator
            007: G, 0:00, locator
            008: H, 0:00, locator
            009: I, 0:00, locator
            010: J, 0:00, locator
            020: A, 1:00, return
            021: A, 2:00, return
            022: A, 3:00, return
            023: A, 4:00, return
            024: A, 5:00, return
            025: A, 6:00, return
            026: A, 7:00, return
            027: A, 8:00, return
            028: A, 9:00, return

            TXT,
            (string)$response->getBody()
        );
    }
}
