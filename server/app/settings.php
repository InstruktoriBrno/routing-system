<?php
declare(strict_types=1);

use App\Application\Settings\Settings;
use App\Application\Settings\SettingsInterface;
use DI\ContainerBuilder;

return function (ContainerBuilder $containerBuilder): void {
    $settings = require __DIR__ . '/config/default.php';
    // Unit-test and integration environment
    if (defined('APP_ENV')) {
        require __DIR__ . '/config/env-' . basename(APP_ENV) . '.php';
    } else {
        require __DIR__ . '/config/env.php';
    }
    
    // Global Settings Object
    $containerBuilder->addDefinitions([
        SettingsInterface::class => function () use ($settings) {
            return new Settings($settings);
        }
    ]);
};
