<?php
declare(strict_types=1);

namespace App\Console;

use Symfony\Component\Console\Input\InputInterface;
use Symfony\Component\Console\Output\OutputInterface;

final class ResumeGameCommand extends CommandBase
{
    protected function define(): void
    {
        $this->setName('resume-game');
        $this->setDescription(
            <<<'TEXT'
            Instructs the gateway to resume the paused game
            TEXT
        );
    }

    protected function execute(InputInterface $input, OutputInterface $output): int
    {
        $res = $this->getGatewayClient($output)->post('/v1/game/resume');
        return $this->processHttpClientResult($res);
    }
}
