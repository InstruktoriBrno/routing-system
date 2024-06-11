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
    ('Tutorial', FALSE, $$
        {
            "duration": 600,
            "routers": {
                "A": { "mac": ["11:12:13:14:15:16"] },
                "B": { "mac": ["11:12:13:14:15:26"] },
                "C": { "mac": ["11:12:13:14:15:36"] }
            },
            "links": ["AB", "BC", "AC"],
            "packets": {
                "000": { "type": "checkin", "destination": "A" },
                "001": { "type": "checkin", "destination": "B" },
                "002": { "type": "checkin", "destination": "C" },
                "010": { "type": "standard", "source": "A", "destination": "B" },
                "011": { "type": "standard", "source": "A", "destination": "B" },
                "012": { "type": "standard", "source": "A", "destination": "B" },
                "020": { "type": "standard", "source": "B", "destination": "A" },
                "030": { "type": "standard", "source": "B", "destination": "C" }
            },
            "events": []
        }
    $$),
    ('Minimal, deleted', TRUE, $$
        {
            "duration": 1,
            "routers": {
                "A": { "mac": ["11:12:13:14:15:16"] }
            },
            "links": [],
            "packets": {},
            "events": []
        }
    $$),
    ('Circle round', FALSE, $$
        {
            "duration": 1200,
            "routers": {
                "A": { "mac": ["11:12:13:14:15:16"] },
                "B": { "mac": ["11:12:13:14:15:26", "11:12:13:14:15:f6"] },
                "C": { "mac": ["11:12:13:14:15:36"] },
                "D": { "mac": ["11:12:13:14:15:46"] },
                "E": { "mac": ["11:12:13:14:15:56"] },
                "F": { "mac": ["11:12:13:14:15:66"] },
                "G": { "mac": ["11:12:13:14:15:76"] },
                "H": { "mac": ["11:12:13:14:15:86"] },
                "I": { "mac": ["11:12:13:14:15:96"] },
                "J": { "mac": ["11:12:13:14:15:a6"] }
            },
            "links": ["AB", "BC", "CD", "DE", "EF", "FG", "GH", "HI", "IJ", "AJ"],
            "packets": {
                "000": { "type": "checkin", "destination": "A" },
                "001": { "type": "checkin", "destination": "B" },
                "002": { "type": "checkin", "destination": "C" },
                "003": { "type": "checkin", "destination": "D" },
                "004": { "type": "checkin", "destination": "E" },
                "005": { "type": "checkin", "destination": "F" },
                "006": { "type": "checkin", "destination": "G" },
                "007": { "type": "checkin", "destination": "H" },
                "008": { "type": "checkin", "destination": "I" },
                "009": { "type": "checkin", "destination": "J" },
                "010": { "type": "standard", "source": "A", "destination": "B" },
                "011": { "type": "standard", "source": "A", "destination": "C" },
                "012": { "type": "standard", "source": "A", "destination": "D" },
                "013": { "type": "standard", "source": "A", "destination": "E" },
                "014": { "type": "standard", "source": "A", "destination": "F" },
                "015": { "type": "standard", "source": "A", "destination": "G" },
                "016": { "type": "standard", "source": "A", "destination": "H" },
                "017": { "type": "standard", "source": "A", "destination": "I" },
                "018": { "type": "standard", "source": "A", "destination": "J" }
            },
            "events": [
                { "type": "linkup", "time": 60, "link": "AF" },
                { "type": "linkdown", "time": 120, "link": "AF" },
                { "type": "linkup", "time": 180, "link": "AF" },
                { "type": "linkdown", "time": 240, "link": "AF" },
                { "type": "linkup", "time": 300, "link": "AF" },
                { "type": "linkdown", "time": 360, "link": "AF" },
                { "type": "linkup", "time": 420, "link": "AF" },
                { "type": "linkdown", "time": 480, "link": "AF" },
                { "type": "linkup", "time": 540, "link": "AF" },
                { "type": "linkdown", "time": 600, "link": "AF" },
                { "type": "linkup", "time": 660, "link": "AC" },
                { "type": "linkup", "time": 660, "link": "AD" },
                { "type": "linkup", "time": 660, "link": "AE" },
                { "type": "linkup", "time": 660, "link": "AF" },
                { "type": "linkup", "time": 660, "link": "AG" },
                { "type": "linkup", "time": 660, "link": "AH" },
                { "type": "linkup", "time": 660, "link": "AI" },
                { "type": "linkup", "time": 660, "link": "BD" },
                { "type": "linkup", "time": 660, "link": "BE" },
                { "type": "linkup", "time": 660, "link": "BF" },
                { "type": "linkup", "time": 660, "link": "BG" },
                { "type": "linkup", "time": 660, "link": "BH" },
                { "type": "linkup", "time": 660, "link": "BI" },
                { "type": "linkup", "time": 660, "link": "CE" },
                { "type": "linkup", "time": 660, "link": "CF" },
                { "type": "linkup", "time": 660, "link": "CG" },
                { "type": "linkup", "time": 660, "link": "CH" },
                { "type": "linkup", "time": 660, "link": "CI" },
                { "type": "linkup", "time": 660, "link": "DF" },
                { "type": "linkup", "time": 660, "link": "DG" },
                { "type": "linkup", "time": 660, "link": "DH" },
                { "type": "linkup", "time": 660, "link": "DI" },
                { "type": "linkup", "time": 660, "link": "EG" },
                { "type": "linkup", "time": 660, "link": "EH" },
                { "type": "linkup", "time": 660, "link": "EI" },
                { "type": "linkup", "time": 660, "link": "FH" },
                { "type": "linkup", "time": 660, "link": "FI" },
                { "type": "linkup", "time": 660, "link": "GI" }
            ]
        }
    $$);

