<?php
declare(strict_types=1);

namespace App\Console;

use App\Application\Settings\SettingsInterface;
use Ivory\Connection\IConnection;
use Psr\Container\ContainerInterface;
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

    protected function getGatewayClient(): \GuzzleHttp\Client
    {
        $cfg = $this->container->get(SettingsInterface::class)->get('gateway');
        return new \GuzzleHttp\Client(['base_uri' => $cfg['base_uri']]);
    }

    protected function processHttpClientResult(ResponseInterface $res, OutputInterface $output): int
    {
        $output->writeln('Status code ' . $res->getStatusCode());
        $output->writeln('Body:');

        $body = $res->getBody()->getContents();
        $bodyJson = json_decode($body, false);
        if ($bodyJson !== null) {
            $output->writeln(json_encode($bodyJson, JSON_PRETTY_PRINT));
        } else {
            $output->writeln($body);
        }

        $exitCode = ($res->getStatusCode() >= 200 && $res->getStatusCode() < 300 ? 0 : 2);
        return $exitCode;
    }
}
