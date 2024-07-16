<?php
declare(strict_types=1);

namespace App\Console;

use Symfony\Component\Console\Input\InputArgument;
use Symfony\Component\Console\Input\InputInterface;
use Symfony\Component\Console\Output\OutputInterface;

final class CloneRoundCommand extends CommandBase
{
    private const ARG_ROUND_IDENT = 'roundId';

    protected function define(): void
    {
        $this->setName('clone-round');
        $this->setDescription(
            <<<'TEXT'
            Clones a game round. Useful for debugging when a fresh round ID is needed.
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

    protected function executeImpl(InputInterface $input, OutputInterface $output): void
    {
        $roundIdent = self::getRoundIdentArgument($input->getArgument, self::ARG_ROUND_IDENT);

        $newRoundIdent = $this->getDb()->querySingleValue(
            'INSERT INTO game_round (game_id, name, spec, api_password)
             SELECT game_id, name, spec, api_password
             FROM game_round
             WHERE api_ident = %i
             RETURNING api_ident',
             $roundIdent
        );
        if ($newRoundIdent === null) {
            throw new CommandRuntimeException("No game_round found with api_ident = $roundIdent");
        }

        $output->writeln("Game round cloned, new api_ident = $newRoundIdent");
    }
}