INSERT INTO game (name, game_date, gateway_address) VALUES
    ('Test game', '2024-06-15', '10.0.0.25'),
    ('Big game', '2024-09-21', '192.168.0.13');

INSERT INTO game_round (game_id, name, spec, server_start_time, api_password)
    SELECT game.id, '0. tutorial', round_spec_lib.spec, TIMESTAMPTZ '2024-06-15 16:00', 'Foo'
    FROM game, round_spec_lib
    WHERE game.name = 'Test game' AND round_spec_lib.name = 'Tutorial'
    
    UNION ALL

    SELECT game.id, '1. round', round_spec_lib.spec, '2024-06-15 16:30', 'WHEEE!'
    FROM game, round_spec_lib
    WHERE game.name = 'Test game' AND round_spec_lib.name = 'Circle round'
    
    UNION ALL

    SELECT game.id, '0. tutorial', round_spec_lib.spec, TIMESTAMPTZ '2024-09-21 15:00', 'Foo'
    FROM game, round_spec_lib
    WHERE game.name = 'Big game' AND round_spec_lib.name = 'Tutorial'
    
    UNION ALL

    SELECT game.id, '1. round', round_spec_lib.spec, '2024-09-21 16:00', 'WHEEE!'
    FROM game, round_spec_lib
    WHERE game.name = 'Big game' AND round_spec_lib.name = 'Circle round'
    
    UNION ALL

    SELECT game.id, '2. round', round_spec_lib.spec, '2024-09-21 17:00', 'Bar'
    FROM game, round_spec_lib
    WHERE game.name = 'Big game' AND round_spec_lib.name = 'Circle round'
    
    UNION ALL

    SELECT game.id, '3. round', round_spec_lib.spec, '2024-09-21 18:00', 'Baz'
    FROM game, round_spec_lib
    WHERE game.name = 'Big game' AND round_spec_lib.name = 'Circle round'
    ;

INSERT INTO game_round_team (game_round_id, team_ident, subteam_id)
    SELECT game_round.id, 'A', subteam.id
    FROM
        game JOIN game_round ON game_round.game_id = game.id,
        subteam
    WHERE game.name = 'Test game' AND game_round.name = '0. tutorial' AND subteam.name = 'A-Team'

    UNION ALL

    SELECT game_round.id, 'B', subteam.id
    FROM
        game JOIN game_round ON game_round.game_id = game.id,
        subteam
    WHERE game.name = 'Test game' AND game_round.name = '0. tutorial' AND subteam.name = 'B-Team'

    UNION ALL

    SELECT game_round.id, 'A', subteam.id
    FROM
        game JOIN game_round ON game_round.game_id = game.id,
        subteam
    WHERE game.name = 'Test game' AND game_round.name = '1. round' AND subteam.name IN ('A-Team', 'B-Team')
    ;
INSERT INTO game_round_team (game_round_id, team_ident, subteam_id)
    SELECT game_round.id, 'A', subteam.id
    FROM
        game JOIN game_round ON game_round.game_id = game.id,
        subteam
    WHERE game.name = 'Big game' AND game_round.name = '0. tutorial' AND subteam.name = 'A-Team'

    UNION ALL

    SELECT game_round.id, 'B', subteam.id
    FROM
        game JOIN game_round ON game_round.game_id = game.id,
        subteam
    WHERE game.name = 'Big game' AND game_round.name = '0. tutorial' AND subteam.name = 'B-Team'

    UNION ALL

    SELECT game_round.id, 'C', subteam.id
    FROM
        game JOIN game_round ON game_round.game_id = game.id,
        subteam
    WHERE game.name = 'Big game' AND game_round.name = '0. tutorial' AND subteam.name = 'Kokokel'

    UNION ALL

    SELECT game_round.id, 'D', subteam.id
    FROM
        game JOIN game_round ON game_round.game_id = game.id,
        subteam
    WHERE game.name = 'Big game' AND game_round.name = '0. tutorial' AND subteam.name = 'Kvík!'

    UNION ALL

    SELECT game_round.id, 'A', subteam.id
    FROM
        game JOIN game_round ON game_round.game_id = game.id,
        subteam
    WHERE game.name = 'Big game' AND game_round.name = '1. round' AND subteam.name IN ('A-Team', 'B-Team')

    UNION ALL

    SELECT game_round.id, 'B', subteam.id
    FROM
        game JOIN game_round ON game_round.game_id = game.id,
        subteam
    WHERE game.name = 'Big game' AND game_round.name = '1. round' AND subteam.name IN ('Kokokel', 'Kvík!')

    UNION ALL

    SELECT game_round.id, 'A', subteam.id
    FROM
        game JOIN game_round ON game_round.game_id = game.id,
        subteam
    WHERE game.name = 'Big game' AND game_round.name = '2. round' AND subteam.name IN ('A-Team', 'Kokokel')

    UNION ALL

    SELECT game_round.id, 'B', subteam.id
    FROM
        game JOIN game_round ON game_round.game_id = game.id,
        subteam
    WHERE game.name = 'Big game' AND game_round.name = '2. round' AND subteam.name IN ('B-Team', 'Kvík!')

    UNION ALL

    SELECT game_round.id, 'A', subteam.id
    FROM
        game JOIN game_round ON game_round.game_id = game.id,
        subteam
    WHERE game.name = 'Big game' AND game_round.name = '3. round' AND subteam.name IN ('A-Team', 'Kvík!')

    UNION ALL

    SELECT game_round.id, 'B', subteam.id
    FROM
        game JOIN game_round ON game_round.game_id = game.id,
        subteam
    WHERE game.name = 'Big game' AND game_round.name = '3. round' AND subteam.name IN ('B-Team', 'Kokokel')
    ;
