TRUNCATE subteam, round_spec_lib, game, game_round, game_round_team, game_round_event;

ALTER SEQUENCE subteam_id_seq RESTART WITH 1;
ALTER SEQUENCE round_spec_lib_id_seq RESTART WITH 1;
ALTER SEQUENCE game_id_seq RESTART WITH 1;
ALTER SEQUENCE game_round_id_seq RESTART WITH 1;
ALTER SEQUENCE game_round_api_ident_seq RESTART WITH 1;
ALTER SEQUENCE game_round_team_id_seq RESTART WITH 1;
ALTER SEQUENCE game_round_event_id_seq RESTART WITH 1;

INSERT INTO subteam (name) VALUES
    ('A-Team'),
    ('B-Team'),
    ('Kokokel'),
    ('Kvík!');

INSERT INTO round_spec_lib (name, is_deleted, spec) VALUES
    ('Debug round', FALSE, $$
        {
            "duration": 300,
            "routers": {
                "A": { "mac": ["e4:65:b8:77:1c:0c"] },
                "B": { "mac": ["e4:65:b8:76:b6:cc"] },
                "C": { "mac": ["d8:bc:38:fd:84:bc"] }
            },
            "links": ["AB", "AC"],
            "packets": {
                "001": { "type": "locator", "releaseTime": 0, "source": "A" },
                "002": { "type": "locator", "releaseTime": 0, "source": "B" },
                "003": { "type": "locator", "releaseTime": 0, "source": "C" },
                "011": { "type": "standard", "releaseTime": 20, "source": "A", "destination": "B", "points": 10 },
                "012": { "type": "standard", "releaseTime": 40, "source": "A", "destination": "C", "points": 20 },
                "013": { "type": "standard", "releaseTime": 60, "source": "B", "destination": "C", "points": 30 },
                "014": { "type": "standard", "releaseTime": 80, "source": "B", "destination": "C", "points": 40 }
            },
            "events": []
        }
    $$);

INSERT INTO game (name, game_date, gateway_address) VALUES
    ('Debug game', '2024-06-15', NULL);

INSERT INTO game_round (game_id, name, spec, server_start_time, api_password)
    SELECT game.id, 'Debug round', round_spec_lib.spec, '2024-06-15 12:30:00', 'Baz'
    FROM game, round_spec_lib
    WHERE game.name = 'Debug game' AND round_spec_lib.name = 'Debug round'
    ;

INSERT INTO game_round_team (game_round_id, team_ident, subteam_id)
    SELECT game_round.id, 'A', subteam.id
    FROM
        game JOIN game_round ON game_round.game_id = game.id,
        subteam
    WHERE game.name = 'Debug game' AND game_round.name = 'Debug round' AND subteam.name IN ('A-Team', 'B-Team', 'Kokokel')
    ;
