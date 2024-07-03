<?php
declare(strict_types=1);

namespace App\Application\Actions\V1\Game;

use App\Application\Actions\Action;
use App\Domain\Game\GameRoundRepository;
use Psr\Log\LoggerInterface;

abstract class GameAction extends Action
{
    protected GameRoundRepository $gameRoundRepository;

    public function __construct(LoggerInterface $logger, GameRoundRepository $gameRoundRepository)
    {
        parent::__construct($logger);
        $this->gameRoundRepository = $gameRoundRepository;
    }

    protected function formatInstructionTime(int $time): string
    {
        $result = sprintf('min %d', $time / 60);

        if ($time % 60 != 0) {
            $result .= sprintf(' sec %d', $time % 60);
        }
        
        return $result;
    }
}
