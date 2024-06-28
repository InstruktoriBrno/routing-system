<?php

declare(strict_types=1);

namespace App\Application\Actions;

use App\Domain\DomainException\DomainRecordNotFoundException;
use JsonException;
use Psr\Http\Message\ResponseInterface as Response;
use Psr\Http\Message\ServerRequestInterface as Request;
use Psr\Log\LoggerInterface;
use Slim\Exception\HttpBadRequestException;
use Slim\Exception\HttpNotFoundException;
use Swaggest\JsonSchema\Schema;
use Swaggest\JsonSchema\Exception as JsonSchemaException;

abstract class Action
{
    protected LoggerInterface $logger;
    protected Request $request;
    protected Response $response;
    protected array $args;

    public function __construct(LoggerInterface $logger)
    {
        $this->logger = $logger;
    }

    /**
     * @throws HttpNotFoundException
     * @throws HttpBadRequestException
     */
    public function __invoke(Request $request, Response $response, array $args): Response
    {
        $this->request = $request;
        $this->response = $response;
        $this->args = $args;

        try {
            return $this->action();
        } catch (DomainRecordNotFoundException $e) {
            throw new HttpNotFoundException($this->request, $e->getMessage());
        }
    }

    /**
     * @throws DomainRecordNotFoundException
     * @throws HttpBadRequestException
     */
    abstract protected function action(): Response;

    /**
     * @return array|object
     */
    protected function getFormData()
    {
        return $this->request->getParsedBody();
    }

    /**
     * @return mixed
     * @throws HttpBadRequestException
     */
    protected function resolveArg(string $name)
    {
        if (!isset($this->args[$name])) {
            throw new HttpBadRequestException($this->request, "Could not resolve argument `{$name}`.");
        }

        return $this->args[$name];
    }

    protected function resolveIntArg(string $name, int $minValue = PHP_INT_MIN, int $maxValue = PHP_INT_MAX): int
    {
        $arg = $this->resolveArg($name);
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
            throw new HttpBadRequestException($this->request, $message);
        }

        return $int;
    }

    protected function resolveStringArg(string $name, string $pregPattern): string
    {
        $arg = $this->resolveArg($name);
        if (!preg_match($pregPattern, $arg)) {
            throw new HttpBadRequestException($this->request, "Argument `{$name}` is expected to match PCRE `$pregPattern`");
        }
        
        return $arg;
    }

    protected function getValidatedBody(string $jsonSchema): \stdClass
    {
        try {
            $schema = Schema::import(json_decode($jsonSchema));
        } catch (\Exception $e) {
            throw new \RuntimeException('Error importing the JSON schema', 0, $e);
        }

        try {
            $body = $this->request->getBody();
            $bodyObject = json_decode($body->getContents(), null, 512, JSON_THROW_ON_ERROR);
            $schema->in($bodyObject);
        } catch (JsonException $e) {
            throw new HttpBadRequestException($this->request, 'Invalid payload - not a valid JSON: ' . $e->getMessage(), $e);
        } catch (JsonSchemaException $e) {
            // NOTE: Catching the generic JsonSchemaException which might be thrown by
            //       \Swaggest\JsonSchema\RefResolver::preProcessReferences() despite
            //       \Swaggest\JsonSchema\SchemaContract::in() declares throwing just \Swaggest\JsonSchema\InvalidValue
            throw new HttpBadRequestException($this->request, 'Invalid payload: ' . $e->getMessage(), $e);
        }

        return $bodyObject;
    }

    /**
     * @param array|object|null $data
     */
    protected function respondWithJsonData($data = null, int $statusCode = 200): Response
    {
        $payload = new ActionPayload($statusCode, $data);

        return $this->respondJson($payload);
    }

    protected function respondJson(ActionPayload $payload): Response
    {
        $json = json_encode($payload, JSON_PRETTY_PRINT);
        $this->response->getBody()->write($json);

        return $this->response
                    ->withHeader('Content-Type', 'application/json')
                    ->withStatus($payload->getStatusCode());
    }

    protected function respondWithPlaintextData($data = null, int $statusCode = 200): Response
    {
        $payload = new ActionPayload($statusCode, $data);

        return $this->respondPlaintext($payload);
    }

    protected function respondPlaintext(ActionPayload $payload): Response
    {
        $this->response->getBody()->write($payload->getData());

        return $this->response
                    ->withHeader('Content-Type', 'text/plain')
                    ->withStatus($payload->getStatusCode());
    }
}
