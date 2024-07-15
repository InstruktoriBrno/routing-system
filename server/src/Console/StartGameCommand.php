<?php
declare(strict_types=1);

namespace App\Console;

use App\Application\Settings\SettingsInterface;
use Symfony\Component\Console\Input\InputArgument;
use Symfony\Component\Console\Input\InputInterface;
use Symfony\Component\Console\Output\OutputInterface;

final class StartGameCommand extends CommandBase
{
    private const ARG_ROUND_IDENT = 'roundId';

    protected function define(): void
    {
        $this->setName('start-game');
        $this->setDescription(
            <<<'TEXT'
            Instructs the gateway to start the game

            This assumes the gateway has already set up the game.
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

        $tuple = $this->getDb()->querySingleTuple(
            'SELECT api_password FROM game_round WHERE api_ident = %int',
            $roundIdent
        );
        if ($tuple === null) {
            $output->writeln(sprintf('No game round exists with API identifier "%s"', $roundIdent));
            return 2;
        }

        $startParams = [
            'roundId' => $roundIdent,
            'password' => $tuple->api_password,
        ];
        
        $cfg = $this->container->get(SettingsInterface::class)->get('server');
        if (!empty($cfg['base_uri'])) {
            $startParams['logRouterEventsEndpointUrl'] = sprintf(
                '%s/v1/game/round/%d/router/',
                $cfg['base_uri'],
                $roundIdent
            );
        }

        $res = $this->getGatewayClient($output)->post('/v1/game/start', [
            'json' => $startParams,
        ]);
        return $this->processHttpClientResult($res, $output);
    }
}
