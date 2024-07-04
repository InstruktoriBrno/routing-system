<?php
declare(strict_types=1);
namespace App\Application\Actions\V1\Game;

use App\Domain\Game\PacketInstruction;
use Psr\Http\Message\ResponseInterface as Response;

class GetRoundPacketInstructionsAction extends GameAction
{
    protected function action(): Response
    {
        if (isset($this->args['routerIds'])) {
            $routerIdList = $this->resolveStringArg('routerIds', '~^[A-Z]*$~');
            $routerIds = ($routerIdList ? preg_split('~~', $routerIdList) : []);
        } else {
            $routerIds = [];
        }

        $roundIdent = $this->resolveIntArg('roundId', 1, 32767);
        $round = $this->gameRoundRepository->findByApiIdent($roundIdent);
        
        $packetInstructions = $this->gameRoundRepository->fetchPacketInstructions($round->getId());
        if ($routerIds) {
            $packetInstructions = $this->filterPacketInstructions($packetInstructions, $routerIds);
        }
        $data = $this->serializePacketInstructions($packetInstructions);

        return $this->respondWithPlaintextData($data);
    }

    private function filterPacketInstructions(array $packetInstructions, array $routerIds): array
    {
        $result = [];
        foreach ($packetInstructions as $inst) {
            assert($inst instanceof PacketInstruction);
            if (in_array($inst->routerIdent, $routerIds)) {
                $result[] = $inst;
            }
        }

        return $result;
    }

    private function serializePacketInstructions(array $instructions): string
    {
        $result = '';
        foreach ($instructions as $inst) {
            assert($inst instanceof PacketInstruction);
            $result .= sprintf("%s %s %s %s%s",
                $this->formatReleaseTime($inst->releaseTime),
                ($inst->routerIdent ?? '-'),
                $inst->cardNum,
                $inst->cardType,
                PHP_EOL
            );
        }

        return $result;
    }

    private function formatReleaseTime(?int $releaseTime): string
    {
        if ($releaseTime === null) {
            return 'check-in';
        }

        return $this->formatInstructionTime($releaseTime);
    }
}
