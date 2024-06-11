DROP TABLE IF EXISTS subteam, round_spec_lib, game, game_round, game_round_team, game_round_event;
DROP DOMAIN IF EXISTS round_time, team_ident, router_ident;
DROP TYPE IF EXISTS event_source;


CREATE DOMAIN round_time AS SMALLINT
    CHECK (VALUE >= 0);

CREATE DOMAIN team_ident AS CHAR(1)
    CHECK (VALUE BETWEEN 'A' AND 'Z');

CREATE DOMAIN router_ident AS CHAR(1)
    CHECK (VALUE BETWEEN 'A' AND 'Z');

CREATE TYPE event_source AS ENUM (
    'online',
    'offline_box',
    'offline_card',
    'ui'
);


CREATE TABLE subteam (
    id BIGINT PRIMARY KEY GENERATED ALWAYS AS IDENTITY,
    name VARCHAR(50) NOT NULL
);

CREATE TABLE round_spec_lib (
    id BIGINT PRIMARY KEY GENERATED ALWAYS AS IDENTITY,
    name VARCHAR(50) NOT NULL,
    spec JSON NOT NULL,
    is_deleted BOOL NOT NULL DEFAULT FALSE
);
COMMENT ON TABLE round_spec_lib IS 'Library of specifications of game rounds. Only used to define specs ahead. For an actual record of a game round, the specs get copied, and are stored independently, so that any later change to round_spec_lib does not affect any already defined game rounds.';

CREATE TABLE game (
    id BIGINT PRIMARY KEY GENERATED ALWAYS AS IDENTITY,
    name VARCHAR(50) NOT NULL,
    game_date DATE NOT NULL,
    gateway_address INET
);

CREATE TABLE game_round (
    id BIGINT PRIMARY KEY GENERATED ALWAYS AS IDENTITY,
    game_id BIGINT NOT NULL REFERENCES game ON DELETE RESTRICT,
    name VARCHAR(50) NOT NULL,
    spec JSON NOT NULL,
    server_start_time TIMESTAMPTZ,
    api_ident SMALLINT CHECK (api_ident > 0) GENERATED ALWAYS AS IDENTITY,
    api_password VARCHAR(20)
);
COMMENT ON COLUMN game_round.server_start_time IS 'Time when the round is supposed to start, as specified to the gateway.';
COMMENT ON COLUMN game_round.api_ident IS 'Identifier of the round as exchanged through the REST API. Extra column besides id due to limited range supported by the gateway.';
COMMENT ON COLUMN game_round.api_password IS 'Password to provide with REST API requests';
CREATE INDEX ON game_round (game_id);
CREATE INDEX ON game_round (api_ident);

CREATE TABLE game_round_team (
    id BIGINT PRIMARY KEY GENERATED ALWAYS AS IDENTITY,
    game_round_id BIGINT NOT NULL REFERENCES game_round ON DELETE CASCADE,
    team_ident team_ident NOT NULL,
    subteam_id BIGINT NOT NULL REFERENCES subteam ON DELETE CASCADE,
    UNIQUE (game_round_id, team_ident, subteam_id)
);
COMMENT ON TABLE game_round_team IS 'Which subteam is assigned to which team in a given round.';

CREATE TABLE game_round_event (
    id BIGINT PRIMARY KEY GENERATED ALWAYS AS IDENTITY,
    game_round_id BIGINT NOT NULL REFERENCES game_round ON DELETE RESTRICT,
    event JSON NOT NULL,
    server_time TIMESTAMPTZ NOT NULL DEFAULT NOW(),
    source event_source NOT NULL,
    team_ident team_ident NOT NULL,
    router_ident router_ident,
    router_mac_address macaddr,
    round_time round_time,
    score INT
);
COMMENT ON COLUMN game_round_event.server_time IS 'Time the server recorded the event';
COMMENT ON COLUMN game_round_event.team_ident IS 'Team which triggered the event (e.g., to whom score should be counted)';
COMMENT ON COLUMN game_round_event.router_ident IS 'Router which emitted the event. The identifiers match the router definitions in game_round.spec.';
COMMENT ON COLUMN game_round_event.router_mac_address IS 'Physical address of the box representing the router which emitted the event. For diagnostic purposes only.';
COMMENT ON COLUMN game_round_event.round_time IS 'At what time of the round (relative to the round start) the event happened. NULL represents pre- or post-round events (e.g., check-in to routers before start, or ad hoc points awarded or special packet logic evaluation). Used for validating whether the event occurred while the game round was still running (vs. card beeps after the round ended).';
COMMENT ON COLUMN game_round_event.score IS 'How many points the team scored for this event';
CREATE UNIQUE INDEX ON game_round_event (game_round_id, round_time, (event->>'card'));
