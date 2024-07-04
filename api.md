# Common

**Router ID:** single uppercase letter A-Z

**Team ID:** single uppercase letter A-Z

**Round ID:** integer 1-32767

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

## Round setup

`PUT` at `/v1/game/round`

Defines the round of the game which will be started with the following `POST` to `/v1/game/start`.

Request body:
```json
{
    "roundId": <round-id>,
    "roundName": <round-name>,
    "duration": <duration>,
    "routers": {
        "<router-id>": {
            "mac": [
                "<router-mac>",
                ...
            ]
        },
        ...
    },
    "links": [
        "<link-from-router-id><link-to-router-id>",
        ...
    ],
    "packets": {
        "<card-number>": {
            "type": "<packet-type>",
            ...packet-params
        },
        ...
    },
    "events": [
        {
            "type": "<event-type>",
            "time": "<event-time>",
            ...event-params
        },
        ...
    ]
}
```
* `<round-id>`: ID of round which will be played
    * integer 1-32767
    * only present for this API endpoint; omitted in the round specs library
    * useful for boxes to check, upon game start (`POST` at `/v1/game/start`), whether the box has the correct round specification
* `<round-name>`: A label for this round
* `<duration>`: how long the round will take
    * non-negative integer: number of seconds
    * any events occurring in the game must have their `<time>` less than `<duration>`
* `<router-id>`: ID of router being active in the network
* `<router-mac>`: physical address of a box representing router `<router-id>`
    * there may be multiple boxes representing the same router (useful in case of replacing a malfunctioning box)
    * example: `"fe:d3:4c:aa:72:11:23"`
    * string; format: 6 lowercase hexadecimal digit pairs separated with `:`
* `<link-from-router-id>`, `<link-to-router-id>`: IDs of routers being linked
    * the link is bi-directional (e.g., `"AB"` is equivalent to `"BA"`); for readability, lexicographical order should be maintained (e.g., specify `"AB"` rather than `"BA"`)
* `<card-number>`: identifier of the cards which can play in the game round
    * string containing a non-negative integer less than 1000, left-padded with zeroes
    * example: `"035"`
    * essentially the same as _card ID_, except the first letter (identifying a team) is ommitted - leveraging the fact each team is supposed to play with an equivalent set of packets
* `events`: list of events affecting the topology
    * order of elements in the list does not matter, their `<event-time>` is relevant; for readability, though, events should be sorted by `<event-time>` in ascending order
* `<packet-type>`: type of the packet the card represents:
    * `"admin"`: Administrator packet - shows current router ID, even outside round
        * by convention, packet `"000"` is always an admin packet
    * ~~`"standard"`: packet to be delivered from one router to another~~
        * ~~score defined by the `"points"` attribute (usually based on shortest path length)~~
        * (deprecated)
    * `"return"`: Packet to be delivered to a destination, then back to the source
        * score: the `points`
    * `"priority"`: Similar to standard packet, only the reward is based on the delivery time. Upon delivery, the point reward is  `pointsPerMinutesLeft * (minutesToDeliver-<timeSpent>+1)`, minimum is `0`.
        * **Label**: Na doručení tohoto paketu máte 5 minut. Čím rychleji doručíte, tím víc bodů.
    * ~~`"hopper"`:~~
        * ~~This packet awards points for every successful hop it takes.~~
        * ~~**Label**: Za každý hop vám tento paket dá 1 bod.~~
        * (deprecated, unless we find this useful again)
    * `"visitall"`:
        * Points are awarded once this packet has visited every single router in the network.
        * **Label**: Jakmile tento paketnavštíví všechy routery, dostanete 50 bodů.
    * `"locator"`: Used to locate a predefined router.
        * The `"source"` router always shows a success screen, all other routers show a fail screen - including pre-game.
        * Never awards points
