from threading import Thread
import time
from flask import Flask, request, jsonify, make_response
from .gateway import Network, RoundDefinition

empty_round_def = {
    "round_id": 0,
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

def round_refresh_routine():
    while True:
        try:
            app.network.broadcast_round_definition(app.current_round)
        except Exception as e:
            print(f"Error while broadcasting round definition: {e}")
        time.sleep(2)

Thread(target=round_refresh_routine, daemon=True).start()


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
