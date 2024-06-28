<?php
declare(strict_types=1);

$container = require __DIR__ . '/../app/container.php';

$client = new GuzzleHttp\Client();

$res = $client->post(
    'http://routing-game/v1/game/round/82/router/C',
    [
        'json' => [
            "routerMac" => "fe:d3:4c:aa:72:11",
            "source" => "online",
            "events" => [
                [ "time" => 15, "card" => "A015", "bearer" => "fe:d3:4c:aa:72:11:23" ],
                [ "time" => 25, "card" => "Z999", "bearer" => "fe:de:33:ab:cd:ef:00", "score" => 10 ],
            ],
        ],
    ]
);

printf('Status code %s%s', $res->getStatusCode(), PHP_EOL);
printf('Body:%s', PHP_EOL);
echo $res->getBody()->getContents(), PHP_EOL;
