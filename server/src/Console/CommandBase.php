<?php
declare(strict_types=1);

namespace App\Console;

use App\Application\Settings\SettingsInterface;
use GuzzleHttp\Handler\MockHandler;
use GuzzleHttp\Psr7\Response;
use Ivory\Connection\IConnection;
use Psr\Container\ContainerInterface;
use Psr\Http\Message\RequestInterface;
use Psr\Http\Message\ResponseInterface;
use Symfony\Component\Console\Command\Command;
use Symfony\Component\Console\Output\OutputInterface;

abstract class CommandBase extends Command
{
    protected ContainerInterface $container;
    private $gatewayClient = null;

    public function __construct(ContainerInterface $container, ?string $name = null)
    {
        parent::__construct($name);

        $this->container = $container;
        $this->define();
    }

    abstract protected function define(): void;

    protected function getDb(): IConnection
    {
        $db = $this->container->get(IConnection::class);
        $db->connect();
        return $db;
    }

    protected function getGatewayClient(OutputInterface $output): \GuzzleHttp\Client
    {
        if ($this->gatewayClient !== null) {
            return $this->gatewayClient;
        }

        $handlerStack = $this->container->get(\GuzzleHttp\HandlerStack::class);
        assert($handlerStack instanceof \GuzzleHttp\HandlerStack);

        if ($output->isVerbose() || $output->isVeryVerbose()) {
            $handlerStack->push(\GuzzleHttp\Middleware::mapRequest(function (RequestInterface $request) use ($output) {
                $output->writeln(sprintf('%s %s', $request->getMethod(), $request->getUri()));

                $body = $request->getBody()->getContents();
                $output->writeln('Body:');

                $bodyFmt = ($output->isVeryVerbose() ? $this->prettyFormatBody($body) : $body);
                $output->writeln($bodyFmt);
        
                return $request;
            }));
        }

        if (!$output->isQuiet()) {
            $handlerStack->push(\GuzzleHttp\Middleware::mapResponse(function (ResponseInterface $response) use ($output) {
                $output->writeln('Response status code ' . $response->getStatusCode());
            
                $body = $response->getBody()->getContents();
                $output->writeln('Body:');
                $output->writeln($this->prettyFormatBody($body));
        
                return $response;
            }));
        }

        $this->gatewayClient = $this->container->get(\GuzzleHttp\Client::class);
        return $this->gatewayClient;
    }

    protected function processHttpClientResult(ResponseInterface $res): int
    {
        $exitCode = ($res->getStatusCode() >= 200 && $res->getStatusCode() < 300 ? 0 : 2);
        return $exitCode;
    }

    private function prettyFormatBody(string $body): string
    {
        if ($body === '') {
            return '<empty body>';
        }

        $bodyJson = json_decode($body, false);
        if ($bodyJson !== null) {
            return json_encode($bodyJson, JSON_PRETTY_PRINT);
        } else {
            return $body;
        }
    }
}
