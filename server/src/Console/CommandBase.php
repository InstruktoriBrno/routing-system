<?php
declare(strict_types=1);

namespace App\Console;

use App\Application\Settings\SettingsInterface;
use Ivory\Connection\IConnection;
use Psr\Container\ContainerInterface;
use Psr\Http\Message\RequestInterface;
use Psr\Http\Message\ResponseInterface;
use Symfony\Component\Console\Command\Command;
use Symfony\Component\Console\Output\OutputInterface;

abstract class CommandBase extends Command
{
    protected ContainerInterface $container;

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
        $cfg = $this->container->get(SettingsInterface::class)->get('gateway');

        $handlerStack = \GuzzleHttp\HandlerStack::create();
        if ($output->isVerbose()) {
            $handlerStack->push(\GuzzleHttp\Middleware::mapRequest(function (RequestInterface $request) use ($output) {
                $output->writeln(sprintf('%s %s', $request->getMethod(), $request->getUri()));

                $body = $request->getBody()->getContents();
                $output->writeln('Body:');
                $output->writeln($this->prettyFormatBody($body));
        
                return $request;
            }));
        }

        return new \GuzzleHttp\Client([
            'base_uri' => $cfg['base_uri'],
            'handler' => $handlerStack,
        ]);
    }

    protected function processHttpClientResult(ResponseInterface $res, OutputInterface $output): int
    {
        $output->writeln('Status code ' . $res->getStatusCode());
        
        $body = $res->getBody()->getContents();
        $output->writeln('Body:');
        $output->writeln($this->prettyFormatBody($body));

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
