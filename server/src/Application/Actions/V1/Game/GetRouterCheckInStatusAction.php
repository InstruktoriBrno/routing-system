<?php
declare(strict_types=1);
namespace App\Application\Actions\V1\Game;

use App\Application\Actions\V1\RouteParam;
use Psr\Http\Message\ResponseInterface as Response;

class GetRouterCheckInStatusAction extends GameAction
{
    protected function action(): Response
    {
        $roundIdent = $this->resolveIntArg(RouteParam::ROUND_ID, 1, 32767);
        $round = $this->gameRoundRepository->findByApiIdent($roundIdent);

        $teams = array_keys($this->gameRoundRepository->fetchTeams($round->getId()));
        $routers = $round->listRouters();

        $locatorCheckIns = $this->gameRoundRepository->fetchSuccessfulLocatorCheckIns($round->getId());

        $result = [
            'roundId' => $roundIdent,
            'checkedIn' => [],
            'missing' => [],
        ];
        foreach ($routers as $router) {
            foreach ($teams as $team) {
                $t = ['router' => $router, 'teamId' => $team];
                $card = $locatorCheckIns->maybe($router, $team);
                if ($card === null) {
                    $result['missing'][] = $t;
                } else {
                    $t['card'] = $card;
                    $result['checkedIn'][] = $t;
                }
            }
        }

        return $this->respondWithJsonData($result);
    }
}
