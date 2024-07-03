<?php
declare(strict_types=1);
namespace Tests\Application\Actions\V1\Game;

use Tests\Application\e2e\EndToEndTest;

class GetRoundEventInstructionsActionTest extends EndToEndTest
{
    public function testNoEvents(): void
    {
        $request = $this->createRequest('GET', '/v1/game/round/1/instructions/events');
        $response = $this->app->handle($request);

        $this->assertEquals(200, $response->getStatusCode());
        $this->assertSame(
            <<<'TXT'
            zadne herni udalosti v tomto kole

            TXT,
            (string)$response->getBody()
        );
    }

    public function testAllEvents(): void
    {
        $request = $this->createRequest('GET', '/v1/game/round/4/instructions/events');
        $response = $this->app->handle($request);

        $this->assertEquals(200, $response->getStatusCode());
        $this->assertSame(
            <<<'TXT'
            min 1: zapnout linku AF
            min 2: vypnout linku AF
            min 3: zapnout linku AF
            min 4: vypnout linku AF
            min 5: zapnout linku AF
            min 6: vypnout linku AF
            min 7: zapnout linku AF
            min 8: vypnout linku AF
            min 9: zapnout linku AF
            min 10: vypnout linku AF
            min 11: zapnout linku AC
            min 11: zapnout linku AD
            min 11: zapnout linku AE
            min 11: zapnout linku AF
            min 11: zapnout linku AG
            min 11: zapnout linku AH
            min 11: zapnout linku AI
            min 11: zapnout linku BD
            min 11: zapnout linku BE
            min 11: zapnout linku BF
            min 11: zapnout linku BG
            min 11: zapnout linku BH
            min 11: zapnout linku BI
            min 11: zapnout linku CE
            min 11: zapnout linku CF
            min 11: zapnout linku CG
            min 11: zapnout linku CH
            min 11: zapnout linku CI
            min 11: zapnout linku DF
            min 11: zapnout linku DG
            min 11: zapnout linku DH
            min 11: zapnout linku DI
            min 11: zapnout linku EG
            min 11: zapnout linku EH
            min 11: zapnout linku EI
            min 11: zapnout linku FH
            min 11: zapnout linku FI
            min 11: zapnout linku GI

            TXT,
            (string)$response->getBody()
        );
    }
}
