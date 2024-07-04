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
            001: A, min 0, locator
            002: B, min 0, locator
            003: C, min 0, locator
            011: A, min 0 sec 30, return
            010: A, min 1, return
            030: B, min 1, return
            012: A, min 1 sec 30, return
            020: B, min 3, return

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
            001: A, min 0, locator
            002: B, min 0, locator
            003: C, min 0, locator
            004: D, min 0, locator
            005: E, min 0, locator
            006: F, min 0, locator
            007: G, min 0, locator
            008: H, min 0, locator
            009: I, min 0, locator
            010: J, min 0, locator
            020: A, min 1, return
            021: A, min 2, return
            022: A, min 3, return
            023: A, min 4, return
            024: A, min 5, return
            025: A, min 6, return
            026: A, min 7, return
            027: A, min 8, return
            028: A, min 9, return

            TXT,
            (string)$response->getBody()
        );
    }
}