* `...packet-params`: attributes according to the packet type:
    * mandatory attributes:
        * `"releaseTime"`: integer
            * Time when the packet should be delivered to the Router box by organisers
            * Not enforced by the game. This is for the game itinerary automation tool
        * `"source": "<router-id>"`
            * Mandatory for all game packet types.
            * ID of router where the packet gets added to the network
    * `"admin"`:
        * No optional properties
    * `"return"`:
        * `"destination": "<router-id>"`
            * ID of router where the packet is to be delivered
        * `"points": <number>`
            * Base for points awarded for successful delivery
            * For delivering from `source` to `destination`, (`points` - 2x`hops` - `deliveryMinutes`) points are awarded, where:
                * `hops` is the number of routers the packet gets beeped (excluding source, including destination; each beep counted even when on a router already visited while delivering)
                * `deliveryMinutes` is the number of whole minutes it takes to delivery the packet to `destination` from `deliveryStartTime`
                * `deliveryStartTime` is either time of the first beep of the packet at `source`, or `releaseTime`+30, whichever happens first
            * For delivering back from `destination` to `source`, (`points` - 2x`hops` - `deliveryMinutes`) points are awarded (on top of the points for `source` -> `destination` delivery), where:
                * `hops` and `deliveryMinutes` are analogous to `source` -> `destination` delivery
                * `deliveryStartTime` is the time of the first beep of the packet at `destination`
            * Points must be explicitly specified.
            * Intent is for `points` to typically be 5x`optimalHops`, where `optimalHops` is the length of the optimal path from `source` to `destination` on the topology at `releaseTime`
    * `"chat"`:
        * same parameters as for type `"return'`
            * for subsequent `source` -> `destination` deliveries, `deliveryStartTime` is the time of the beep at `source` concluding the preceding `destination` -> `source` delivery)
        * `"roundTripCount"`: integer
            * Number of `source` -> `destination` -> `source` deliveries the packet supports until it declares it's done
        * `"messages"`: array of strings
            * Messages to show on the box display (a conversation, a joke, ...)
            * Contains 2x`roundTripCount` strings, each corresponding to one supported delivery.
    * `"priority"`:
        * `"destination": "<router-id>"`
            * ID of router where the packet is to be delivered
        * `"pointsPerMinuteLeft": <number>`
            * Multiplier for the time left
            * Default = 4
        * `"minutesToDeliver": <number>`
            * Maximum time to deliver. After this time, the reward is 0.
            * Default = 5
    * `"hopper"`:
        * `"pointsPerHop": <number>`
            * Points awarded for every successful hop
            * Default = 1
    * `"visitall"`:
        * `"points"`:
            * Points awarded once all routers have been visited
            * Default = 10 (recommended = 60)
    * `"locator"`:
        * No optional properties
* `<event-type>`: type of the event:
    * `"linkdown"`: a link gets deactivated at `<event-time>`
        * since `<event-time>`, the routers stop accepting packets being transmitted between these routers
        * corresponding physical action (disabling the link box) should be performed >30 seconds before `<event-time>` to prevent race conditions (e.g., a player picking up a packet from the link and bringing it to the router - then the router should accept the packet if within 30 seconds of the physical closure of the link box)
    * `"linkup"`: new link gets activated at `<event-time>`
        * since `<event-time>`, the routers being linked start accepting packets being transmitted between these routers
        * corresponding physical action (enabling the link box) can be performed exactly at `<event-time>` or with a minor delay (by the time players might want to deliver packets using the new link, the routers should have already applied the event on the topology)
* `<event-time>`: when the event happens
    * non-negative integer: number of seconds after round start
* `...event-params`: attributes according to the event type:
    * `"linkdown"`, `"linkup"`:
        * `"link": "<from-router-id><to-router-id>"`
            * the affected link

Request body example:
```json
{
    "roundId": 42,
    "duration": 600,
    "routers": {
        "A": {
            "mac": ["xx:xx:xx:xx:xx:xx"]
        },
        "B": {
            "mac": ["xx:xx:xx:xx:xx:xx", "yy:yy:yy:yy:yy:yy"]
        },
        "C": {
            "mac": ["xx:xx:xx:xx:xx:xx"]
        }
    },
    "links": [
        "AB",
        "BC",
        "AC"
    ],
    "packets": {
        "001": {
            "type": "locator",
            "releaseTime": 0,
            "source": "A"
        },
        "035": {
            "type": "standard",
            "releaseTime": 30,
            "source": "A",
            "destination": "C",
            "points": 10
        }
    },
    "events": [
        {
            "type": "linkdown",
            "time": 180,
            "link": "AB"
        },
        {
            "type": "linkup",
            "time": 240,
            "link": "AB"
        }
    ]
}
```

Successful response to this call means the gateway accepted the round specification and started to upload it to boxes. Before starting the game, `GET` at `/v1/status` should be used to check readiness.

## Start the game

`POST` at `/v1/game/start`

Starts the game, which was previously set up with `POST` to `/v1/game/round`. The game starts immediately.

Request body:
```json
{
    "roundId": <round-id>,
    "password": "<password>"
}
```
* `<round-id>`: a unique identifier of the round, decided by the caller
    * integer 1-32767
    * to be used as the URL parameter and as the username for the HTTP Basic Auth with any subsequent calls from the gateway to the server, together with `<password>`
    * needs to be the same as with the prior `POST` to `/v1/game/round`; any box having a different round set will reject to start, and the whole request will fail
* `<password>` string
    * a password to be used for the HTTP Basic Auth with any subsequent calls from the gateway to the server, together with `<round-id>`


## Pause the game

`POST` at `/v1/game/pause`

Pauses the game. The game time stops.


## Resume the game

`POST` at `/v1/game/resume`

Resumes the game previously paused by `POST` to `/v1/game/pause`. The game resumes immediately. The game time continues from where it stopped when paused.


## Diagnostics: gateway + router status

`GET` at `/v1/status`

