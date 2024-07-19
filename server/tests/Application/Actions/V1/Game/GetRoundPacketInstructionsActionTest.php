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
            0:00 A 001 locator
            0:00 B 002 locator
            0:00 C 003 locator
            0:30 A 011 tcp
            1:00 A 010 tcp
            1:00 B 030 tcp
            1:30 A 012 tcp
            3:00 B 020 tcp

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
            0:00 A 001 locator
            0:00 B 002 locator
            0:00 C 003 locator
            0:00 D 004 locator
            0:00 E 005 locator
            0:00 F 006 locator
            0:00 G 007 locator
            0:00 H 008 locator
            0:00 I 009 locator
            0:00 J 010 locator
            1:00 A 020 tcp
            2:00 A 021 tcp
            3:00 A 022 tcp
            4:00 A 023 tcp
            5:00 A 024 tcp
            6:00 A 025 tcp
            7:00 A 026 tcp
            8:00 A 027 tcp
            9:00 A 028 tcp

            TXT,
            (string)$response->getBody()
        );
    }
}
