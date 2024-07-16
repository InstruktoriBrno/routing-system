<?php
declare(strict_types=1);

namespace App\Console;

use Symfony\Component\Console\Input\InputArgument;
use Symfony\Component\Console\Input\InputInterface;
use Symfony\Component\Console\Output\OutputInterface;

final class SetupGameCommand extends CommandBase
{
    private const ARG_ROUND_IDENT = 'roundId';
    private const OPT_START = 'start';
    private const SLEEP_BEFORE_START = 3;

    protected function define(): void
    {
        $this->setName('setup-game');
        $this->setDescription(
            <<<'TEXT'
            Instructs the gateway to set up the game
            TEXT
        );

        $this->addArgument(
            self::ARG_ROUND_IDENT,
            InputArgument::REQUIRED,
            <<<'TEXT'
            Game round API identifier. This will usually be the game_round.id.
            TEXT
        );

        $this->addOption(
            self::OPT_START,
            null,
            null,
            'After setup succeeds, wait 3 seconds and start the game.'
        );
    }

    protected function executeImpl(InputInterface $input, OutputInterface $output): void
    {
        $roundIdent = self::getRoundIdentArgument($input, self::ARG_ROUND_IDENT);

        $round = $this->getDb()->querySingleTuple(
            'SELECT * FROM game_round WHERE api_ident = %i',
            $roundIdent
        );
        if ($round === null) {
            throw new CommandRuntimeException(sprintf('No game round exists with API identifier "%s"', $roundIdent));
        }

        $spec = $round->spec->getValue();
        $spec->roundId = $round->api_ident;
        $spec->roundName = $round->name;

        $res = $this->getGatewayClient($output)->put('/v1/game/round', ['json' => $spec]);
        $this->processHttpClientResult($res);

        if (!$input->hasOption(self::OPT_START)) {
            return;
        }

        $output->writeln('');
        $output->writeln(sprintf('Will start in %d seconds...', self::SLEEP_BEFORE_START));
        sleep(self::SLEEP_BEFORE_START);
        $output->writeln('');

        $res = $this->getGatewayClient($output)->post('/v1/game/start', [
            'json' => [
                'roundId' => $round->api_ident,
                'password' => $round->api_password
            ],
        ]);
        $this->processHttpClientResult($res);
    }
}
