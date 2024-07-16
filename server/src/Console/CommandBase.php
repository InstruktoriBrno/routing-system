<?php
declare(strict_types=1);

namespace App\Console;

use Ivory\Connection\IConnection;
use Psr\Container\ContainerInterface;
use Psr\Http\Message\RequestInterface;
use Psr\Http\Message\ResponseInterface;
use Symfony\Component\Console\Command\Command;
use Symfony\Component\Console\Input\InputInterface;
use Symfony\Component\Console\Output\OutputInterface;
use Throwable;

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

    abstract protected function executeImpl(InputInterface $input, OutputInterface $output): void;

    protected function execute(InputInterface $input, OutputInterface $output): int
    {
        try {
            $this->executeImpl($input, $output);
            return 0;
        } catch (CommandInputException $e) {
            $output->writeln('Input error: ' . $e->getMessage());
            return 1;
        } catch (CommandRuntimeException $e) {
            $output->writeln($e->getMessage());
            return 2;
        } catch (Throwable $t) {
            $output->write(sprintf('%s: %s', get_class($t), $t->getMessage()));
            return 3;
        }
    }

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

        $handlerStack->push(\GuzzleHttp\Middleware::mapResponse(function (ResponseInterface $response) use ($output) {
            if (!$output->isQuiet() || !self::isHttpSuccess($response)) {
                $output->writeln('Response status code ' . $response->getStatusCode());
            
                $body = $response->getBody()->getContents();
                $output->writeln('Body:');
                $output->writeln($this->prettyFormatBody($body));
        
                return $response;
            }
        }));

        $this->gatewayClient = $this->container->get(\GuzzleHttp\Client::class);
        return $this->gatewayClient;
    }

    protected function processHttpClientResult(ResponseInterface $res): void
    {
        if (!self::isHttpSuccess($res)) {
            throw new CommandRuntimeException('Error result');
        }
    }

    private static function isHttpSuccess(ResponseInterface $res): bool
    {
        return ($res->getStatusCode() >= 200 && $res->getStatusCode() < 300);
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
