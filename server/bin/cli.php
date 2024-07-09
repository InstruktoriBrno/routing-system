<?php
declare(strict_types=1);

use App\Application\Settings\SettingsInterface;
use Symfony\Component\Console\Application;

if (isset($_SERVER['REQUEST_METHOD'])) {
    echo "Only CLI allowed. Script stopped.\n";
    exit(1);
}

$container = require __DIR__ . '/../app/container.php';

$commands = $container->get(SettingsInterface::class)->get('commands');

$application = new Application();
foreach ($commands as $class) {
    $application->add($container->get($class));
}

$application->run();
