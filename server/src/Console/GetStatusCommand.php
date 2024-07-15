<?php
declare(strict_types=1);

namespace App\Console;

use Symfony\Component\Console\Input\InputInterface;
use Symfony\Component\Console\Output\OutputInterface;

final class GetStatusCommand extends CommandBase
{
    protected function define(): void
    {
        $this->setName('get-status');
        $this->setDescription(
            <<<'TEXT'
            Fetches the current status of the game from the gateway
            TEXT
        );
    }

    protected function execute(InputInterface $input, OutputInterface $output): int
    {
        $res = $this->getGatewayClient($output)->get('/v1/status');
        return $this->processHttpClientResult($res, $output);
    }
}
