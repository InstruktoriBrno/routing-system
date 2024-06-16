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
001: A, min 0, locator
002: B, min 0, locator
003: C, min 0, locator
011: A, min 0 sec 30, standard
010: A, min 1, standard
030: B, min 1, standard
012: A, min 1 sec 30, standard
020: B, min 3, standard

TXT
            ,
            (string)$response->getBody()
        );


        $request = $this->createRequest('GET', '/v1/game/round/4/instructions');
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
020: A, min 1, standard
021: A, min 2, standard
022: A, min 3, standard
023: A, min 4, standard
024: A, min 5, standard
025: A, min 6, standard
026: A, min 7, standard
027: A, min 8, standard
028: A, min 9, standard

TXT
            ,
            (string)$response->getBody()
        );
    }
}
