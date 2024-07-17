<?php
declare(strict_types=1);
namespace App\Application\Actions\V1\Game;

use App\Application\Actions\V1\RouteParam;
use Psr\Http\Message\ResponseInterface as Response;

class LogRouterEventsAction extends GameAction
{
    private const REQUEST_PAYLOAD_SCHEMA = /** @lang JSON */
        <<<'JSON'
{
    "type": "object",
    "properties": {
        "routerMac": {
            "type": "string",
            "pattern": "^([0-9a-f]{2}:){5}[0-9a-f]{2}$"
        },
        "source": {
            "type": "string",
            "enum": ["online", "offline_box", "offline_card", "ui"]
        },
        "events": {
            "type": "array",
            "items": {
                "type": "object",
                "properties": {
                    "time": {
                        "type": "integer",
                        "minimum": 0
                    },
                    "card": {
                        "type": "string",
                        "pattern": "^[A-Z][0-9]{3}$"
                    },
                    "bearer": {
                        "type": "string",
                        "pattern": "^([0-9a-f]{2}:){6}[0-9a-f]{2}$"
                    },
                    "score": {
                        "type": "integer"
                    }
                },
                "required": ["time", "card"]
            }
        }
    },
    "required": ["routerMac", "source", "events"]
}
JSON;

    protected function action(): Response
    {
        $roundIdent = $this->resolveIntArg(RouteParam::ROUND_ID, 1, 32767);
        $routerIdent = $this->resolveStringArg(RouteParam::ROUTER_ID, '~^[A-Z]$~');

        $body = $this->getValidatedBody(self::REQUEST_PAYLOAD_SCHEMA);

        $round = $this->gameRoundRepository->findByApiIdent($roundIdent);
        $cnt = $this->gameRoundRepository->logRouterEvents(
            $round->getId(),
            $routerIdent,
            $body->routerMac,
            $body->source,
            $body->events
        );

        return $this->respondWithJsonData(['insertCnt' => $cnt]);
    }
}
