<?php
declare(strict_types=1);

namespace App\Console;

use Symfony\Component\Console\Input\InputArgument;
use Symfony\Component\Console\Input\InputInterface;
use Symfony\Component\Console\Output\OutputInterface;

final class ListRoundsCommand extends CommandBase
{
    private const ARG_GAME_ID = 'gameId';
    private const ARG_VAL_ALL = 'all';

    protected function define(): void
    {
        $this->setName('list-rounds');
        $this->setDescription(
            <<<'TEXT'
            Lists all games rounds defined for a game, or all rounds.

            Use list-games to get a list of games.
            TEXT
        );

        $this->addArgument(
            self::ARG_GAME_ID,
            InputArgument::REQUIRED,
            <<<'TEXT'
            Game ID to list rounds from. "all" to list all game rounds from any game.
            TEXT
        );
    }

    protected function execute(InputInterface $input, OutputInterface $output): int
    {
        $gameIdStr = $input->getArgument(self::ARG_GAME_ID);
        $listAll = (strcasecmp($gameIdStr, self::ARG_VAL_ALL) === 0);

        $rel = $this->getDb()->query(
            'SELECT *
             FROM game_round
             WHERE COALESCE(game_id = %i, TRUE)
             ORDER BY api_ident',
            ($listAll ? null : $gameIdStr)
        );
        foreach ($rel as $t) {
            $output->writeln(sprintf("%d\t%s", $t->api_ident, $t->name));
        }
        return 0;
    }
}
