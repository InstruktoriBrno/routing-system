<?php
declare(strict_types=1);

$client = require __DIR__ . '/../app/cli-http-client.php';

$res = $client->post('/v1/game/pause');

printf('Status code %s%s', $res->getStatusCode(), PHP_EOL);
printf('Body:%s', PHP_EOL);
echo $res->getBody()->getContents(), PHP_EOL;
