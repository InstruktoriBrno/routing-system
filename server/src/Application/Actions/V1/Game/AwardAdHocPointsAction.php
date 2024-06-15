<?php
declare(strict_types=1);
namespace App\Application\Actions\V1\Game;

use Psr\Http\Message\ResponseInterface as Response;

class AwardAdHocPointsAction extends GameAction
{
    private const REQUEST_PAYLOAD_SCHEMA = /** @lang JSON */
        <<<'JSON'
{
    "type": "object",
    "properties": {
        "score": {
            "type": "integer"
        },
        "reason": {
            "type": "string"
        }
    },
    "required": ["score"]
}
JSON;

    protected function action(): Response
    {
        $roundIdent = $this->resolveIntArg('roundId', 1, 32767);
        $teamIdent = $this->resolveStringArg('teamId', '~^[A-Z]$~');

        $body = $this->getValidatedBody(self::REQUEST_PAYLOAD_SCHEMA);

        $round = $this->gameRoundRepository->findByApiIdent($roundIdent);
        $this->gameRoundRepository->awardAdHocPoints(
            $round->getId(),
            $teamIdent,
            'ui',
            $body
        );

        return $this->respondWithJsonData();
    }
}
