<?php
declare(strict_types=1);
namespace App\Application\Actions\V1\Score;

use App\Domain\Game\GameRound;
use App\Domain\Score\RoundScoreRecord;
use Psr\Http\Message\ResponseInterface as Response;

class GetTeamScoreboardAction extends ScoreAction
{
    protected function action(): Response
    {
        $teamIdent = $this->resolveStringArg('teamId', '~^[A-Z]$~');

        $roundIdent = $this->resolveIntArg('roundId', 1, 32767);
        $round = $this->gameRoundRepository->findByApiIdent($roundIdent);

        $scoreByTeam = $this->scoreRepository->fetchRoundScoreByTeam($round->getId());
        $teamScore = ($scoreByTeam[$teamIdent] ?? new RoundScoreRecord($teamIdent));

        $maxScorePerPacketType = [];
        foreach ($scoreByTeam as $scoreRec) {
            foreach ($scoreRec->perPacketType as $packetType => $score) {
                $s =& $maxScorePerPacketType[$packetType];
                if (!isset($s)) {
                    $s = 0;
                }
                $s = max($s, $score);
                unset($s);
            }
        }

        $data = $this->serializeScoreForTeam($round, $teamScore, $maxScorePerPacketType);

        return $this->respondWithJsonData($data);
    }

    private function serializeScoreForTeam(GameRound $round, RoundScoreRecord $teamScore, array $maxScorePerPacketType): array
    {
        $result = [
            'roundId' => $round->getId(),
            'roundName' => $round->getName(),
            'teamId' => $teamScore->teamIdent,
            'overallScore' => $teamScore->overallScore,
            'adHocScore' => $teamScore->adHocScore,
            'perPacketType' => [],
        ];
        foreach ($teamScore->perPacketType as $packetType => $packetTypeScore) {
            $result['perPacketType'][$packetType] = [
                'thisTeamScore' => $packetTypeScore,
                'bestTeamScore' => ($maxScorePerPacketType[$packetType] ?? $packetTypeScore),
            ];
        }

        return $result;
    }
}
