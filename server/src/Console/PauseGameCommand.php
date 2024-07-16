<?php
declare(strict_types=1);

namespace App\Console;

use Symfony\Component\Console\Input\InputInterface;
use Symfony\Component\Console\Output\OutputInterface;

final class PauseGameCommand extends CommandBase
{
    protected function define(): void
    {
        $this->setName('pause-game');
        $this->setDescription(
            <<<'TEXT'
            Instructs the gateway to pause the game
            TEXT
        );
    }

    protected function executeImpl(InputInterface $input, OutputInterface $output): void
    {
        $res = $this->getGatewayClient($output)->post('/v1/game/pause');
        $this->processHttpClientResult($res);
    }
}