Response body:
```json
{
    "boxes": {
        "48:e7:29:a4:34:fc": {
            "active_round_hash": "0c301d8ceb1960cf70d98bfef2aa1a93b54c5ce7d4e7f065a4119ff7e0965e20",
            "active_round_id": 0,
            "game_state": "NOT_RUNNING",
            "game_time": 0,
            "last_seen": 1718347098.48914,
            "node_type": 1,
            "parent": "24:0a:c4:82:85:a1",
            "round_download_progress": 100,
            "router_id": "N/A"
        }
    },
    "gateway": {
        "game_state": "NOT_RUNNING",
        "game_time": 0,
        "round": 0,
        "round_hash": "0c301d8ceb1960cf70d98bfef2aa1a93b54c5ce7d4e7f065a4119ff7e0965e20",
        "time": 1718347098.86965
    }
}
```

TODO: Write proper specification


# Server API

## Diagnostics: gateway + router status

`GET` at `/v1/status`

Response body: same as Gateway API `GET` at `/v1/status`

## Log router events
`POST` at `/v1/game/round/<round-id>/router/<router-id>`, using HTTP Basic auth.

Request body:
```json
{
    "routerMac": "<router-mac>",
    "source": "<event-source>",
    "events": [
        { "time": 15, "card": "A015", "bearer": "fe:d3:4c:aa:72:11:23" },
        { "time": 25, "card": "Z999", "bearer": "fe:de:33:ab:cd:ef:00", "score": 10 }
    ]
}
```
* `<round-id>`: round ID for which to record the event
    * integer: the same as specified by the most recent `POST` to `/v1/game/start`
* `<router-id>`: router ID which emitted the event
    * string: identifier of the router as specified in the round specs by the most recent `PUT` to `/v1/game/round`
* `<router-mac>`: physical address of the box which emitted the event
    * string: MAC address in the notation like `"fe:d3:4c:aa:72:11"`
* `<event-source>`: from what source the events were reported
    * string, one of:
        * `"online"`: reported online from the router through the network
        * `"offline_box"`: downloaded from the persistent storage of the box
        * `"offline_card"`: downloaded from the card
        * `"ui"`: entered manually through the server UI
* `time`: timestamp, relative to the start of the round
    * non-negative integer: number of seconds after round start
* `card`: identifier of the card which beeped
    * card ID
* `bearer`: bearer (physical card) which beeped
    * optional bearer ID
    * for diagnostic purposes only
* `score`: points awarded for this card beep
    * optional integer

Uniqueness: there can only be one event per (`<round-id>`, `time`, `card`) combination. Even across multiple POST requests (even across events of different router IDs).
* Only the first such event will be processed, the rest will get skipped.
* Why: protects from the same events being accidentally posted repeatedly, e.g., in case of manual logs upload.

Response body:
```
{
    "insertCnt": <cnt>
}
```
* `<cnt>`: number of records actually inserted into the database
    * integer

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
* `reason`: why the points were awarded
    * optional string

## Game round setup validation

`GET` at `/v1/game/round/<round-id>/validate-setup`, using HTTP Basic auth.

Validates the setup for a game round.

Response body:
* If no errors are found, response is just `OK`.
* If errors are found, response contains the list of errors, each on a new line.

## Game round setup instructions for packets

1. `GET` at `/v1/game/round/<round-id>/instructions/packets`, using HTTP Basic auth.
2. `GET` at `/v1/game/round/<round-id>/instructions/packets/<router-id-list>`, using HTTP Basic auth.

Provides instructions on which packets to drop where at what time.
* `<router-id-list>`: if provided, only instructions for the listed routers are produced
    * optional string - concatenation of router IDs
    * example: `ADGHI` for instructions involving only these 5 routers

Response body (plaintext):
```
<card-number>: <router-id>, <time>, <packet-type>
...
```

Response body example:
```
001: A, 2:00, standard
005: C, 3:00, standard
014: H, 8:30, hopper
```


## Game round setup instructions for game events

`GET` at `/v1/game/round/<round-id>/instructions/events`, using HTTP Basic Auth.

Provides instructions on what game event to perform at what time.

Response body (plaintext):
```
<time>: <event-type> <event-params>
...
```
or, in case of no game events defined:
```
zadne herni udalosti v tomto kole
```

Response body example:
```
2:00 vypnout linku AB
5:30 zapnout linku AB
8:00 vypnout linku CD
```


## Router check-in status

`GET` at `/v1/game/round/<round-id>/checkin`

Retrieves the status of players checking in to routers before the round is started.

Response body:
```json
{
    "roundId": <round-id>,
    "checkedIn": [
        { "router": "<router-id>", "teamId": "<team-id>", "card": "<card-id>" },
        ...
    ],
    "missing": [
        { "router": "<router-id>", "teamId": "<team-id>" },
        ...
    ]
}
```
* `<router-id>`: ID of router which has (not yet) been checked in by the given team
* `<team-id>`: team who have (not yet) checked in on the given router
* `<card-id>`: card used for checking in


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
