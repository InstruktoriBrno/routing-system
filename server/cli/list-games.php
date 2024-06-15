<?php
declare(strict_types=1);

use Ivory\Connection\IConnection;

$container = require __DIR__ . '/../app/container.php';

$db = $container->get(IConnection::class);
$db->connect();

$rel = $db->query('SELECT * FROM game ORDER BY game_date DESC, name, id');
foreach ($rel as $t) {
    printf("%s\t%s, gateway %s\n", $t->game_date->format('Y-m-d'), $t->name, $t->gateway_address);
}
