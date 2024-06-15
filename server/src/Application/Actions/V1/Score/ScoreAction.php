<?php
declare(strict_types=1);

namespace App\Application\Actions\V1\Score;

use App\Application\Actions\Action;
use App\Domain\Game\GameRoundRepository;
use App\Domain\Score\ScoreRepository;
use Psr\Log\LoggerInterface;

abstract class ScoreAction extends Action
{
    protected GameRoundRepository $gameRoundRepository;
    protected ScoreRepository $scoreRepository;

    public function __construct(LoggerInterface $logger, GameRoundRepository $gameRoundRepository, ScoreRepository $scoreRepository)
    {
        parent::__construct($logger);
        $this->gameRoundRepository = $gameRoundRepository;
        $this->scoreRepository = $scoreRepository;
    }
}
