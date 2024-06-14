from threading import Lock, Thread
import time
import traceback
from flask import Flask, request, jsonify, make_response
from .gateway import GameState, Network, NetworkMessages, RoundDefinition
import os

empty_round_def = {
    "duration": 0,
    "roundId": 0,
    "routers": {},
    "links": [],
    "packets": {},
    "events": []
}


app = Flask("rg_gateway")
app.config.from_envvar("GATEWAY_SETTINGS", silent=True)

app.network = Network(app.config["INTERFACE_PORT"])
app.network.start()

app.current_round = RoundDefinition(empty_round_def)
app.current_round_lock = Lock()
app.game_state = GameState.NOT_RUNNING
app.round_start_time = None
app.round_pause_time = None

def round_refresh_routine():
    while True:
        try:
            app.network.broadcast_round_definition(app.current_round)
        except Exception as e:
            print(traceback.format_exc())
            print(f"Error while broadcasting round definition: {e}")
        time.sleep(0.5)

def round_refresh_routine_targeted():
    global app
    while True:
        for id, box in app.network.list_boxes().items():
            try:
                with app.current_round_lock:
                    if box.active_round_id != app.current_round.id or box.active_round_hash != app.current_round.hash.hex() or box.round_download_progress < 100:
                        app.network.send_round_definition(id, app.current_round)
                game_time_offset = 0
                if app.game_state == GameState.RUNNING:
                    game_time_offset = app.round_start_time
                app.network.send_message(id, NetworkMessages.GAME_STATE, bytes([app.game_state]) + game_time_offset.to_bytes(4, "little"))
            except Exception as e:
                print(traceback.format_exc())
                print(f"Error while broadcasting round definition: {e}")
        time.sleep(1)

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
    with app.current_round_lock:
        new_round = RoundDefinition(request.json)
        app.current_round = new_round
        return {}

@app.route("/v1/game/prepare", methods=["POST"])
def prepare_game():
    for id in app.network.list_boxes().keys():
        app.game_state = GameState.PREPARATION
    return {}

@app.route("/v1/game/start", methods=["POST"])
def start():
    global app
    with app.current_round_lock:
        app.round_start_time = app.network.network_time()
        app.round_pause_time = None
        app.game_state = GameState.RUNNING
    return {}

@app.route("/v1/game/pause", methods=["POST"])
def pause_game():
    global app
    with app.current_round_lock:
        app.round_pause_time = app.network.network_time()
        app.game_state = GameState.PAUSED
    return {}

@app.route("/v1/game/resume", methods=["POST"])
def resume_game():
    global app
    with app.current_round_lock:
        app.round_start_time = app.network.network_time() - app.round_pause_time + app.round_start_time
        app.round_pause_time = None
        app.game_state = GameState.RUNNING
    return {}

@app.route("/v1/game/stop", methods=["POST"])
def stop_game():
    global app
    with app.current_round_lock:
        app.game_state = GameState.NOT_RUNNING
    return {}

@app.route("/v1/game/status")
def game_status():
    game_time = 0
    if app.game_state == GameState.RUNNING:
        game_time = app.network.network_time() - app.round_start_time
    elif app.game_state == GameState.PAUSED:
        game_time = app.round_pause_time - app.round_start_time

    return {
        "gateway": {
            "time": time.time(),
            "game_state": app.game_state.name,
            "game_time": int(game_time / 1000),
            "round": app.current_round.id,
            "round_hash": app.current_round.hash.hex(),
        },
        "boxes": app.network.list_boxes()
    }


if __name__ == "__main__":
    app.run(threaded=False)
