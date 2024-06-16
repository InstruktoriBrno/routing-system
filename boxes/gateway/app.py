import json
from threading import Lock, Thread
import time
import traceback
from flask import Flask, request, jsonify, make_response, g
from .gateway import GameState, Network, NetworkMessages, RoundDefinition
import os
import sqlite3
import requests
from requests.auth import HTTPBasicAuth

DATABASE = 'db.sqlite3'

empty_round_def = {
    "duration": 0,
    "roundId": 0,
    "routers": {},
    "links": [],
    "packets": {},
    "events": []
}

app = Flask("rg_gateway")

def get_db():
    db = getattr(g, '_database', None)
    if db is None:
        db = g._database = sqlite3.connect(DATABASE)
    return db

@app.teardown_appcontext
def close_connection(exception):
    db = getattr(g, '_database', None)
    if db is not None:
        db.close()

def init_db():
    if not os.path.exists(DATABASE):
        with app.app_context():
            db = get_db()
            cursor = db.cursor()
            cursor.execute('''CREATE TABLE IF NOT EXISTS game (
                              address TEXT,
                              id TEXT,
                              password TEXT,
                              game_state INTEGER,
                              round_start_time INTEGER,
                              round_pause_time INTEGER,
                              round_def TEXT)''')
            db.commit()
            # Insert a single row if the table is empty
            cursor.execute('SELECT COUNT(*) FROM game')
            if cursor.fetchone()[0] == 0:
                cursor.execute('''INSERT INTO game (address, id, password, game_state, round_start_time, round_pause_time, round_def)
                                  VALUES (?, ?, ?, ?, ?, ?, ?)''',
                               ('', '', '', 0, 0, 0, json.dumps(empty_round_def)))
                db.commit()

init_db()

def set_game_params(address, id, password, game_state, round_start_time, round_pause_time, round_def):
    with app.app_context():
        db = get_db()
        cursor = db.cursor()
        cursor.execute('''UPDATE game SET address = ?, id = ?, password = ?, game_state = ?, round_start_time = ?, round_pause_time = ?, round_def = ?
                          WHERE rowid = 1''', (address, id, password, game_state, round_start_time, round_pause_time, round_def))
        db.commit()
    return "Parameters set successfully!"

def get_game_params():
    with app.app_context():
        db = get_db()
        cursor = db.cursor()
        cursor.execute('SELECT * FROM game WHERE rowid = 1')
        row = cursor.fetchone()
        if row:
            return {
                "address": row[0],
                "id": row[1],
                "password": row[2],
                "game_state": row[3],
                "round_start_time": row[4],
                "round_pause_time": row[5],
                "round_def": row[6]
            }


def submit_card_visit_to_server(box_id, event):
    game = get_game_params()

    payload = {
        "routerMac": box_id,
        "source": "online",
        "events": [event]
    }

    url = "http://" + game["address"] + f"/v1/game/round/{game['id']}/router/{event['router']}"
    print("Submitting card visit to server:", payload, url)

    response = requests.post(
        url = url,
        json=payload)

    if response.status_code == 200:
        print('Request was successful!')
        print('Response:', response.json())
    else:
        print('Request failed with status code:', response.status_code)
        print('Response:', response.text)

app.config.from_envvar("GATEWAY_SETTINGS", silent=True)

app.network = Network(app.config["INTERFACE_PORT"], submit_card_visit_to_server)
app.network.start()

app.current_round = RoundDefinition(empty_round_def)
app.current_round_lock = Lock()
# app.game_state = GameState.NOT_RUNNING
# app.round_start_time = None
# app.round_pause_time = None

def round_refresh_routine():
    while True:
        try:
            app.network.broadcast_round_definition(app.current_round)
        except Exception as e:
            print(traceback.format_exc())
            print(f"Error while broadcasting round definition: {e}")
        time.sleep(0.5)

