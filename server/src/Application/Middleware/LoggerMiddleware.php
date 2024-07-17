<?php
declare(strict_types=1);
namespace App\Application\Middleware;

use Psr\Http\Message\RequestInterface;
use Psr\Http\Message\ResponseInterface;
use Psr\Http\Server\MiddlewareInterface;
use Psr\Http\Server\RequestHandlerInterface;
use Psr\Log\LoggerInterface;

class LoggerMiddleware implements MiddlewareInterface
{
    private LoggerInterface $logger;

    public function __construct(LoggerInterface $logger)
    {
        $this->logger = $logger;
    }

    public function __invoke(callable $handler) // GuzzleHttp\Client middleware "interface"
    {
        return function (RequestInterface $request, array $options) use ($handler) {
            $promise = $handler($request, $options);
            assert($promise instanceof \GuzzleHttp\Promise\PromiseInterface);
            return $promise->then(
                function (ResponseInterface $response) use ($request) {
                    $this->logger->info(sprintf('%s --> %s', $this->formatRequest($request), $this->formatResponse($response)));
                    return $response;
                },
                function ($throwable) use ($request) {
                    $this->logger->info(sprintf('%s --> %s', $this->formatRequest($request), $throwable));
                    throw $throwable;
                }
            );
        };
    }

    public function process(RequestInterface $request, RequestHandlerInterface $handler): ResponseInterface
    {
        try {
            $response = $handler->handle($request);
            $this->logger->info(sprintf('%s --> %s', $this->formatRequest($request), $this->formatResponse($response)));
            return $response;
        } catch (\Throwable $t) {
            $this->logger->info(sprintf('%s --> %s', $this->formatRequest($request), $this->formatThrowable($t)));
            throw $t;
        }
    }

    private function formatRequest(RequestInterface $req): string
    {
        return sprintf('%s %s', $req->getMethod(), $req->getUri());
    }

    private function formatResponse(ResponseInterface $res): string
    {
        $result = sprintf('%s %s', $res->getStatusCode(), $res->getReasonPhrase());
        if ($res->getStatusCode() >= 300) {
            $body = $res->getBody()->getContents();
            if ($body !== '') {
                $result .= ": $body";
            }
        }

        return $result;
    }

    private function formatThrowable(\Throwable $t): string
    {
        return 'error: ' . $t->getMessage();
    }
}
