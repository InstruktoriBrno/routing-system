<?php

declare(strict_types=1);

namespace App\Application\Actions;

use App\Application\Actions\Action;
use Psr\Http\Message\ResponseInterface as Response;
use Slim\Exception\HttpNotImplementedException;

class NotImplementedAction extends Action
{
    protected function action(): Response
    {
        throw new HttpNotImplementedException($this->request);
    }
}
