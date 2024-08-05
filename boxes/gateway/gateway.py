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
    MTU = 250

    def __init__(self, source_json):
        self._source_json = source_json
        self._validate()
        self._preprocess()

    def _validate(self):
        # TBA implement validation
        pass

    def _preprocess(self):
        router_defs = [(ord(router_id), json_minify(router_def)) for router_id, router_def in self._source_json["routers"].items()]
        self.network_router_defs = split_into_sized_chunks(router_defs, self.MTU, lambda x: len(x[1]) + 2)

        link_defs = [json_minify(link_def) for link_def in self._source_json["links"]]
        self.network_link_defs = split_into_sized_chunks(link_defs, self.MTU, lambda x: len(x) + 1)

        packet_defs = [(int(packet_id), json_minify(packet_def)) for packet_id, packet_def in self._source_json["packets"].items()]
        self.network_packet_defs = split_into_sized_chunks(packet_defs, self.MTU, lambda x: len(x[1]) + 2)

        event_defs = [json_minify(event_def) for event_def in self._source_json["events"]]
        self.network_event_defs = split_into_sized_chunks(event_defs, self.MTU, lambda x: len(x) + 1)

        self.round_items = len(router_defs) + len(link_defs) + len(packet_defs) + len(event_defs)

        hash = sha256()
        hash.update(json_minify(self._source_json).encode("utf-8"))
        self.hash = hash.digest()

    @property
    def duration(self):
        return self._source_json["duration"]

    @property
    def id(self):
        return self._source_json["roundId"]

class NetworkMessages(IntEnum):
    NODE_STATUS = 1
    ROUND_HEADER = 2
    ROUTER_DEFINITION = 3
    LINK_DEFINITION = 4
    PACKET_DEFINITION = 5
    EVENT_DEFINITION = 6
    PREPARE_GAME = 7,
    GAME_STATE = 8,
    PACKET_VISIT = 9

class NodeType(IntEnum):
    ROOT = 0
    BOX = 1
    REPEATER = 2


class GameState(IntEnum):
    NOT_RUNNING = 0
    PREPARATION = 1
    RUNNING = 2
    PAUSED = 3
    FINISHED = 4


@dataclass
class NodeStatus:
    node_type: str
    parent: str

    active_round_id: int
    active_round_hash: str
    round_download_progress: int

    game_state: str
    game_time: int
    router_id: str

    last_seen: float = 0

def handles(message_name):
    def decorator(func):
        func._message_type = message_name
        return func
    return decorator


