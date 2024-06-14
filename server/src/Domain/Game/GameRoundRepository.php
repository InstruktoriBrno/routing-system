<?php
declare(strict_types=1);

namespace App\Domain\Game;

use App\Domain\DomainException\DomainRecordNotFoundException;
use Ivory\Connection\IConnection;

class GameRoundRepository
{
    private $db;

    public function __construct(IConnection $db)
    {
        $this->db = $db;
    }

    public function findByApiIdent(int $roundApiIdent): GameRound
    {
        $this->db->connect();

        $t = $this->db->querySingleTuple(
            'SELECT *
             FROM game_round
             WHERE api_ident = %int',
             $roundApiIdent
        );
        if ($t === null) {
            throw new DomainRecordNotFoundException("No game round `$roundApiIdent` found");
        }

        return new GameRound($t->id, $t->game_id, $t->name, $t->spec->getValue(), $t->api_ident, $t->api_password);
    }

    public function logRouterEvents(int $roundId, string $routerIdent, string $routerMac, string $eventSource, array $eventData): void
    {
        $this->db->connect();
        // TODO: insert in a transaction; assume $eventData is a correct list of JSON data
    }

    /**
     * @param int $roundId
     * @return PacketInstruction[]
     */
    public function fetchPacketInstructions(int $roundId): array
    {
        $this->db->connect();

        $rel = $this->db->query(
            <<<'SQL'
        SELECT
            card_num,
            attr->>'source' AS router_ident,
            CAST(attr->>'releaseTime' AS INT) AS release_time
        FROM game_round, json_each(spec->'packets') p (card_num, attr)
        WHERE id = %int
        ORDER BY release_time ASC NULLS FIRST, card_num, router_ident
SQL
            ,
            $roundId
        );

        $result = [];
        foreach ($rel as $t) {
            $pi = new PacketInstruction();
            $pi->cardNum = $t->card_num;
            $pi->routerIdent = $t->router_ident;
            $pi->releaseTime = $t->release_time;
            $result[] = $pi;
        }

        return $result;
    }
}
