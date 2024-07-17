<?php
declare(strict_types=1);

namespace App\Console;

use App\Application\Middleware\LoggerMiddleware;
use App\Application\Settings\SettingsInterface;
use Ivory\Connection\IConnection;
use Monolog\Handler\StreamHandler;
use Monolog\Logger;
use Psr\Container\ContainerInterface;
use Psr\Http\Message\RequestInterface;
use Psr\Http\Message\ResponseInterface;
use Psr\Log\LoggerInterface;
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
            $exitMessage = null;
            $exitCode = 0;
        } catch (CommandInputException $e) {
            $exitMessage = 'Input error: ' . $e->getMessage();
            $exitCode = 1;
        } catch (CommandRuntimeException $e) {
            $exitMessage = $e->getMessage();
            $exitCode = 2;
        } catch (Throwable $t) {
            $exitMessage = sprintf('%s: %s', get_class($t), $t->getMessage());
            $exitCode = 3;
        }

        if ($exitMessage !== null) {
            $output->writeln($exitMessage);
        }

        $logger = $this->container->get(LoggerInterface::class);
        $logger->info(sprintf('%s --> %d %s', $input->__toString(), $exitCode, $exitMessage));
        
        return $exitCode;
    }

    protected static function getStringArgument(InputInterface $input, string $name, string $pregPattern): string
    {
        $arg = $input->getArgument($name);
        if (!preg_match($pregPattern, $arg)) {
            throw new CommandInputException("Argument `{$name}` is expected to match PCRE `$pregPattern`");
        }
        
        return $arg;
    }

    protected static function getIntArgument(InputInterface $input, string $name, int $minValue = PHP_INT_MIN, int $maxValue = PHP_INT_MAX): int
    {
        $arg = $input->getArgument($name);
        $int = filter_var($arg, FILTER_VALIDATE_INT, [
            'options' => [
                'min_range' => $minValue,
                'max_range' => $maxValue,
            ]
        ]);
        if ($int === false) {
            $message = "Argument `{$name}` is expected to be an integer";
            if ($minValue > PHP_INT_MIN && $maxValue < PHP_INT_MAX) {
                $message .= " in range [$minValue, $maxValue]";
            } elseif ($minValue > PHP_INT_MIN) {
                $message .= " >= $minValue";
            } elseif ($maxValue < PHP_INT_MAX) {
                $message .= " <= $maxValue";
            }
            throw new CommandInputException($message);
        }

        return $int;
    }

    protected static function getRoundIdentArgument(InputInterface $input, string $name): int
    {
        return self::getIntArgument($input, $name, 1, 32767);
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

        $cfg = $this->container->get(SettingsInterface::class)->get('gateway-client-logger');
        $logger = new Logger($cfg['name']);
        $logger->pushHandler(new StreamHandler($cfg['path'], $cfg['level']));
        $handlerStack->push(new LoggerMiddleware($logger));

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