class Network(Thread):
    def __init__(self, port, on_card_visit):
        super().__init__()
        self.daemon = True
        self._port = Serial(port, 921600, timeout=1)
        self._last_timestamp = (0, 0)
        self._boxes = {}
        self._write_lock = Lock()

        self.on_card_visit = on_card_visit
        self._port.read_all()

    def run(self):
        while True:
            raw_line = None
            try:
                raw_line = self._port.readline()
                if raw_line is None:
                    continue
                line = raw_line.decode("utf-8").strip()
            except Exception:
                print("Invalid data: ", raw_line)
                continue
            if not line:
                continue
            if line.startswith("L:"):
                print("LOG    ", line[2:])
            elif line.startswith("T:"):
                self._handle_timestamp(line[2:])
            elif line.startswith("D:"):
                self._handle_data(line[2:])
            else:
                print("Unknown message:", line)

    def prune_inactive_boxes(self):
        TIMEOUT = 5
        now = time.time()
        for box_id, status in list(self._boxes.items()):
            if now - status.last_seen > TIMEOUT:
                del self._boxes[box_id]

    def _handle_timestamp(self, timestamp_message):
        timestamp = int(timestamp_message)
        self._last_timestamp = (timestamp, int(time.time() * 1000))

    def _handle_data(self, data_message):
        try:
            data = base64.b64decode(data_message)
            author = data[0:6].hex(":")
            message_type = NetworkMessages(data[6])
            for method in dir(self):
                if hasattr(getattr(self, method), "_message_type"):
                    if getattr(self, method)._message_type == message_type:
                        getattr(self, method)(author, data[7:])
                        return
            print("Unknown message type:", message_type)
            print(author, data.hex())
        except Exception as e:
            print(e)

    @handles(NetworkMessages.NODE_STATUS)
    def _handle_box_status(self, box_id, data):
        try:
            router_id_num = data[47]
            if router_id_num == -1 or router_id_num == 255:
                router_id_str = "N/A"
            else:
                router_id_str = chr(router_id_num)
            status = NodeStatus(
                node_type=NodeType(data[0]),
                parent=data[1:7].hex(":"),
                active_round_id=int.from_bytes(data[7:11], "little"),
                active_round_hash=data[11:43].hex(),
                round_download_progress=data[43],
                game_state=GameState(data[44]).name,
                game_time=int.from_bytes(data[45:47], "little"),
                router_id=router_id_str,
                last_seen=time.time()
            )
            print("Box status: ", box_id, status)
            self._boxes[box_id] = status
        except Exception as e:
            print(e)

    @handles(NetworkMessages.PACKET_VISIT)
    def _handle_packet_visit(self, box_id, data):
        time = int.from_bytes(data[0:2], "little")
        physical_card_id = data[2:9].hex(":")
        team_id = data[9]
        seq_num = data[10]
        router_id = data[11]
        has_score = data[12]
        score = data[13]
        event = {
            "time": time,
            "card": f"{chr(team_id)}{seq_num:03d}",
            "router": chr(router_id),
            "score": score if has_score else None,
            "bearer": physical_card_id,
        }

        self.on_card_visit(box_id, event)
        print("Packet visit: ", box_id, event)

    def network_time(self):
        """
        Return the current network timestamp (ms)
        """
        return self._last_timestamp[0] + int(time.time() * 1000) - self._last_timestamp[1]

    def list_boxes(self):
        self.prune_inactive_boxes()
        return self._boxes

    def _send_round_definition(self, command, round_definition):
        with self._write_lock:
            round_header = bytes([NetworkMessages.ROUND_HEADER])
            round_header += round_definition.id.to_bytes(4, "little")
            round_header += round_definition.hash
            round_header += round_definition.duration.to_bytes(4, "little")
            self._port.write(command)
            self._port.write(base64.b64encode(round_header))
            self._port.write(b'\n')
            time.sleep(0.2)

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
                                print("Item def: ", item_def)
                                payload += int(item_id).to_bytes(1, "little") + item_def.encode("utf-8") + b'\00'
                        else:
                            for item_def in item_pack:
                                print("Item def: ", item_def)
                                payload += item_def.encode("utf-8") + b'\00'

                    initial_index = defs_seen
                    defs_seen += len(item_pack)

                    packet = bytes([message_type])
                    packet += round_definition.id.to_bytes(4, "little")
                    packet += round_definition.hash
                    packet += initial_index.to_bytes(4, "little")
                    packet += defs_seen.to_bytes(4, "little")
                    packet += sum([len(x) for x in item_packs]).to_bytes(4, "little")
                    packet += payload

                    self._port.write(command)
                    self._port.write(base64.b64encode(packet))
                    self._port.write(b'\n')
                    time.sleep(0.2) # Nasty hack, let FW process the data, don't care about flow control

    def broadcast_round_definition(self, round_definition):
        return self._send_round_definition("B:".encode("utf-8"), round_definition)

    def send_round_definition(self, recipient, round_definition):
        return self._send_round_definition(f"S:{recipient}".encode("utf-8"), round_definition)

    def send_message(self, recipient, message_id, payload=bytes([])):
        with self._write_lock:
            packet = bytes([message_id])
            packet += payload
            self._port.write(f"S:{recipient}".encode("utf-8"))
            self._port.write(base64.b64encode(packet))
            self._port.write(b'\n')
            time.sleep(0.2)
