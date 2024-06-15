<?php
declare(strict_types=1);

use App\Application\Settings\SettingsInterface;

$container = require __DIR__ . '/../app/container.php';

$settings = $container->get(SettingsInterface::class);
return new GuzzleHttp\Client(['base_uri' => $settings->get('gateway')['base_uri']]);
