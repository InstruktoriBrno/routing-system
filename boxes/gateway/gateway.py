import base64
from dataclasses import dataclass
from threading import Thread, Lock
from serial import Serial
from enum import IntEnum
from hashlib import sha256
import json
import time

def split_into_sized_chunks(source, limit, size_fn):
    result = []
    current = []
    current_size = 0
    for item in source:
        item_size = size_fn(item)
        if current_size + item_size > limit:
            result.append(current)
            current = []
            current_size = 0
        current.append(item)
        current_size += item_size
    result.append(current)
    return result

def json_minify(source):
    return json.dumps(source, separators=(',', ':'))

class RoundDefinition:
    MTU = 1400

    def __init__(self, source_json):
        self._source_json = source_json
        self._validate()
        self._preprocess()

    def _validate(self):
        # TBA implement validation
        pass

    def _preprocess(self):
        router_defs = [(router_id, json_minify(router_def)) for router_id, router_def in self._source_json["routers"].items()]
        self.network_router_defs = split_into_sized_chunks(router_defs, self.MTU, lambda x: len(x[1]) + 2)

        link_defs = [json_minify(link_def) for link_def in self._source_json["links"]]
        self.network_link_defs = split_into_sized_chunks(link_defs, self.MTU, lambda x: len(x) + 1)

        packet_defs = [(packet_id, json_minify(packet_def)) for packet_id, packet_def in self._source_json["packets"].items()]
        self.network_packet_defs = split_into_sized_chunks(packet_defs, self.MTU, lambda x: len(x[1]) + 2)

        event_defs = [json_minify(event_def) for event_def in self._source_json["events"]]
        self.network_event_defs = split_into_sized_chunks(event_defs, self.MTU, lambda x: len(x) + 1)

        self.round_items = len(router_defs) + len(link_defs) + len(packet_defs) + len(event_defs)

        hash = sha256()
        hash.update(json_minify(self._source_json).encode("utf-8"))
        self.hash = hash.digest()
        self.id = self._source_json["round_id"]

class NetworkMessages(IntEnum):
    NODE_STATUS = 1
    ROUND_HEADER = 2
    ROUTER_DEFINITION = 3
    LINK_DEFINITION = 4
    PACKET_DEFINITION = 5
    EVENT_DEFINITION = 6

class NodeType(IntEnum):
    ROOT = 0
    BOX = 1
    REPEATER = 2

@dataclass
class NodeStatus:
    node_type: str
    parent: str

    active_round_id: int
    active_round_hash: str
    round_download_progress: int

    last_seen: float = 0

def handles(message_name):
    def decorator(func):
        func._message_type = message_name
        return func
    return decorator


class Network(Thread):
    def __init__(self, port):
        super().__init__()
        self.daemon = True
        self._port = Serial(port, 921600, timeout=0.2)
        self._last_timestamp = (0, 0)
        self._boxes = {}
        self._write_lock = Lock()

        self.on_card_visit = None

    def run(self):
        while True:
            raw_line = None
            try:
                raw_line = self._port.readline()
                line = raw_line.decode("utf-8").strip()
            except Exception:
                print("Invalid data: ", raw_line)
                continue
            if not line:
                continue
            if line.startswith("L:"):
                print(line)
            elif line.startswith("T:"):
                self._handle_timestamp(line[2:])
            elif line.startswith("D:"):
                self._handle_data(line[2:])
            else:
                print("Unknown message:", line)

    def prune_inactive_boxes(self):
        TIMEOUT = 1
        now = time.time()
        for box_id, status in list(self._boxes.items()):
            if now - status.last_seen > TIMEOUT:
                del self._boxes[box_id]

    def _handle_timestamp(self, timestamp_message):
        timestamp = int(timestamp_message)
        self._last_timestamp = (timestamp, int(time.time() * 1000))

    def _handle_data(self, data_message):
        data = base64.b64decode(data_message)
        author = data[0:6].hex(":")
        message_type = NetworkMessages(data[7])
        for method in dir(self):
            if hasattr(getattr(self, method), "_message_type"):
                if getattr(self, method)._message_type == message_type:
                    getattr(self, method)(author, data[7:])
                    return
        raise ValueError("Unknown message type:", message_type)

    @handles(NetworkMessages.NODE_STATUS)
    def _handle_box_status(self, box_id, data):
        status = NodeStatus(
            node_type=NodeType(data[0]),
            parent=data[1:7].hex(":"),
            active_round_id=int.from_bytes(data[7:11], "little"),
            active_round_hash=data[11:43].hex(),
            round_download_progress=int.from_bytes(data[43:47], "little"),
            last_seen=time.time()
        )
        self._boxes[box_id] = status

    def network_time(self):
        """
        Return the current network timestamp (ms)
        """
        return self._last_timestamp[0] + int(time.time() * 1000) - self._last_timestamp[1]

    def list_boxes(self):
        self.prune_inactive_boxes()
        return self._boxes

    def broadcast_round_definition(self, round_definition):
        with self._write_lock:
            round_header = bytes([NetworkMessages.ROUND_HEADER])
            round_header += round_definition.id.to_bytes(4, "little")
            round_header += round_definition.hash
            round_header += round_definition.round_items.to_bytes(4, "little")
            print("About to send: ", base64.b64encode(round_header))
            self._port.write("B:".encode("utf-8"))
            self._port.write(base64.b64encode(round_header))
            self._port.write(b'\n')

            messages = {
                NetworkMessages.ROUTER_DEFINITION: round_definition.network_router_defs,
                NetworkMessages.LINK_DEFINITION: round_definition.network_link_defs,
                NetworkMessages.PACKET_DEFINITION: round_definition.network_packet_defs,
                NetworkMessages.EVENT_DEFINITION: round_definition.network_event_defs
            }

            for message_type, item_packs in messages.items():
                defs_seen = 0
                for item_pack in item_packs:
                    payload = bytes()
                    if len(item_pack) > 0:
                        if isinstance(item_pack[0], tuple):
                            for item_id, item_def in item_pack:
                                payload += item_id.to_bytes(1) + item_def.encode("utf-8") + b'\00'
                        else:
                            for item_def in item_pack:
                                payload += item_def.encode("utf-8") + b'\00'

                    initial_index = defs_seen
                    defs_seen += len(item_pack)

                    packet = bytes([message_type])
                    packet += round_definition.id.to_bytes(4, "little")
                    packet += round_definition.hash
                    packet += initial_index.to_bytes(4, "little")
                    packet += defs_seen.to_bytes(4, "little")
                    packet += payload

                    self._port.write("B:".encode("utf-8"))
                    self._port.write(base64.b64encode(packet))
                    self._port.write(b'\n')
                    time.sleep(0.05) # Nasty hack, let FW process the data, don't care about flow control
