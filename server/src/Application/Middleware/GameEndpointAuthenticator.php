<?php
declare(strict_types=1);

namespace App\Application\Middleware;

use App\Application\Actions\V1\RouteParam;
use App\Application\Settings\SettingsInterface;
use Ivory\Connection\IConnection;
use Psr\Http\Message\RequestInterface;
use Psr\Http\Message\ResponseInterface;
use Psr\Http\Message\ServerRequestInterface;
use Psr\Http\Server\MiddlewareInterface;
use Psr\Http\Server\RequestHandlerInterface;
use Slim\Routing\RouteContext;
use Slim\Routing\RoutingResults;

/**
 * Authenticator for the HTTP Basic Auth-protected /v1/game/ endpoints.
 * 
 * This is a stateful object. Authentication for /v1/game/ depends on the concrete roundId.
 * Unfortunately, HttpBasicAuthentication does not pass the request to the authenticator, only
 * username and password. It passes the request as the rule deciding whether authentication
 * should even take place, though.
 * 
 * Hence this object. First, its MiddlewareInterface::process() method should be called, which
 * will keep the request and then proceed with the next handler. Then, implementing the
 * authentication itself, it reads the request.
 * 
 * It is the application's responsibility to register this middleware *after* HttpBasicAuthentication,
 * so that it gets called first.
 */
class GameEndpointAuthenticator implements MiddlewareInterface
{
    private IConnection $db;
    private SettingsInterface $settings;
    private ?RequestInterface $currentRequest = null;

    public function __construct(IConnection $db, SettingsInterface $settings)
    {
        $this->db = $db;
        $this->settings = $settings;
    }

    public function process(RequestInterface $request, RequestHandlerInterface $handler): ResponseInterface
    {
        $this->currentRequest = $request;
        return $handler->handle($request);
    }

    public function __invoke($arg): bool
    {
        if ($arg instanceof ServerRequestInterface) {
            return $this->shouldAuthenticate();
        } elseif (is_array($arg)) {
            return $this->authenticate($arg);
        } else {
            throw new \LogicException('Unexpected argument type: ' . gettype($arg));
        }
    }

    private function shouldAuthenticate(): bool
    {
        return !empty($this->settings->get('server')['auth']);
    }

    private function authenticate(array $credentials): bool
    {
        if (!isset($credentials['user'], $credentials['password'])) {
            return false;
        }

        $username = $credentials['user'];
        $password = $credentials['password'];

        $roundIdent = filter_var($username, FILTER_VALIDATE_INT);
        if ($roundIdent === false) {
            return false;
        }

        $requestedRoundIdentStr = $this->getRequestedRoundIdentStr();
        if ((string)$roundIdent !== $requestedRoundIdentStr) {
            return false; // the username is only valid for requests targeting that specific game round
        }

        $apiPassword = $this->db->querySingleValue(
            'SELECT api_password FROM game_round WHERE api_ident = %i',
            $roundIdent
        );
        return ($apiPassword === $password);
    }

    private function getRequestedRoundIdentStr(): ?string
    {
        if ($this->currentRequest === null) {
            throw new \LogicException('Cannot authenticate - unknown request.');
        }

        if ($this->currentRequest instanceof ServerRequestInterface) {
            $routingResults = $this->currentRequest->getAttribute(RouteContext::ROUTING_RESULTS);
            if ($routingResults instanceof RoutingResults) {
                $args = $routingResults->getRouteArguments();
                if (isset($args[RouteParam::ROUND_ID])) {
                    return $args[RouteParam::ROUND_ID];
                }
            }
        }
        
        // as a fallback, if Slim didn't provide the routing results, parse from the URL
        $uriPath = $this->currentRequest->getUri()->getPath();
        if (preg_match('~^/v1/game/round/(\\d+)/~', $uriPath, $m)) {
            return $m[1];
        } else {
            return null;
        }
    }
}
