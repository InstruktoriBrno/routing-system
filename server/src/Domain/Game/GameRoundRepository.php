<?php
declare(strict_types=1);

namespace App\Domain\Game;

use App\Domain\DomainException\DomainRecordNotFoundException;
use Ivory\Connection\IConnection;
use Ivory\Data\Map\IValueMap;

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

    public function awardAdHocPoints(int $roundId, string $teamIdent, string $source, \stdClass $event): void
    {
        $this->db->connect();

        $this->db->command(<<<'SQL'
            INSERT INTO game_round_event (game_round_id, event, source, team_ident, score)
                VALUES (%int, %json, %s, %s, %int)
SQL
            ,
            $roundId,
            $event,
            $source,
            $teamIdent,
            $event->score
        );
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

    public function fetchTeams(int $roundId): array
    {
        $this->db->connect();

        $rel = $this->db->query(<<<'SQL'
            SELECT
                grt.team_ident,
                s.name AS subteam_name
            FROM
                game_round_team grt
                JOIN subteam s ON s.id = grt.subteam_id
            WHERE
                grt.game_round_id = %int
            ORDER BY
                team_ident,
                subteam_name
SQL
            ,
            $roundId
        );

        $result = [];
        foreach ($rel as $t) {
            if (!isset($result[$t->team_ident])) {
                $result[$t->team_ident] = [];
            }
            $result[$t->team_ident][] = $t->subteam_name;
        }

        return $result;
    }

    public function fetchSuccessfulLocatorCheckIns(int $roundId): IValueMap
    {
        $this->db->connect();

        $rel = $this->db->query(<<<'SQL'
            WITH locator_cards (card_num, router_ident) AS (
                SELECT
                    card_num,
                    def->>'source'
                FROM
                    game_round,
                    json_each(spec->'packets') e (card_num, def)
                WHERE
                    id = %int AND
                    def->>'type' = 'locator'
            )
            SELECT
                gre.router_ident,
                gre.team_ident,
                gre.event->>'card' AS card
            FROM
                game_round_event gre
                JOIN locator_cards ON gre.event->>'card' = gre.team_ident || locator_cards.card_num
            WHERE
                gre.game_round_id = %int AND
                gre.router_ident = locator_cards.router_ident
SQL
            ,
            $roundId,
            $roundId
        );

        return $rel->assoc('router_ident', 'team_ident', 'card');
    }
}
