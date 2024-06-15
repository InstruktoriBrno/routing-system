<?php
declare(strict_types=1);

$client = require __DIR__ . '/../app/cli-http-client.php';

$res = $client->get('/v1/status');

printf('Status code %s%s', $res->getStatusCode(), PHP_EOL);
printf('Body:%s', PHP_EOL);

$body = $res->getBody()->getContents();
echo json_encode(json_decode($body, false), JSON_PRETTY_PRINT), PHP_EOL;
