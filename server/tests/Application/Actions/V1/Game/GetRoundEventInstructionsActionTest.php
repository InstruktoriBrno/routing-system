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
            1:00 zapnout linku AF
            2:00 vypnout linku AF
            3:00 zapnout linku AF
            4:00 vypnout linku AF
            5:00 zapnout linku AF
            6:00 vypnout linku AF
            7:00 zapnout linku AF
            8:00 vypnout linku AF
            9:00 zapnout linku AF
            10:00 vypnout linku AF
            11:00 zapnout linku AC
            11:00 zapnout linku AD
            11:00 zapnout linku AE
            11:00 zapnout linku AF
            11:00 zapnout linku AG
            11:00 zapnout linku AH
            11:00 zapnout linku AI
            11:00 zapnout linku BD
            11:00 zapnout linku BE
            11:00 zapnout linku BF
            11:00 zapnout linku BG
            11:00 zapnout linku BH
            11:00 zapnout linku BI
            11:00 zapnout linku CE
            11:00 zapnout linku CF
            11:00 zapnout linku CG
            11:00 zapnout linku CH
            11:00 zapnout linku CI
            11:00 zapnout linku DF
            11:00 zapnout linku DG
            11:00 zapnout linku DH
            11:00 zapnout linku DI
            11:00 zapnout linku EG
            11:00 zapnout linku EH
            11:00 zapnout linku EI
            11:00 zapnout linku FH
            11:00 zapnout linku FI
            11:00 zapnout linku GI

            TXT,
            (string)$response->getBody()
        );
    }
}
