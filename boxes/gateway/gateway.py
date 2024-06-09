from dataclasses import dataclass
from threading import Thread
from serial import Serial
import time

@dataclass
class BoxStatus:
    parent: str
    network_depth: int
    active_round_id: int
    round_download_progress: int
    last_seen: int = 0

    @staticmethod
    def from_message(fields):
        return BoxStatus(
            parent=fields[0],
            network_depth=int(fields[1]),
            active_round_id=int(fields[2]),
            round_download_progress=int(fields[3])
        )

def handles(message_name):
    def decorator(func):
        func._message_name = message_name
        return func
    return decorator

class Network(Thread):
    def __init__(self, port):
        super().__init__()
        self.daemon = True
        self._port = Serial(port, 921600, timeout=0.2)
        self._last_timestamp = (0, 0)
        self._boxes = {}

        self.on_card_visit = None

    def run(self):
        while True:
            line = self._port.readline().decode("utf-8").strip()
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
        fields = [x.strip() for x in data_message.split(",")]
        message_name = fields[0]
        for method in dir(self):
            if hasattr(getattr(self, method), "_message_name"):
                if getattr(self, method)._message_name == message_name:
                    getattr(self, method)(fields[1:])
                    return
        raise ValueError("Unknown message type:", message_name)

    @handles("BoxStatus")
    def _handle_box_status(self, fields):
        box_id = fields[0]
        status = BoxStatus.from_message(fields[1:])
        status.last_seen = time.time()
        self._boxes[box_id] = status

    def network_time(self):
        """
        Return the current network timestamp (ms)
        """
        return self._last_timestamp[0] + int(time.time() * 1000) - self._last_timestamp[1]

    def list_boxes(self):
        self.prune_inactive_boxes()
        return self._boxes
