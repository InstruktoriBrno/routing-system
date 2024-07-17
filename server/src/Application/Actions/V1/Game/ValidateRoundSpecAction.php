<?php
declare(strict_types=1);
namespace App\Application\Actions\V1\Game;

use App\Application\Actions\V1\RouteParam;
use App\Domain\Game\GameRoundSpecValidator;
use Psr\Http\Message\ResponseInterface as Response;

class ValidateRoundSpecAction extends GameAction
{
    protected function action(): Response
    {
        $roundIdent = $this->resolveIntArg(RouteParam::ROUND_ID, 1, 32767);
        $round = $this->gameRoundRepository->findByApiIdent($roundIdent);

        $validator = new GameRoundSpecValidator();
        $errors = $validator->validate($round->getSpec());

        $response = ($errors ? implode(PHP_EOL, $errors) : 'OK') . PHP_EOL;
        return $this->respondWithPlaintextData($response);
    }
}
