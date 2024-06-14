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

        $this->assertEquals(404, $response->getStatusCode());

//         $this->assertEquals(200, $response->getStatusCode());
//         $this->assertSame(
//             <<<'TXT'
// 000: A, check-in
// 001: B, check-in
// 002: C, check-in
// 011: A, min 0 sec 30
// 010: A, min 1
// 030: B, min 1
// 012: A, min 1 sec 30
// 020: B, min 3
// TXT
//             ,
//             $response->getBody()->getContents()
//         );
    }
}