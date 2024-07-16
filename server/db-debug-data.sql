TRUNCATE subteam, round_spec_lib, game, game_round, game_round_team, game_round_event;

-- ALTER SEQUENCE subteam_id_seq RESTART WITH 1;
-- ALTER SEQUENCE round_spec_lib_id_seq RESTART WITH 1;
-- ALTER SEQUENCE game_id_seq RESTART WITH 1;
-- ALTER SEQUENCE game_round_id_seq RESTART WITH 1;
-- ALTER SEQUENCE game_round_api_ident_seq RESTART WITH 1;
-- ALTER SEQUENCE game_round_team_id_seq RESTART WITH 1;
-- ALTER SEQUENCE game_round_event_id_seq RESTART WITH 1;

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
    $$),
    ('Test: Warmup', FALSE, $$
{
    "duration": 1200,
    "routers": {
        "A": {
            "mac": ["D8:BC:38:FD:85:BC"]
        },
        "B": {
            "mac": ["A8:03:2A:F4:51:F0"]
        },
        "C": {
            "mac": ["D8:BC:38:FD:BA:84"]
        },
        "D": {
            "mac": ["10:97:BD:40:B7:28"]
        },
        "E": {
            "mac": ["08:3A:F2:09:BE:98"]
        },
        "F": {
            "mac": ["E4:65:B8:76:7C:B4"]
        },
        "G": {
            "mac": ["EC:64:C9:85:7F:64"]
        },
        "H": {
            "mac": ["D8:BC:38:F9:33:1C"]
        },
        "I": {
            "mac": ["E4:65:B8:77:0A:A4"]
        },
        "J": {
            "mac": ["E4:65:B8:76:EC:E0"]
        },
        "K": {
            "mac": ["48:E7:29:A4:34:FC"]
        },
        "L": {
            "mac": ["E4:65:B8:76:7B:58"]
        },
        "M": {
            "mac": ["EC:64:C9:85:E6:AC"]
        },
        "N": {
            "mac": ["34:94:54:4E:59:7C"]
        },
        "O": {
            "mac": ["E4:65:B8:77:10:0C"]
        }
    },
    "links": [ "AB", "BC", "CD", "DE", "EF", "FG", "GA", "HI", "IJ", "JK", "KL", "LM", "MN", "NO", "OH"],
    "packets": {
        "000": {"type": "admin", "releaseTime":0, "source": "A"},
        "001": {"type": "standard", "releaseTime":0, "source": "A", "destination": "C", "points": 10},
        "002": {"type": "standard", "releaseTime":0, "source": "B", "destination": "D", "points": 10},
        "003": {"type": "standard", "releaseTime":0, "source": "C", "destination": "E", "points": 10},
        "005": {"type": "standard", "releaseTime":0, "source": "D", "destination": "F", "points": 10},
        "012": {"type": "standard", "releaseTime":0, "source": "E", "destination": "G", "points": 10},
        "010": {"type": "standard", "releaseTime":0, "source": "F", "destination": "A", "points": 10},
        "008": {"type": "standard", "releaseTime":0, "source": "G", "destination": "B", "points": 10},
        "006": {"type": "standard", "releaseTime":0, "source": "H", "destination": "J", "points": 10},
        "013": {"type": "standard", "releaseTime":0, "source": "I", "destination": "O", "points": 10},
        "004": {"type": "standard", "releaseTime":0, "source": "J", "destination": "L", "points": 10},
        "014": {"type": "standard", "releaseTime":0, "source": "K", "destination": "I", "points": 10},
        "015": {"type": "standard", "releaseTime":0, "source": "L", "destination": "N", "points": 10},
        "011": {"type": "standard", "releaseTime":0, "source": "M", "destination": "K", "points": 10},
        "009": {"type": "standard", "releaseTime":0, "source": "N", "destination": "H", "points": 10},
        "007": {"type": "standard", "releaseTime":0, "source": "O", "destination": "M", "points": 10}
    },
    "events": [
    ]
}
$$),
('Test: Round 1', FALSE, $$
{
    "duration": 900,
    "routers": {
        "L": {
            "mac": ["D8:BC:38:FD:85:BC"]
        },
        "A": {
            "mac": ["A8:03:2A:F4:51:F0"]
        },
        "D": {
            "mac": ["D8:BC:38:FD:BA:84"]
        },
        "C": {
            "mac": ["10:97:BD:40:B7:28"]
        },
        "J": {
            "mac": ["08:3A:F2:09:BE:98"]
        },
        "E": {
            "mac": ["E4:65:B8:76:7C:B4"]
        },
        "B": {
            "mac": ["EC:64:C9:85:7F:64"]
        },
        "M": {
            "mac": ["D8:BC:38:F9:33:1C"]
        },
        "O": {
            "mac": ["E4:65:B8:77:0A:A4"]
        },
        "N": {
            "mac": ["E4:65:B8:76:EC:E0"]
        },
        "F": {
            "mac": ["48:E7:29:A4:34:FC"]
        },
        "K": {
            "mac": ["E4:65:B8:76:7B:58"]
        },
        "G": {
            "mac": ["EC:64:C9:85:E6:AC"]
        },
        "I": {
            "mac": ["34:94:54:4E:59:7C"]
        },
        "H": {
            "mac": ["E4:65:B8:77:10:0C"]
        }
    },
    "links": [  "AD", "AL", "AM",
                "BE", "BI", "BL",
                "CD", "CJ", "CO",
                "DN",
                "EG", "EJ",
                "FJ", "FK", "FN",
                "GI", "GK",
                "HI", "HL", "HM",
                "JK",
                "MO",
                "NO"],
    "packets": {
        "000": {"type": "admin", "releaseTime":0, "source": "A"},

        "002": {"type": "standard", "releaseTime":0, "source": "A", "destination": "C", "points": 10},
        "008": {"type": "standard", "releaseTime":0, "source": "B", "destination": "D", "points": 10},
        "005": {"type": "standard", "releaseTime":0, "source": "C", "destination": "E", "points": 10},
        "003": {"type": "standard", "releaseTime":0, "source": "D", "destination": "F", "points": 10},
        "010": {"type": "standard", "releaseTime":0, "source": "E", "destination": "A", "points": 10},
        "014": {"type": "standard", "releaseTime":0, "source": "F", "destination": "A", "points": 10},
        "011": {"type": "standard", "releaseTime":0, "source": "G", "destination": "B", "points": 10},
        "007": {"type": "standard", "releaseTime":0, "source": "H", "destination": "O", "points": 10},
        "009": {"type": "standard", "releaseTime":0, "source": "I", "destination": "K", "points": 10},
        "012": {"type": "standard", "releaseTime":0, "source": "J", "destination": "N", "points": 10},
        "015": {"type": "standard", "releaseTime":0, "source": "K", "destination": "A", "points": 10},
        "001": {"type": "standard", "releaseTime":0, "source": "L", "destination": "I", "points": 10},
        "006": {"type": "standard", "releaseTime":0, "source": "M", "destination": "N", "points": 10},
        "004": {"type": "standard", "releaseTime":0, "source": "N", "destination": "M", "points": 10},
        "013": {"type": "standard", "releaseTime":0, "source": "O", "destination": "H", "points": 10},

        "016": {"type": "hopper", "releaseTime":120, "source": "F", "pointsPerHop": 1},
        "017": {"type": "hopper", "releaseTime":120, "source": "L", "pointsPerHop": 1},

        "021": {"type": "standard", "releaseTime":600, "source": "A", "destination": "J", "points": 10},
        "022": {"type": "standard", "releaseTime":600, "source": "B", "destination": "C", "points": 10},
        "023": {"type": "standard", "releaseTime":600, "source": "C", "destination": "I", "points": 10},
        "024": {"type": "standard", "releaseTime":600, "source": "D", "destination": "B", "points": 10},
        "025": {"type": "standard", "releaseTime":600, "source": "E", "destination": "A", "points": 10},
        "026": {"type": "standard", "releaseTime":600, "source": "F", "destination": "M", "points": 10},
        "027": {"type": "standard", "releaseTime":600, "source": "G", "destination": "L", "points": 10},
        "028": {"type": "standard", "releaseTime":600, "source": "H", "destination": "K", "points": 10},
        "029": {"type": "standard", "releaseTime":600, "source": "I", "destination": "O", "points": 10},
        "030": {"type": "standard", "releaseTime":600, "source": "J", "destination": "H", "points": 10},
        "031": {"type": "standard", "releaseTime":600, "source": "K", "destination": "D", "points": 10},
        "032": {"type": "standard", "releaseTime":600, "source": "L", "destination": "J", "points": 10},
        "033": {"type": "standard", "releaseTime":600, "source": "M", "destination": "G", "points": 10},
        "034": {"type": "standard", "releaseTime":600, "source": "N", "destination": "B", "points": 10},
        "035": {"type": "standard", "releaseTime":600, "source": "O", "destination": "H", "points": 10},
 
        "041": {"type": "standard", "releaseTime":900, "source": "A", "destination": "E", "points": 10},
        "042": {"type": "standard", "releaseTime":900, "source": "B", "destination": "N", "points": 10},
        "044": {"type": "standard", "releaseTime":900, "source": "D", "destination": "H", "points": 10},
        "045": {"type": "standard", "releaseTime":900, "source": "E", "destination": "M", "points": 10},
        "046": {"type": "standard", "releaseTime":900, "source": "F", "destination": "L", "points": 10},
        "048": {"type": "standard", "releaseTime":900, "source": "H", "destination": "N", "points": 10},
        "049": {"type": "standard", "releaseTime":900, "source": "I", "destination": "F", "points": 10},
        "050": {"type": "standard", "releaseTime":900, "source": "J", "destination": "H", "points": 10},
        "051": {"type": "standard", "releaseTime":900, "source": "K", "destination": "A", "points": 10},
        "052": {"type": "standard", "releaseTime":900, "source": "L", "destination": "F", "points": 10},
        "053": {"type": "standard", "releaseTime":900, "source": "M", "destination": "K", "points": 10},
        "054": {"type": "standard", "releaseTime":900, "source": "N", "destination": "E", "points": 10},
        "055": {"type": "standard", "releaseTime":900, "source": "O", "destination": "G", "points": 10},
        
        "056": {"type": "priority", "releaseTime":900, "source": "C", "destination": "I", "pointsPerMinuteLeft": 4, "minutesToDeliver": 5},
        "057": {"type": "priority", "releaseTime":900, "source": "G", "destination": "A", "pointsPerMinuteLeft": 4, "minutesToDeliver": 5}

    },
    "events": [
    ]
}
$$),
('Test: Round 2', FALSE, $$
{
    "duration": 900,
    "routers": {
        "E": {
            "mac": ["D8:BC:38:FD:85:BC"]
        },
        "H": {
            "mac": ["A8:03:2A:F4:51:F0"]
        },
        "G": {
            "mac": ["D8:BC:38:FD:BA:84"]
        },
        "I": {
            "mac": ["10:97:BD:40:B7:28"]
        },
        "M": {
            "mac": ["08:3A:F2:09:BE:98"]
        },
        "A": {
            "mac": ["E4:65:B8:76:7C:B4"]
        },
        "D": {
            "mac": ["EC:64:C9:85:7F:64"]
        },
        "F": {
            "mac": ["D8:BC:38:F9:33:1C"]
        },
        "K": {
            "mac": ["E4:65:B8:77:0A:A4"]
        },
        "J": {
            "mac": ["E4:65:B8:76:EC:E0"]
        },
        "L": {
            "mac": ["48:E7:29:A4:34:FC"]
        },
        "N": {
            "mac": ["E4:65:B8:76:7B:58"]
        },
        "O": {
            "mac": ["EC:64:C9:85:E6:AC"]
        },
        "B": {
            "mac": ["34:94:54:4E:59:7C"]
        },
        "C": {
            "mac": ["E4:65:B8:77:10:0C"]
        }
    },
    "links": [  
        "AB", "AD", "AM", "AN", "AO",
        "BC", "BD", "BO",
        "CD", "DE", "EF",
        "DE",
        "EF", "EH",
        "FG", "FH", "FI", "FK",
        "GH", "GI", "GJ",
        "HI",
        "IJ", "IK", "IM",
        "JK", "JL",
        "KL",
        "LM", "LN",
        "MN", "MO",
        "NO"
    ],
    "packets": {
        "000": {"type": "admin", "releaseTime":0, "source": "A"},
        "010": {"type": "standard", "releaseTime":0, "source": "A", "destination": "C", "points": 10},
        "009": {"type": "standard", "releaseTime":0, "source": "B", "destination": "D", "points": 10},
        "007": {"type": "standard", "releaseTime":0, "source": "C", "destination": "E", "points": 10},
        "008": {"type": "standard", "releaseTime":0, "source": "D", "destination": "F", "points": 10},
        "001": {"type": "standard", "releaseTime":0, "source": "E", "destination": "A", "points": 10},
        "006": {"type": "standard", "releaseTime":0, "source": "F", "destination": "A", "points": 10},
        "003": {"type": "standard", "releaseTime":0, "source": "G", "destination": "B", "points": 10},
        "002": {"type": "standard", "releaseTime":0, "source": "H", "destination": "O", "points": 10},
        "005": {"type": "standard", "releaseTime":0, "source": "I", "destination": "K", "points": 10},
        "004": {"type": "standard", "releaseTime":0, "source": "J", "destination": "N", "points": 10},
        "013": {"type": "standard", "releaseTime":0, "source": "K", "destination": "A", "points": 10},
        "014": {"type": "standard", "releaseTime":0, "source": "L", "destination": "I", "points": 10},
        "012": {"type": "standard", "releaseTime":0, "source": "M", "destination": "N", "points": 10},
        "015": {"type": "standard", "releaseTime":0, "source": "N", "destination": "M", "points": 10},
        "011": {"type": "standard", "releaseTime":0, "source": "O", "destination": "H", "points": 10},
        
        "016": {"type": "visitall", "releaseTime":180, "source": "H", "points": 50},
        "017": {"type": "visitall", "releaseTime":180, "source": "L", "points": 50},
        "018": {"type": "priority", "releaseTime":180, "source": "O", "destination": "G", "pointsPerMinuteLeft": 4, "minutesToDeliver": 5},

        "019": {"type": "priority", "releaseTime":300, "source": "C", "destination": "H", "pointsPerMinuteLeft": 4, "minutesToDeliver": 5},
        "020": {"type": "priority", "releaseTime":300, "source": "C", "destination": "I", "pointsPerMinuteLeft": 4, "minutesToDeliver": 5},
        "021": {"type": "priority", "releaseTime":300, "source": "C", "destination": "N", "pointsPerMinuteLeft": 4, "minutesToDeliver": 5},

        "022": {"type": "standard", "releaseTime":300, "source": "B", "destination": "L", "points": 10},
        "023": {"type": "standard", "releaseTime":300, "source": "B", "destination": "G", "points": 10},
        "024": {"type": "standard", "releaseTime":300, "source": "B", "destination": "M", "points": 10},

        "025": {"type": "priority", "releaseTime":300, "source": "K", "destination": "M", "pointsPerMinuteLeft": 4, "minutesToDeliver": 5},
        "026": {"type": "priority", "releaseTime":300, "source": "K", "destination": "D", "pointsPerMinuteLeft": 4, "minutesToDeliver": 5},
        "027": {"type": "priority", "releaseTime":300, "source": "M", "destination": "E", "pointsPerMinuteLeft": 4, "minutesToDeliver": 5},
        "028": {"type": "priority", "releaseTime":300, "source": "M", "destination": "K", "pointsPerMinuteLeft": 4, "minutesToDeliver": 5},

        "061": {"type": "standard", "releaseTime":600, "source": "O", "destination": "K", "points": 10},
        "062": {"type": "standard", "releaseTime":600, "source": "O", "destination": "E", "points": 10},
        "063": {"type": "standard", "releaseTime":600, "source": "O", "destination": "F", "points": 10},
        "064": {"type": "standard", "releaseTime":600, "source": "H", "destination": "C", "points": 10},
        "065": {"type": "standard", "releaseTime":600, "source": "H", "destination": "A", "points": 10},
        "066": {"type": "standard", "releaseTime":600, "source": "H", "destination": "J", "points": 10},
        "067": {"type": "standard", "releaseTime":600, "source": "G", "destination": "D", "points": 10},
        "068": {"type": "standard", "releaseTime":600, "source": "G", "destination": "N", "points": 10},
        "069": {"type": "standard", "releaseTime":600, "source": "G", "destination": "B", "points": 10},

        "071": {"type": "hopper", "releaseTime":660, "source": "A", "pointsPerHop": 1},
        "072": {"type": "hopper", "releaseTime":660, "source": "F", "pointsPerHop": 1},
        "073": {"type": "hopper", "releaseTime":660, "source": "L", "pointsPerHop": 1},
        "074": {"type": "visitall", "releaseTime":660, "source": "A", "points": 50},
        "075": {"type": "visitall", "releaseTime":660, "source": "F", "points": 50},
        "076": {"type": "visitall", "releaseTime":660, "source": "L", "points": 50},

        "091": {"type": "standard", "releaseTime":900, "source": "A", "destination": "J", "points": 10},
        "092": {"type": "standard", "releaseTime":900, "source": "B", "destination": "C", "points": 10},
        "093": {"type": "standard", "releaseTime":900, "source": "C", "destination": "I", "points": 10},
        "094": {"type": "standard", "releaseTime":900, "source": "D", "destination": "B", "points": 10},
        "095": {"type": "standard", "releaseTime":900, "source": "E", "destination": "A", "points": 10},
        "096": {"type": "standard", "releaseTime":900, "source": "F", "destination": "M", "points": 10},
        "097": {"type": "standard", "releaseTime":900, "source": "G", "destination": "L", "points": 10},
        "098": {"type": "standard", "releaseTime":900, "source": "H", "destination": "K", "points": 10},
        "099": {"type": "standard", "releaseTime":900, "source": "I", "destination": "O", "points": 10},
        "100": {"type": "standard", "releaseTime":900, "source": "J", "destination": "H", "points": 10},
        "101": {"type": "standard", "releaseTime":900, "source": "K", "destination": "D", "points": 10},
        "102": {"type": "standard", "releaseTime":900, "source": "L", "destination": "J", "points": 10},
        "103": {"type": "standard", "releaseTime":900, "source": "M", "destination": "G", "points": 10},
        "104": {"type": "standard", "releaseTime":900, "source": "N", "destination": "B", "points": 10},
        "105": {"type": "standard", "releaseTime":900, "source": "O", "destination": "H", "points": 10},

        "106": {"type": "visitall", "releaseTime":960, "source": "O", "points": 50},
        "107": {"type": "visitall", "releaseTime":960, "source": "J", "points": 50},
        "108": {"type": "visitall", "releaseTime":960, "source": "C", "points": 50}
    },
    "events": [
    ]
}
$$),
('Test: Round 3', FALSE,
$$
{
    "duration": 900,
    "routers": {
        "D": {
            "mac": ["D8:BC:38:FD:85:BC"]
        },
        "G": {
            "mac": ["A8:03:2A:F4:51:F0"]
        },
        "L": {
            "mac": ["D8:BC:38:FD:BA:84"]
        },
        "M": {
            "mac": ["10:97:BD:40:B7:28"]
        },
        "B": {
            "mac": ["08:3A:F2:09:BE:98"]
        },
        "H": {
            "mac": ["E4:65:B8:76:7C:B4"]
        },
        "C": {
            "mac": ["EC:64:C9:85:7F:64"]
        },
        "J": {
            "mac": ["D8:BC:38:F9:33:1C"]
        },
        "O": {
            "mac": ["E4:65:B8:77:0A:A4"]
        },
        "E": {
            "mac": ["E4:65:B8:76:EC:E0"]
        },
        "A": {
            "mac": ["48:E7:29:A4:34:FC"]
        },
        "N": {
            "mac": ["E4:65:B8:76:7B:58"]
        },
        "F": {
            "mac": ["EC:64:C9:85:E6:AC"]
        },
        "K": {
            "mac": ["34:94:54:4E:59:7C"]
        },
        "I": {
            "mac": ["E4:65:B8:77:10:0C"]
        }
    },
    "links": [ 
        "AG", "AJ", "AL", "AM",
        "BH", "BM", "BN", "BO",
        "CD", "CH", "CI", "CK",
        "DG", "DI", "DJ",
        "EL", "EM", "EO",
        "FH", "FK", "FN",
        "GJ", "GL",
        "HK", "HN",
        "IK", "IJ",
        "JO",
        "LM", "LO"
    ],
    "packets": {
"000": {"type": "admin", "releaseTime":0, "source": "A"},
"014": {"type": "standard", "releaseTime":0, "source": "A", "destination": "E", "points": 10},
"012": {"type": "standard", "releaseTime":0, "source": "B", "destination": "F", "points": 10},
"008": {"type": "standard", "releaseTime":0, "source": "C", "destination": "G", "points": 10},
"001": {"type": "standard", "releaseTime":0, "source": "D", "destination": "H", "points": 10},
"004": {"type": "standard", "releaseTime":0, "source": "E", "destination": "I", "points": 10},
"011": {"type": "standard", "releaseTime":0, "source": "F", "destination": "J", "points": 10},
"002": {"type": "standard", "releaseTime":0, "source": "G", "destination": "K", "points": 10},
"010": {"type": "standard", "releaseTime":0, "source": "H", "destination": "L", "points": 10},
"007": {"type": "standard", "releaseTime":0, "source": "I", "destination": "M", "points": 10},
"006": {"type": "standard", "releaseTime":0, "source": "J", "destination": "N", "points": 10},
"009": {"type": "standard", "releaseTime":0, "source": "K", "destination": "O", "points": 10},
"003": {"type": "standard", "releaseTime":0, "source": "L", "destination": "A", "points": 10},
"005": {"type": "standard", "releaseTime":0, "source": "M", "destination": "B", "points": 10},
"015": {"type": "standard", "releaseTime":0, "source": "N", "destination": "C", "points": 10},
"013": {"type": "standard", "releaseTime":0, "source": "O", "destination": "D", "points": 10},

"020": {"type": "hopper", "releaseTime":300, "source": "A", "pointsPerHop": 1},
"022": {"type": "hopper", "releaseTime":300, "source": "C", "pointsPerHop": 1},
"024": {"type": "hopper", "releaseTime":300, "source": "B", "pointsPerHop": 1},
"021": {"type": "standard", "releaseTime":300, "source": "A", "destination": "C", "points": 10},
"023": {"type": "standard", "releaseTime":300, "source": "C", "destination": "B", "points": 10},
"025": {"type": "standard", "releaseTime":300, "source": "B", "destination": "A", "points": 10},

"026": {"type": "visitall", "releaseTime":360, "source": "D", "points": 50},
"028": {"type": "visitall", "releaseTime":360, "source": "H", "points": 50},
"030": {"type": "visitall", "releaseTime":360, "source": "O", "points": 50},
"027": {"type": "standard", "releaseTime":360, "source": "D", "destination": "O", "points": 10},
"029": {"type": "standard", "releaseTime":360, "source": "H", "destination": "H", "points": 10},
"031": {"type": "standard", "releaseTime":360, "source": "O", "destination": "D", "points": 10},

"036": {"type": "visitall", "releaseTime":420, "source": "J", "points": 50},
"038": {"type": "visitall", "releaseTime":420, "source": "K", "points": 50},
"040": {"type": "visitall", "releaseTime":420, "source": "M", "points": 50},
"037": {"type": "standard", "releaseTime":420, "source": "J", "destination": "K", "points": 10},
"039": {"type": "standard", "releaseTime":420, "source": "K", "destination": "M", "points": 10},
"041": {"type": "standard", "releaseTime":420, "source": "M", "destination": "J", "points": 10},

"042": {"type": "hopper", "releaseTime":480, "source": "N", "pointsPerHop": 1},
"044": {"type": "hopper", "releaseTime":480, "source": "I", "pointsPerHop": 1},
"046": {"type": "hopper", "releaseTime":480, "source": "L", "pointsPerHop": 1},
"043": {"type": "standard", "releaseTime":480, "source": "N", "destination": "L", "points": 10},
"045": {"type": "standard", "releaseTime":480, "source": "I", "destination": "N", "points": 10},
"047": {"type": "standard", "releaseTime":480, "source": "L", "destination": "I", "points": 10},

"061": {"type": "standard", "releaseTime":600, "source": "A", "destination": "F", "points": 10},
"062": {"type": "standard", "releaseTime":600, "source": "B", "destination": "G", "points": 10},
"063": {"type": "standard", "releaseTime":600, "source": "C", "destination": "H", "points": 10},
"064": {"type": "standard", "releaseTime":600, "source": "D", "destination": "I", "points": 10},
"065": {"type": "standard", "releaseTime":600, "source": "E", "destination": "J", "points": 10},
"066": {"type": "standard", "releaseTime":600, "source": "F", "destination": "K", "points": 10},
"067": {"type": "standard", "releaseTime":600, "source": "G", "destination": "L", "points": 10},
"068": {"type": "standard", "releaseTime":600, "source": "H", "destination": "M", "points": 10},
"069": {"type": "standard", "releaseTime":600, "source": "I", "destination": "N", "points": 10},
"070": {"type": "standard", "releaseTime":600, "source": "J", "destination": "O", "points": 10},
"071": {"type": "standard", "releaseTime":600, "source": "K", "destination": "A", "points": 10},
"072": {"type": "standard", "releaseTime":600, "source": "L", "destination": "B", "points": 10},
"073": {"type": "standard", "releaseTime":600, "source": "M", "destination": "C", "points": 10},
"074": {"type": "standard", "releaseTime":600, "source": "N", "destination": "D", "points": 10},
"075": {"type": "standard", "releaseTime":600, "source": "O", "destination": "E", "points": 10},

"076": {"type": "visitall", "releaseTime":660, "source": "E", "points": 50},
"078": {"type": "visitall", "releaseTime":660, "source": "F", "points": 50},
"080": {"type": "visitall", "releaseTime":660, "source": "G", "points": 50},
"077": {"type": "standard", "releaseTime":660, "source": "E", "destination": "F", "points": 10},
"079": {"type": "standard", "releaseTime":660, "source": "F", "destination": "G", "points": 10},
"081": {"type": "standard", "releaseTime":660, "source": "G", "destination": "E", "points": 10},

"091": {"type": "priority", "releaseTime":900, "source": "A", "destination": "G", "pointsPerMinuteLeft": 4, "minutesToDeliver": 5},
"092": {"type": "priority", "releaseTime":900, "source": "B", "destination": "G", "pointsPerMinuteLeft": 4, "minutesToDeliver": 5},
"093": {"type": "priority", "releaseTime":900, "source": "C", "destination": "G", "pointsPerMinuteLeft": 4, "minutesToDeliver": 5},
"094": {"type": "priority", "releaseTime":900, "source": "D", "destination": "G", "pointsPerMinuteLeft": 4, "minutesToDeliver": 5},
"095": {"type": "priority", "releaseTime":900, "source": "E", "destination": "G", "pointsPerMinuteLeft": 4, "minutesToDeliver": 5},
"096": {"type": "priority", "releaseTime":900, "source": "F", "destination": "G", "pointsPerMinuteLeft": 4, "minutesToDeliver": 5},
"097": {"type": "priority", "releaseTime":900, "source": "G", "destination": "G", "pointsPerMinuteLeft": 4, "minutesToDeliver": 5},
"098": {"type": "priority", "releaseTime":900, "source": "H", "destination": "G", "pointsPerMinuteLeft": 4, "minutesToDeliver": 5},
"099": {"type": "priority", "releaseTime":900, "source": "I", "destination": "G", "pointsPerMinuteLeft": 4, "minutesToDeliver": 5},
"100": {"type": "priority", "releaseTime":900, "source": "J", "destination": "G", "pointsPerMinuteLeft": 4, "minutesToDeliver": 5},
"101": {"type": "priority", "releaseTime":900, "source": "K", "destination": "G", "pointsPerMinuteLeft": 4, "minutesToDeliver": 5},
"102": {"type": "priority", "releaseTime":900, "source": "L", "destination": "G", "pointsPerMinuteLeft": 4, "minutesToDeliver": 5},
"103": {"type": "priority", "releaseTime":900, "source": "M", "destination": "G", "pointsPerMinuteLeft": 4, "minutesToDeliver": 5},
"104": {"type": "priority", "releaseTime":900, "source": "N", "destination": "G", "pointsPerMinuteLeft": 4, "minutesToDeliver": 5},
"105": {"type": "priority", "releaseTime":900, "source": "O", "destination": "G", "pointsPerMinuteLeft": 4, "minutesToDeliver": 5},

"106": {"type": "standard", "releaseTime":960, "source": "C", "destination": "H", "points": 10},
"107": {"type": "standard", "releaseTime":960, "source": "D", "destination": "K", "points": 10},
"108": {"type": "standard", "releaseTime":960, "source": "J", "destination": "F", "points": 10},
"109": {"type": "standard", "releaseTime":960, "source": "N", "destination": "C", "points": 10},
"110": {"type": "standard", "releaseTime":960, "source": "F", "destination": "I", "points": 10},
"111": {"type": "standard", "releaseTime":960, "source": "H", "destination": "I", "points": 10},

"112": {"type": "visitall", "releaseTime":990, "source": "M", "points": 50},
"113": {"type": "visitall", "releaseTime":990, "source": "E", "points": 50},
"114": {"type": "visitall", "releaseTime":990, "source": "A", "points": 50}

    },
    "events": [
    ]
}
$$);

INSERT INTO game (name, game_date, gateway_address) VALUES
    ('Debug game', '2024-06-15', NULL);

INSERT INTO game (name, game_date, gateway_address) VALUES
    ('Test game', '2024-06-15', NULL);

INSERT INTO game_round (game_id, name, spec, api_password)
    SELECT game.id, round_spec_lib.name, round_spec_lib.spec, 'Baz'
    FROM game, round_spec_lib
    WHERE game.name = 'Debug game' AND round_spec_lib.name IN (
        'Test: Warmup', 'Test: Round 1', 'Test: Round 2', 'Test: Round 3'
    )
    ;

INSERT INTO game_round (game_id, name, spec, api_password)
    SELECT game.id, 'Debug round', round_spec_lib.spec, 'Baz'
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
