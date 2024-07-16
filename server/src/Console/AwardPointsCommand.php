<?php
declare(strict_types=1);

namespace App\Console;

use App\Domain\Game\EventSource;
use App\Domain\Game\GameRoundRepository;
use Symfony\Component\Console\Input\InputArgument;
use Symfony\Component\Console\Input\InputInterface;
use Symfony\Component\Console\Output\OutputInterface;

final class AwardPointsCommand extends CommandBase
{
    private const ARG_ROUND_IDENT = 'roundId';
    private const ARG_TEAM_IDENT = 'teamId';
    private const ARG_SCORE = 'score';
    private const ARG_REASON = 'reason';

    protected function define(): void
    {
        $this->setName('award-points');
        $this->setDescription(
            <<<'TEXT'
            Awards ad hoc points to a team for a game round.
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
            self::ARG_TEAM_IDENT,
            InputArgument::REQUIRED,
            <<<'TEXT'
            Team to awards points to. E.g., "A".
            TEXT
        );
        $this->addArgument(
            self::ARG_SCORE,
            InputArgument::REQUIRED,
            <<<'TEXT'
            Number of points to award. Can even be negative for penalty or as a correction of previous awards.
            TEXT
        );
        $this->addArgument(
            self::ARG_REASON,
            InputArgument::OPTIONAL,
            <<<'TEXT'
            Why the points were awarded.
            TEXT
        );
    }

    protected function execute(InputInterface $input, OutputInterface $output): int
    {
        $roundIdent = (int)$input->getArgument(self::ARG_ROUND_IDENT);
        $teamIdent = $input->getArgument(self::ARG_TEAM_IDENT);
        $score = $input->getArgument(self::ARG_SCORE);
        $reason = $input->getArgument(self::ARG_REASON);

        $event = new \stdClass();
        $event->score = $score;
        if ($reason !== null) {
            $event->reason = $reason;
        }

        $gameRepository = $this->container->get(GameRoundRepository::class);
        assert($gameRepository instanceof GameRoundRepository);

        $round = $gameRepository->findByApiIdent($roundIdent);
        $gameRepository->awardAdHocPoints(
            $round->getId(),
            $teamIdent,
            EventSource::UI->value,
            $event
        );
        return 0;
    }
}
