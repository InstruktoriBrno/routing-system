<?php
declare(strict_types=1);

namespace App\Domain\Score;

use Ivory\Connection\IConnection;

class ScoreRepository
{
    private $db;

    public function __construct(IConnection $db)
    {
        $this->db = $db;
    }

    /**
     * @param int $roundId
     * @return RoundScoreRecord[]
     */
    public function fetchRoundScoreByTeam(int $roundId): array
    {
        $this->db->connect();

        $rel = $this->db->query(<<<'SQL'
            WITH card_types (card_num, card_type) AS (
                SELECT card_num, def->>'type'
                FROM
                    game_round,
                    json_each(spec->'packets') e (card_num, def)
                WHERE
                    game_round.id = %int
            )
            SELECT
                team_ident,
                (gre.router_ident IS NULL) AS is_ad_hoc_score,
                ct.card_type,
                SUM(gre.score) AS score
            FROM
                game_round_event gre
                LEFT JOIN card_types ct ON ct.card_num = SUBSTRING(gre.event->>'card' FROM 2)
            WHERE
                gre.game_round_id = %int AND
                gre.score IS NOT NULL
            GROUP BY
                team_ident,
                ct.card_type,
                (gre.router_ident IS NULL),
                ct.card_type
SQL
            ,
            $roundId,
            $roundId
        );

        $result = [];
        foreach ($rel as $t) {
            $rec =& $result[$t->team_ident];
            if (!isset($rec)) {
                $rec = new RoundScoreRecord($t->team_ident);
            }
            
            $rec->overallScore += $t->score;
            if ($t->is_ad_hoc_score) {
                $rec->adHocScore += $t->score;
            } else {
                $rec->perPacketType[$t->card_type] = $t->score;
            }

            unset($rec);
        }

        return $result;
    }
}
