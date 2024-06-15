<?php
declare(strict_types=1);

use Ivory\Connection\IConnection;

$container = require __DIR__ . '/app/container.php';

$db = $container->get(IConnection::class);
$db->connect();
$rel = $db->query('SELECT * FROM game ORDER BY game_date DESC, name, id');
foreach ($rel as $t) {
    printf('%s on %s, gateway %s%s', $t->name, $t->game_date->format('Y-m-d'), $t->gateway_address, PHP_EOL);
}
