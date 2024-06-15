<?php
declare(strict_types=1);

$container = require __DIR__ . '/../app/container.php';

$client = new GuzzleHttp\Client();

$res = $client->get('http://routing-game/v1/score/round/4/team/A');

printf('Status code %s%s', $res->getStatusCode(), PHP_EOL);
printf('Body:%s', PHP_EOL);
echo $res->getBody()->getContents(), PHP_EOL;
