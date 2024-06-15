<?php
declare(strict_types=1);
namespace Tests\Application\Actions\V1\Game;

use Tests\TestCase;

class GetRoundInstructionsActionTest extends TestCase
{
    public function testAction(): void
    {
        $request = $this->createRequest('GET', '/v1/game/round/1/instructions');
        $response = $this->app->handle($request);

        $this->assertEquals(200, $response->getStatusCode());
        $this->assertSame(
            <<<'TXT'
001: A, min 0
002: B, min 0
003: C, min 0
011: A, min 0 sec 30
010: A, min 1
030: B, min 1
012: A, min 1 sec 30
020: B, min 3

TXT
            ,
            (string)$response->getBody()
        );


        $request = $this->createRequest('GET', '/v1/game/round/4/instructions');
        $response = $this->app->handle($request);

        $this->assertEquals(200, $response->getStatusCode());
        $this->assertSame(
            <<<'TXT'
001: A, min 0
002: B, min 0
003: C, min 0
004: D, min 0
005: E, min 0
006: F, min 0
007: G, min 0
008: H, min 0
009: I, min 0
010: J, min 0
020: A, min 1
021: A, min 2
022: A, min 3
023: A, min 4
024: A, min 5
025: A, min 6
026: A, min 7
027: A, min 8
028: A, min 9

TXT
            ,
            (string)$response->getBody()
        );
    }
}
