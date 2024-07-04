<?php
declare(strict_types=1);
namespace App\Application\Actions\V1\Game;

use App\Domain\Game\EventInstruction;
use App\Domain\Game\GameRoundEventType;
use Psr\Http\Message\ResponseInterface as Response;

class GetRoundEventInstructionsAction extends GameAction
{
    protected function action(): Response
    {
        $roundIdent = $this->resolveIntArg('roundId', 1, 32767);
        $round = $this->gameRoundRepository->findByApiIdent($roundIdent);
        
        $eventInstructions = $this->gameRoundRepository->fetchEventInstructions($round->getId());
        $data = $this->serializeEventInstructions($eventInstructions);

        return $this->respondWithPlaintextData($data);
    }

    private function serializeEventInstructions(array $instructions): string
    {
        if (!$instructions) {
            return 'zadne herni udalosti v tomto kole' . PHP_EOL;
        }

        $result = '';
        foreach ($instructions as $inst) {
            assert($inst instanceof EventInstruction);
            $result .= sprintf("%s %s%s",
                $this->formatInstructionTime($inst->time),
                $this->formatEventDescription($inst->type, $inst->parameter),
                PHP_EOL
            );
        }

        return $result;
    }

    private function formatEventDescription(GameRoundEventType $type, string $parameter): string
    {
        return match($type) {
            GameRoundEventType::LinkUp => "zapnout linku $parameter",
            GameRoundEventType::LinkDown => "vypnout linku $parameter",
        };
    }
}
