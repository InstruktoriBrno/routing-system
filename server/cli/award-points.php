<?php
declare(strict_types=1);

$container = require __DIR__ . '/../app/container.php';

$client = new GuzzleHttp\Client();

$res = $client->post('http://routing-game/v1/game/round/4/team/A', ['json' => ['score' => 1]]);

printf('Status code %s%s', $res->getStatusCode(), PHP_EOL);
printf('Body:%s', PHP_EOL);
echo $res->getBody()->getContents(), PHP_EOL;
