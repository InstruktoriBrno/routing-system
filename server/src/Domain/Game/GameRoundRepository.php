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

    /**
     * @param int $roundId
     * @return PacketInstruction[]
     */
    public function fetchPacketInstructions(int $roundId): array
    {
        $query = <<<'SQL'
        SELECT
            card_num,
            attr->>'source' AS router_ident,
            CAST(attr->>'releaseTime' AS INT) AS release_time
        FROM game_round, json_each(spec->'packets') p (card_num, attr)
        WHERE id = %d
        ORDER BY release_time ASC NULLS FIRST, card_num, router_ident
SQL;
        return []; // TODO
    }
}
