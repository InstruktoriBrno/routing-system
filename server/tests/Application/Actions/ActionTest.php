<?php

declare(strict_types=1);

namespace Tests\Application\Actions;

use App\Application\Actions\Action;
use App\Application\Actions\ActionPayload;
use DateTimeImmutable;
use Monolog\Logger;
use Psr\Http\Message\ResponseInterface as Response;
use Psr\Log\LoggerInterface;
use Tests\TestCase;

class ActionTest extends TestCase
{
    public function testActionSetsHttpCodeInRespond()
    {
        $logger = $this->getDummyLogger();

        $testAction = new class ($logger) extends Action {
            public function __construct(
                LoggerInterface $loggerInterface
            ) {
                parent::__construct($loggerInterface);
            }

            public function action(): Response
            {
                return $this->respondJson(
                    new ActionPayload(
                        202,
                        [
                            'willBeDoneAt' => (new DateTimeImmutable())->format(DateTimeImmutable::ATOM)
                        ]
                    )
                );
            }
        };

        $app = $this->createAppInstance();
        $app->get('/test-action-response-code', $testAction);
        $request = $this->createRequest('GET', '/test-action-response-code');
        $response = $app->handle($request);

        $this->assertEquals(202, $response->getStatusCode());
    }

    public function testActionSetsHttpCodeRespondData()
    {
        $logger = $this->getDummyLogger();

        $testAction = new class ($logger) extends Action {
            public function __construct(
                LoggerInterface $loggerInterface
            ) {
                parent::__construct($loggerInterface);
            }

            public function action(): Response
            {
                return $this->respondWithJsonData(
                    [
                        'willBeDoneAt' => (new DateTimeImmutable())->format(DateTimeImmutable::ATOM)
                    ],
                    202
                );
            }
        };

        $app = $this->createAppInstance();
        $app->get('/test-action-response-code', $testAction);
        $request = $this->createRequest('GET', '/test-action-response-code');
        $response = $app->handle($request);

        $this->assertEquals(202, $response->getStatusCode());
    }

    private function getDummyLogger(): LoggerInterface
    {
        return new Logger(__CLASS__);
    }
}
