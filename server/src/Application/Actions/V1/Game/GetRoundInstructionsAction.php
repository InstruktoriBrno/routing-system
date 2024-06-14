<?php
declare(strict_types=1);
namespace App\Application\Actions\V1\Game;

use App\Domain\Game\PacketInstruction;
use Psr\Http\Message\ResponseInterface as Response;

class GetRoundInstructionsAction extends GameAction
{
    protected function action(): Response
    {
        $roundIdent = $this->resolveIntArg('roundId', 1, 32767);

        $round = $this->gameRoundRepository->findByApiIdent($roundIdent);
        
        $packetInstructions = $this->gameRoundRepository->fetchPacketInstructions($round->getId());
        $data = $this->serializePacketInstructions($packetInstructions);

        return $this->respondWithPlaintextData($data);
    }

    private function serializePacketInstructions(array $instructions): string
    {
        $result = '';
        foreach ($instructions as $inst) {
            assert($inst instanceof PacketInstruction);
            $result .= sprintf("%s: %s, %s\n", $inst->cardNum, ($inst->routerIdent ?? '-'), $this->formatReleaseTime($inst->releaseTime));
        }

        return $result;
    }

    private function formatReleaseTime(?int $releaseTime): string
    {
        if ($releaseTime === null) {
            return 'check-in';
        }

        $result = sprintf('min %d', $releaseTime / 60);

        if ($releaseTime % 60 != 0) {
            $result .= sprintf(' sec %d', $releaseTime % 60);
        }
        
        return $result;
    }
}
