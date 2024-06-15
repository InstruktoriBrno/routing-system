<?php
declare(strict_types=1);

namespace App\Application\Actions\V1\Status;

use App\Application\Actions\Action;
use Psr\Http\Message\ResponseInterface as Response;

class StatusAction extends Action
{
    protected function action(): Response
    {
        // TODO: add param to find out gateway host and port
        return $this->respondWithJsonData([
            'gateway' => new \stdClass(),
            'routers' => [
                [
                    'router' => 'A',
                ],
            ],
        ]);
    }
}
