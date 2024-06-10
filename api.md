# Common

**Router ID:** integer 0-127

**Team ID:** single uppercase letter A-Z

**Round ID:** integer 1-2147483647

**Card ID:** string
* example: `"A015"`
* format: `"<team-id><card-number>"`, where:
    * `<team-id>` is a _team ID_
    * `<card-id>` is a 3-digit card number

**Bearer ID:** string
* example: `"fe:d3:4c:aa:72:11:23"`
* format: 7 lowercase hexadecimal digit pairs separated with `:`
* this is the unique physical card ID as manufactured


# Gateway API

## ~~Get scanned cards buffer~~ (obsolete)

`GET` at `/v1/cards/list`

Useful for initial mapping of physical to logical card IDs.

Retrieves the current buffer of scanned cards. Repeated calls will result in the same list of cards plus newly scanned cards. `DELETE` at `/v1/cards/list` can be used to clear the buffer.

Physical card IDs are listed in the scanning order as FIFO. The caller is expected to handle any duplicates, if present (e.g., by ignoring duplicates if consecutive but raising an error in case of intertwined duplicates).

Response body:
```json
[
    "<physical-card-id>",
    ...
]
```

## ~~Clears scanned cards buffer~~ (obsolete)

`DELETE` at `/v1/cards/list`

Clears the buffer of cards scanned so far. Unless new cards get scanned, the subsequent `GET` at `/v1/cards/list` should return an empty list.

## ~~Cards mapping setup: set~~ (obsolete)

`PUT` at `/v1/cards/mapping`

Defines mapping of physical card IDs to (logical) card IDs. Clears any previous card mapping and sets the provided mapping as the new one (even if empty).

Request body:
```json
{
    "<physical-card-id>": "<card-id>",
    ...
}
```

## ~~Cards mapping setup: add~~ (obsolete)

`PATCH` at `/v1/cards/mapping`

Adds on top of any previously mapped cards. If a given `"<card-id>"` was previously mapped, the previous mapping gets replaced with the new one, i.e., the new `"<physical-card-id>"` gets active, and the old `"<physical-card-id>"` stops being recognized.

Request body:
```json
{
    "<physical-card-id>": "<card-id>",
    ...
}
```

## ~~Initial cards mapping: set sequence~~ (dropped - Gateway will do on its own)

`PUT` at `/v1/cards/mapping-sequence`

For assigning cards their (logical) card IDs, this operation instructs the Gateway to assign `"<card-id>"` to the next scanned card. Any subsequent cards scanned will get their card ID assigned to `"<card-id+1>"`,  `"<card-id+2>"`, etc.

Request body:
```json
{
    "nextCardId": "<card-id>"
}
```

## Round setup

`PUT` at `/v1/game/round`

Defines the round of the game which will be started with the following `POST` to `/v1/game/start`.

Request body example:
```json
{
    "round_id": 42,
    "routers": {
        "0": {
            "mac": "xx:xx:xx:xx:xx:xx",
            "nickname": "A"
        },
        "1": {
            "mac": "xx:xx:xx:xx:xx:xx",
            "nickname": "B"
        },
        "2": {
            "mac": "xx:xx:xx:xx:xx:xx",
            "nickname": "C"
        }
    },
    "links": [
        [0, 1],
        [1, 2],
        [2, 0]
    ],
    "packets": {
        "0": {
            "type": "standard",
            "source": 0,
            "destination": 2
        }
    },
    "events": [
        {
            "type": "linkdown",
            "timestamp": 3,
            "edge": [0, 1]
        },
        {
            "type": "linkup",
            "timestamp": 4,
            "edge": [0, 1]
        }
    ]
}
```

Successful response to this call means the whole network has applied the new round definition and is ready to start the game.

## Start the game

`POST` at `/v1/game/start`

Starts the game, which was previously set up with `POST` to `/v1/game/round`.

Request body:
```json
{
    "roundId": <round-id>,
    "password": "<password>",
    "currentTime": "<current-time>",
    "startTime": "<start-time>"
}
```
* `<round-id>` integer
    * a unique identifier of the round, decided by the caller
    * to be used as the URL parameter and as the username for the HTTP Basic Auth with any subsequent calls from the gateway to the server, together with `<password>`
    * needs to be the same as with the prior `POST` to `/v1/game/round`; any box having a different round set will reject to start, and the whole request will fail
