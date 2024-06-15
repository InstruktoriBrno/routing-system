<?php
declare(strict_types=1);
namespace App\Domain\Score;

class RoundScoreRecord
{
    public string $teamIdent;
    public int $overallScore = 0;
    public int $adHocScore = 0;
    public array $perPacketType = [];

    public function __construct(string $teamIdent)
    {
        $this->teamIdent = $teamIdent;
    }
}
