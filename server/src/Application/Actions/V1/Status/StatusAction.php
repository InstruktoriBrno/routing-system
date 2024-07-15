<?php
declare(strict_types=1);

namespace App\Application\Actions\V1\Status;

use App\Application\Actions\Action;
use Psr\Http\Message\ResponseInterface as Response;
use Psr\Log\LoggerInterface;

class StatusAction extends Action
{
    private $gatewayClient;

    public function __construct(LoggerInterface $logger, \GuzzleHttp\Client $gatewayClient)
    {
        parent::__construct($logger);
        $this->gatewayClient = $gatewayClient;
    }

    protected function action(): Response
    {
        $res = $this->gatewayClient->get('/v1/status');
        $body = $res->getBody()->getContents();
        $bodyObj = json_decode($body, true);

        return $this->respondWithJsonData($bodyObj);
    }
}