* `<password>` string
    * a password to be used for the HTTP Basic Auth with any subsequent calls from the gateway to the server, together with `<round-id>`
* `<current-time>` string - UTC date/time formatted as `YYYY-MM-DDThh:mm:ssZ`
    * the date/time as currently set on the server
* `<start-time>`  string - UTC date/time formatted as `YYYY-MM-DDThh:mm:ssZ`
    * the date/time (from the server's perspective) at which the round should start


## Diagnostics: gateway + router status

`GET` at `/v1/status`

Response body:
```json
{
    "gateway": {
        
    },
    "routers": [
        { "router": "<router-id>" }
    ]
}
```
TODO: specify data above according to what the gateway can provide (router/gateway being online, last ping, status of game round definition...). Should provide network diagnostics plus pretty much any of the above `POST` requests should have some visible bits in the status report.


# Server API

## Diagnostics: gateway + router status

`GET` at `/v1/status`

Response body: same as Gateway API `GET` at `/v1/status`

## Log router events
`POST` at `/v1/game/round/<round-id>/router/<router-id>`, using HTTP Basic auth.

Request body:
```json
[
    { "time": 15, "card": "A015", "bearer": "fe:d3:4c:aa:72:11:23" },
    { "time": 25, "card": "Z999", "bearer": "fe:de:33:ab:cd:ef:00", "score": 10 }
]
```
* `<round-id>`: round ID for which to record the event
    * integer: the same as specified by the most recent `POST` to `/v1/game/start`
* `<router-id>`: router ID which emitted the event
    * string: identifier of the router as specified in the round specs by the most recent `PUT` to `/v1/game/round`
* `time`: timestamp, relative to the start of the round
    * non-negative integer: number of seconds after round start
* `card`: identifier of the card which beeped
    * card ID
* `bearer`: bearer (physical card) which beeped
    * optional bearer ID
    * for diagnostic purposes only
* `score`: points awarded for this card beep
    * optional non-negative integer

Uniqueness: there can only be one event per (`<round-id>`, `time`, `card`) combination. Even across multiple POST requests (even across events of different router IDs).
* Only the first such event will be processed, the rest will get skipped.
* Why: protects from the same events being accidentally posted repeatedly, e.g., in case of manual logs upload.

## Ad hoc points awarded to a team
`POST` at `/v1/game/round/<round-id>/team/<team-id>`, using HTTP Basic auth.

Request body:
```json
{
    "score": 4,
    "reason": "non-working endpoint"
}
```
* `score`: points awarded to the team
    * integer (even negative to award a penalty, or to correct previous mistake awarding extra points)
* `reason`: why the points were awareded
    * optional string

## Scoreboard for a team

`GET` at `/v1/score/round/<round-id>/team/<team-id>`

Response body:
```json
{
    "roundId": <round-id>,
    "roundName": "<round-name>",
    "teamId": "<team-id>",
    "overallScore": <overall-score>,
    "adHocScore": <ad-hoc-score>,
    "perPacketType": {
        "<packet-type>": {
            "thisTeamScore": <packet-type-team-score>,
            "bestTeamScore": <packet-type-best-score>
        },
        ...
    },
    "insights": {
        "invalidBeeps": <invalid-beeps>,
        "redundantBeeps": <redundant-beeps>
    },
    "subteamRatings" [
        {
            "subteamId": <subteam-id>,
            "subteamName": "<subteam-name>",
            "rating": <subteam-rating>
        },
        ...
    ]
}
```

## Overall scoreboard

`GET` at `/v1/score/round/<round-id>/overall`

Overall score after round `<round-id>`

Response body:
```json
{
    "roundId": <round-id>,
    "roundName": "<round-name>",
    "scoreboard": [
        {
            "subteamId": "<subteam-id>",
            "subteamName": "<subteam-name>",
            "rating": <subteam-rating>,
            "score": <subteam-score>
        }
    ]
}
```
* `scoreboard` is a list of teams' results, sorted from the best to the worst (sorting criteria defined by the server)
* `<team-rating>`: integer
* `<team-score>`: integer
    * overall team's score, summed up from the first until the requested round
