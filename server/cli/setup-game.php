<?php
declare(strict_types=1);

use Ivory\Connection\IConnection;

$client = require __DIR__ . '/../app/cli-http-client.php';

$roundIdent = 1;

$db = $container->get(IConnection::class);
$db->connect();

$round = $db->querySingleTuple('SELECT * FROM game_round WHERE api_ident = %int', $roundIdent);
$spec = $round->spec->getValue();
$spec->roundId = $round->api_ident;
$spec->roundName = $round->name;
// echo json_encode($spec);

$res = $client->put('/v1/game/round', ['json' => $spec]);

printf('Status code %s%s', $res->getStatusCode(), PHP_EOL);
printf('Body:%s', PHP_EOL);
echo $res->getBody()->getContents(), PHP_EOL;
