<?php
declare(strict_types=1);

$client = require __DIR__ . '/../app/cli-http-client.php';

$roundIdent = 1;
$password = 'foo';

$res = $client->post('/v1/game/start', ['json' => ['roundId' => $roundIdent, 'password' => $password]]);

printf('Status code %s%s', $res->getStatusCode(), PHP_EOL);
printf('Body:%s', PHP_EOL);
echo $res->getBody()->getContents(), PHP_EOL;
