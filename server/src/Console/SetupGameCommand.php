<?php
declare(strict_types=1);

namespace App\Console;

use Symfony\Component\Console\Input\InputArgument;
use Symfony\Component\Console\Input\InputInterface;
use Symfony\Component\Console\Output\OutputInterface;

final class SetupGameCommand extends CommandBase
{
    private const ARG_ROUND_IDENT = 'roundId';

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
    }

    protected function execute(InputInterface $input, OutputInterface $output): int
    {
        $roundIdent = $input->getArgument(self::ARG_ROUND_IDENT);

        $round = $this->getDb()->querySingleTuple(
            'SELECT * FROM game_round WHERE api_ident = %int',
            $roundIdent
        );
        $spec = $round->spec->getValue();
        $spec->roundId = $round->api_ident;
        $spec->roundName = $round->name;

        $output->writeln(json_encode($spec), OutputInterface::VERBOSITY_VERBOSE);

        $res = $this->getGatewayClient()->put('/v1/game/round', ['json' => $spec]);
        return $this->processHttpClientResult($res, $output);
    }
}
