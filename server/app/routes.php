<?php
declare(strict_types=1);

use App\Application\Actions\NotImplementedAction;
use App\Application\Actions\V1\Game\AwardAdHocPointsAction;
use App\Application\Actions\V1\Game\GetRoundEventInstructionsAction;
use App\Application\Actions\V1\Game\GetRoundPacketInstructionsAction;
use App\Application\Actions\V1\Game\GetRouterCheckInStatusAction;
use App\Application\Actions\V1\Game\LogRouterEventsAction;
use App\Application\Actions\V1\Game\ValidateRoundSpecAction;
use App\Application\Actions\V1\Score\GetTeamScoreboardAction;
use App\Application\Actions\V1\Status\StatusAction;
use App\Application\Middleware\GameEndpointAuthenticator;
use DI\Container;
use Psr\Http\Message\ResponseInterface as Response;
use Psr\Http\Message\ServerRequestInterface as Request;
use Slim\App;
use Slim\Interfaces\RouteCollectorProxyInterface as Group;
use Tuupola\Middleware\HttpBasicAuthentication;

return function (App $app, Container $container) {
    $app->options('/{routes:.*}', function (Request $request, Response $response) {
        // CORS Pre-Flight OPTIONS Request Handler
        return $response;
    });

    $app->get('/', function (Request $request, Response $response) {
        $response->getBody()->write('See https://github.com/InstruktoriBrno/routing-system/blob/master/api.md');
        return $response;
    });

    // NOTE: keep the parameter names in sync with App\Application\Actions\V1\RouteParam

    $app->group('/v1/status', function (Group $group) {
        $group->get('', StatusAction::class);
        $group->get('/round/{roundId}/checkin', GetRouterCheckInStatusAction::class);
    });

    $gameGroup = $app->group('/v1/game', function (Group $group) {
        $group->post('/round/{roundId}/router/{routerId}', LogRouterEventsAction::class);
        $group->post('/round/{roundId}/team/{teamId}', AwardAdHocPointsAction::class);
        $group->get('/round/{roundId}/validate-setup', ValidateRoundSpecAction::class);
        $group->get('/round/{roundId}/instructions/packets', GetRoundPacketInstructionsAction::class);
        $group->get('/round/{roundId}/instructions/packets/{routerIds}', GetRoundPacketInstructionsAction::class);
        $group->get('/round/{roundId}/instructions/events', GetRoundEventInstructionsAction::class);
    });

    $authenticator = $container->get(GameEndpointAuthenticator::class);
    $authMiddleware = new HttpBasicAuthentication([
        'secure' => false, // use HTTP Basic Auth over HTTP (i.e., not HTTPS)
        'authenticator' => $authenticator,
    ]);
    $gameGroup->addMiddleware($authMiddleware->addRule($authenticator));
    $gameGroup->addMiddleware($authenticator);

    $app->group('/v1/score', function (Group $group) {
        $group->get('/round/{roundId}/team/{teamId}', GetTeamScoreboardAction::class);
        $group->get('/round/{roundId}/overall', NotImplementedAction::class);
    });
};
