<?php
declare(strict_types=1);

namespace App\Console;

use Ivory\Value\Json;
use Ivory\Value\TimestampTz;
use Symfony\Component\Console\Input\InputArgument;
use Symfony\Component\Console\Input\InputInterface;
use Symfony\Component\Console\Input\InputOption;
use Symfony\Component\Console\Output\OutputInterface;

final class TailEventsCommand extends CommandBase
{
    private const ARG_ROUND_IDENT = 'roundId';
    private const ARG_NUM_EVENTS = 'num';
    private const OPT_WITH_ROUTER_MAC = 'with-router-mac';

    private const ARG_NUM_EVENTS_ALL = 'all';

    protected function define(): void
    {
        $this->setName('tail-events');
        $this->setDescription(
            <<<'TEXT'
            Prints the last N events logged for a game round. "Last" means sorted by physical time as logged on the server.
            TEXT
        );

        $this->addArgument(
            self::ARG_ROUND_IDENT,
            InputArgument::REQUIRED,
            <<<'TEXT'
            Game round API identifier. This will usually be the game_round.id.
            TEXT
        );
        $this->addArgument(
            self::ARG_NUM_EVENTS,
            InputArgument::OPTIONAL,
            <<<'TEXT'
            Number of events to print. 10 for last ten events. "all" for all events from the round start.
            TEXT,
            self::ARG_NUM_EVENTS_ALL
        );

        $this->addOption(
            self::OPT_WITH_ROUTER_MAC,
            'm',
            InputOption::VALUE_NONE,
            <<<'TEXT'
            Printer the router MAC address along with its idenfier.
            TEXT
        );
    }

    protected function executeImpl(InputInterface $input, OutputInterface $output): void
    {
        $round = $this->loadRoundFromIdentArgument($input, self::ARG_ROUND_IDENT);
        $eventNumStr = self::getStringArgument($input, self::ARG_NUM_EVENTS, '~^\\d+|' . preg_quote(self::ARG_NUM_EVENTS_ALL) . '$~i');
        $withRouterMac = $input->getOption(self::OPT_WITH_ROUTER_MAC);

        if (strcasecmp($eventNumStr, self::ARG_NUM_EVENTS_ALL) == 0) {
            $eventNum = null;
        } else {
            $eventNum = (int)$eventNumStr;
        }

        $events = $this->getDb()->query(
            'SELECT *
             FROM (
                 SELECT id, server_time, event, team_ident, router_ident, router_mac_address
                 FROM game_round_event
                 WHERE game_round_id = %int
                 ORDER BY server_time DESC, id DESC
                 LIMIT %int
             ) t
             ORDER BY server_time ASC, id',
            $round->id,
            $eventNum
        );
        foreach ($events as $event) {
            assert($event->server_time instanceof TimestampTz);
            assert($event->event instanceof Json);

            $line = $event->server_time->format('Y-m-d H:i:s');
            $line .= ' team ' . $event->team_ident;
            if ($event->router_ident) {
                $line .= 'router ' . $event->router_event;
                if ($withRouterMac) {
                    $line .= ' (' . $event->router_mac_address . ')';
                }
            }
            $line .= ' ' . preg_replace('~\\r\\n|\\r|\\n~', ' ', $event->event->getEncoded());

            $output->writeln($line);
        }
    }
}
