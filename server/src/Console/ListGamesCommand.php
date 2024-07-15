<?php
declare(strict_types=1);

namespace App\Console;

use Symfony\Component\Console\Input\InputInterface;
use Symfony\Component\Console\Output\OutputInterface;

final class ListGamesCommand extends CommandBase
{
    protected function define(): void
    {
        $this->setName('list-games');
        $this->setDescription(
            <<<'TEXT'
            Lists all games defined in the database
            TEXT
        );
    }

    protected function execute(InputInterface $input, OutputInterface $output): int
    {
        $rel = $this->getDb()->query('SELECT * FROM game ORDER BY game_date DESC, name, id');
        foreach ($rel as $t) {
            $output->writeln(sprintf(
                "%d\t%s\t%s, gateway %s",
                $t->id,
                $t->game_date->format('Y-m-d'),
                $t->name,
                $t->gateway_address
            ));
        }
        return 0;
    }
}
