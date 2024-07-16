<?php

declare(strict_types=1);

use App\Application\Settings\SettingsInterface;
use Slim\App;
use Tuupola\Middleware\HttpBasicAuthentication;

return function (App $app, SettingsInterface $settings) {
    if (!empty($settings->get('server')['auth'])) {
        $app->add(HttpBasicAuthentication::class);
    }
};
