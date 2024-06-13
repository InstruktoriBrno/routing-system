<?php
declare(strict_types=1);

namespace App\Domain\Game;

use App\Domain\DomainException\DomainRecordNotFoundException;

class GameRoundRepository
{
    public function findByApiIdent(int $roundApiIdent): GameRound
    {
        // TODO: find the database record
        throw new DomainRecordNotFoundException("No game round `$roundApiIdent` found");
    }

    public function logRouterEvents(int $roundId, string $routerIdent, string $routerMac, string $eventSource, array $eventData): void
    {
        // TODO: insert in a transaction; assume $eventData is a correct list of JSON data
    }
}