def round_refresh_routine_targeted():
    while True:
        game = get_game_params()
        app.current_round = RoundDefinition(json.loads(game["round_def"]))
        for id, box in list(app.network.list_boxes().items()):
            try:
                game_time_offset = 0
                if game["game_state"] == GameState.RUNNING:
                    game_time_offset = game["round_start_time"]
                app.network.send_message(id, NetworkMessages.GAME_STATE, bytes([game["game_state"]]) + game_time_offset.to_bytes(4, "little"))
            except Exception as e:
                print(traceback.format_exc())
                print(f"Error while broadcasting round definition: {e}")
        for id, box in list(app.network.list_boxes().items()):
            try:
                if box.active_round_id != app.current_round.id or box.active_round_hash != app.current_round.hash.hex() or box.round_download_progress < 100:
                    app.network.send_round_definition(id, app.current_round)
            except Exception as e:
                print(traceback.format_exc())
                print(f"Error while broadcasting round definition: {e}")
        time.sleep(0.5)

Thread(target=round_refresh_routine_targeted, daemon=True).start()


@app.route("/boxes")
@app.route("/boxes/as_json")
def boxes():
    return jsonify(app.network.list_boxes())

@app.route("/boxes/as_dot")
def boxes_as_dot():
    edges = set()
    nodes = {}

    for node, obj in app.network.list_boxes().items():
        parent = obj.parent
        if parent is not None:
            edges.add(tuple(sorted((node, parent))))

        node_text = f'<b>{node}</b>'
        for attr in dir(obj):
            if not attr.startswith('__') and not callable(getattr(obj, attr)) and attr != 'parent':
                node_text += f'<br align="left"/>{attr}: {getattr(obj, attr)}'


        nodes[node] = node_text

    dot_output = "graph G {\n"
    dot_output += '    node [shape=rectangle, style=filled, fillcolor=lightgrey, fontname="Helvetica"];\n'
    for node, text in nodes.items():
        dot_output += f'    "{node}" [label=<{text}>];\n'
    for edge in edges:
        dot_output += f'    "{edge[0]}" -- "{edge[1]}";\n'
    dot_output += "}\n"

    response = make_response(dot_output, 200)
    response.mimetype = "text/plain"
    return response


@app.route("/v1/game/round", methods=["PUT"])
def set_round():
    game = get_game_params()
    new_round = RoundDefinition(request.json)
    game["round_def"] = json.dumps(request.json)
    set_game_params(**game)
    return {}

@app.route("/v1/game/prepare", methods=["POST"])
def prepare_game():
    game = get_game_params()
    game["game_state"] = GameState.PREPARATION
    set_game_params(**game)
    return {}

@app.route("/v1/game/start", methods=["POST"])
def start():
    print("Starting game with params:", request.json)
    game = get_game_params()
    game["round_start_time"] = app.network.network_time()
    game["round_pause_time"] = None
    game["game_state"] = GameState.RUNNING
    game["id"] = request.json["roundId"]
    game["password"] = request.json["password"]
    game["address"] = request.remote_addr
    set_game_params(**game)
    return {}

@app.route("/v1/game/pause", methods=["POST"])
def pause_game():
    game = get_game_params()
    game["round_pause_time"] = app.network.network_time()
    game["game_state"] = GameState.PAUSED
    set_game_params(**game)
    return {}

@app.route("/v1/game/resume", methods=["POST"])
def resume_game():
    game = get_game_params()
    game["round_start_time"] = app.network.network_time() - game["round_pause_time"] + game["round_start_time"]
    game["round_pause_time"] = None
    game["game_state"] = GameState.RUNNING
    set_game_params(**game)
    return {}

@app.route("/v1/game/stop", methods=["POST"])
def stop_game():
    game = get_game_params()
    game["game_state"] = GameState.NOT_RUNNING
    set_game_params(**game)
    return {}

@app.route("/v1/status")
def game_status():
    game = get_game_params()
    game_time = 0
    if game["game_state"] == GameState.RUNNING:
        game_time = app.network.network_time() - game["round_start_time"]
    elif app.game_state == GameState.PAUSED:
        game_time = game["round_pause_time"] - game["round_start_time"]

    return {
        "gateway": {
            "time": time.time(),
            "game_state": GameState(game["game_state"]).name,
            "game_time": int(game_time / 1000),
            "round": app.current_round.id,
            "round_hash": app.current_round.hash.hex(),
        },
        "boxes": app.network.list_boxes()
    }


if __name__ == "__main__":
    app.run(threaded=False)
