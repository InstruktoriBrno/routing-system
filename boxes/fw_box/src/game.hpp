#pragma once

#include "mesh_net.hpp"
#include <cassert>
#include <vector>
#include <algorithm>

#include <ArduinoJson.h>
#include <routing_game.hpp>
#include <span.hpp>
#include <string_view>
#include <sys.hpp>
#include <mesh_net.hpp>

struct BigRamAllocator: ArduinoJson::Allocator {
    void* allocate(size_t size) override {
        return heap_caps_malloc(size, MALLOC_CAP_8BIT);
    }
    void deallocate(void* pointer) override {
        heap_caps_free(pointer);
    }

    void* reallocate(void* ptr, size_t new_size) override {
    return heap_caps_realloc(ptr, new_size, MALLOC_CAP_8BIT);
  }

  static BigRamAllocator* instance() {
    static BigRamAllocator instance;
    return &instance;
  }
};




inline MacAddress parseMAC(const char *macStr) {
    MacAddress mac;
    char buf[3] = {0};

    for (int i = 0; i != 6; i++) {
        buf[0] = macStr[3 * i];
        buf[1] = macStr[3 * i + 1];
        mac[i] = strtoul(buf, nullptr, 16);
    }

    return mac;
}

inline rg::PacketType packet_type_from_str(const char* s) {
    using namespace std::literals;

    if (s == "admin"sv)
        return rg::PacketType::Admin;
    if (s == "standard"sv)
        return rg::PacketType::Standard;
    if (s == "priority"sv)
        return rg::PacketType::Priority;
    if (s == "hopper"sv)
        return rg::PacketType::Hopper;
    if (s == "visitall"sv)
        return rg::PacketType::VisitAll;
    if (s == "locator"sv)
        return rg::PacketType::Locator;
    if (s == "tcp"sv)
        return rg::PacketType::TCP;
    if (s == "chat"sv)
        return rg::PacketType::Chat;

    system_trap((std::string("Unknown packet type: ") + s).c_str());
}

inline rg::TopologyEventType topology_event_type_from_str(const char* s) {
    using namespace std::literals;

    if (s == "linkdown"sv)
        return rg::TopologyEventType::LinkDown;
    if (s == "linkup"sv)
        return rg::TopologyEventType::LinkUp;

    system_trap((std::string("Unknown topology event type: ") + s).c_str());
}

class GameUpdater {
    struct Interval {
        int start;
        int end;

        bool operator<(const Interval& other) const {
            return start < other.start;
        }
    };

    template <typename T>
    class List {
        std::vector<Interval> _intervals;
        std::vector<T> _items;
        int _total = 0;

        void _merge_intervals() {
            if (_intervals.empty())
            return;

            std::sort(_intervals.begin(), _intervals.end());

            int index = 0;

            for (int i = 1; i < _intervals.size(); ++i) {
                if (_intervals[index].end >= _intervals[i].start) {
                    _intervals[index].end = std::max(_intervals[index].end, _intervals[i].end);
                } else {
                    ++index;
                    _intervals[index] = _intervals[i];
                }
            }

            _intervals.resize(index + 1);
        }
    public:
        void clear() {
            _intervals.clear();
            _items.clear();
            _total = -1;
        }

        void set_total(int total) {
            _total = total;
            _items.resize(total);
        }

        int total() const {
            if (_total == -1)
                return 0;
            return _total;
        }

        bool has_range(int start, int end) const {
            for (const auto& interval : _intervals) {
                if (interval.start <= start && interval.end >= end) {
                    return true;
                }
            }

            return false;
        }

        int items_count() const {
            int count = 0;
            for (const auto& interval : _intervals) {
                count += interval.end - interval.start;
            }

            return count;
        }

        bool is_complete() const {
            return _total == 0 || (_intervals.size() == 1 && _intervals[0].start == 0 && _intervals[0].end == _total);
        }

        void add_item(int idx, const T& item) {
            _intervals.push_back({idx, idx + 1});
            _merge_intervals();
            _items[idx] = item;
        }

        const std::vector<T>& items() const {
            return _items;
        }
    };

    int _round_id = 0;
    Sha256 _round_hash = {0};
    int _round_duration = 0;

    rg::RouterId _who_am_i = -1;

    List<rg::RouterId> _routers;
    List<std::tuple<rg::RouterId, rg::RouterId>> _links;
    List<std::tuple<rg::CardSeqNum, rg::PacketInfo>> _packets;
    List<rg::TopologyEvent> _events;
public:
    GameUpdater() = default;

    int round_id() const {
        return _round_id;
    }

    const Sha256& round_hash() const {
        return _round_hash;
    }

    rg::RouterId who_am_i() const {
        return _who_am_i;
    }

    void on_round_header(int round_id, const Sha256& round_hash, int round_duration) {
        if (round_id == _round_id && round_hash == _round_hash)
            return;

        _routers.clear();
        _links.clear();
        _packets.clear();
        _events.clear();
        _who_am_i = -1;

        _round_id = round_id;
        _round_hash = round_hash;
        _round_duration = round_duration;
    }

    void on_router_definition_pack(int total, int start_idx, int end_idx, tcb::span<uint8_t> data) {
        _routers.set_total(total);
        if (_routers.has_range(start_idx, end_idx))
            return;

        auto data_it = data.begin();
        int idx_to_add = start_idx;
        while (data_it != data.end()) {
            rg::RouterId router_id = *data_it;
            ++data_it;

            auto end_it = data_it;
            while (end_it != data.end() && *end_it != 0)
                ++end_it;

            JsonDocument doc(BigRamAllocator::instance());
            DeserializationError error = deserializeJson(doc, reinterpret_cast<char*>(&*data_it), end_it - data_it);
            if (error) {
                rg_log_e("game", "Failed to deserialize router packet definition: %s, %s", error.c_str(), reinterpret_cast<char*>(&*data_it));
                system_trap((std::string("Cannot read routers:\n") + error.c_str()).c_str());
                return;
            }
            JsonObject object = doc.as<JsonObject>();
            for (JsonVariant item : object["mac"].as<JsonArray>()) {
                auto mac = parseMAC(item.as<const char*>());
                if (parseMAC(item.as<const char*>()) == my_mac_address()) {
                    _who_am_i = router_id;
                    rg_log_d("game", "I am %d", _who_am_i);
                    break;
                }
            }

            _routers.add_item(idx_to_add, router_id);
            rg_log_i("LOAD", "Router %d", router_id);

            data_it = ++end_it;
            idx_to_add++;
        }
    }

    void on_link_definition_pack(int total, int start_idx, int end_idx, tcb::span<uint8_t> data) {
        _links.set_total(total);
        if (_links.has_range(start_idx, end_idx))
            return;

        auto data_it = data.begin();
        int idx_to_add = start_idx;
        while (data_it != data.end()) {
            auto end_it = data_it;
            while (end_it != data.end() && *end_it != 0)
                ++end_it;

            JsonDocument doc(BigRamAllocator::instance());
            DeserializationError error = deserializeJson(doc, reinterpret_cast<char*>(&*data_it), end_it - data_it);
            if (error) {
                rg_log_e("game", "Failed to deserialize link definition: %s", error.c_str());
                system_trap((std::string("Cannot read links:\n") + error.c_str()).c_str());
                return;
            }

            const char* link = doc.as<const char*>();
            rg::RouterId from = link[0];
            rg::RouterId to = link[1];

            _links.add_item(idx_to_add, {from, to});
            rg_log_i("LOAD", "Link %d -> %d", from, to);

            idx_to_add++;
            data_it = ++end_it;
        }
    }

    void on_packet_definition_pack(int total, int start_idx, int end_idx, tcb::span<uint8_t> data) {
        _packets.set_total(total);
        if (_packets.has_range(start_idx, end_idx))
            return;

        auto data_it = data.begin();
        int idx_to_add = start_idx;
        while (data_it != data.end()) {
            int packet_id = *data_it;
            ++data_it;

            auto end_it = data_it;
            while (end_it != data.end() && *end_it != 0)
                ++end_it;

            rg_log_d("game", "Pdef: %s", reinterpret_cast<char*>(&*data_it));
            JsonDocument doc(BigRamAllocator::instance());
            DeserializationError error = deserializeJson(doc, reinterpret_cast<char*>(&*data_it), end_it - data_it);
            if (error) {
                rg_log_e("game", "Failed to deserialize router packet definition: %s, %s", error.c_str(), reinterpret_cast<char*>(&*data_it));
                system_trap((std::string("Cannot read packets:\n") + error.c_str()).c_str());
                return;
            }

            JsonObject object = doc.as<JsonObject>();
            rg::PacketInfo packet_info;

            packet_info.type = packet_type_from_str(object["type"].as<const char*>());
            packet_info.releaseTime = object["releaseTime"].as<int>();
            packet_info.source = object["source"].as<const char*>()[0];

            if (object.containsKey("destination"))
                packet_info.destination = object["destination"].as<const char*>()[0];
            if (object.containsKey("points"))
                packet_info.points = object["points"].as<int>();
            if (object.containsKey("pointsPerMinute_left"))
                packet_info.pointsPerMinuteLeft = object["pointsPerMinute_left"].as<int>();
            if (object.containsKey("minutesToDeliver"))
                packet_info.minutesToDeliver = object["minutesToDeliver"].as<int>();
            if (object.containsKey("pointsPerHop"))
                packet_info.pointsPerHop = object["pointsPerHop"].as<int>();

            _packets.add_item(idx_to_add, {packet_id, packet_info});

            data_it = ++end_it;
            idx_to_add++;
        }
    }

    void on_topology_event_pack(int total, int start_idx, int end_idx, tcb::span<uint8_t> data) {
        _events.set_total(total);
        if (_events.has_range(start_idx, end_idx))
            return;

        auto data_it = data.begin();
        int idx_to_add = start_idx;
        while (data_it != data.end()) {
            auto end_it = data_it;
            while (end_it != data.end() && *end_it != 0)
                ++end_it;

            JsonDocument doc(BigRamAllocator::instance());
            DeserializationError error = deserializeJson(doc, reinterpret_cast<char*>(&*data_it), end_it - data_it);
            if (error) {
                rg_log_e("game", "Failed to deserialize topology event definition: %s", error.c_str());
                system_trap((std::string("Cannot read topology events:\n") + error.c_str()).c_str());
                return;
            }

            JsonObject object = doc.as<JsonObject>();
            rg::TopologyEvent topology_event;

            topology_event.type = topology_event_type_from_str(object["type"].as<const char*>());
            topology_event.time = object["time"].as<int>();

            if (object.containsKey("link")) {
                auto link = object["link"].as<const char*>();
                topology_event.link = {link[0], link[1]};
            }

            _events.add_item(idx_to_add, topology_event);

            data_it = ++end_it;
            idx_to_add++;
        }
    }

    bool is_update_in_progress() const {
        return !_routers.is_complete() || !_links.is_complete() || !_packets.is_complete() || !_events.is_complete();
    }

    int update_percents() const {
        int items_count = _routers.items_count() + _links.items_count() + _packets.items_count() + _events.items_count();
        int total = _routers.total() + _links.total() + _packets.total() + _events.total();
        if (total == 0)
            return 100;
        return items_count * 100 / total;
    }

    rg::RoundSetup build_round_setup() const {
        rg_log_i("G", "Free heap: %d", ESP.getFreeHeap());
        rg_log_i("G", "Biggest memory chunk: %d", heap_caps_get_largest_free_block(MALLOC_CAP_32BIT));

        rg::Network network;
        for (rg::RouterId router_id : _routers.items())
            network.add_router(router_id);
        for (const auto& [from, to] : _links.items())
            network.add_connection(from, to);

        std::map<rg::CardSeqNum, rg::PacketInfo> packets;
        for (const auto& [seq, info] : _packets.items())
            packets[seq] = info;

        rg_log_i("game", "Who am I: %d", _who_am_i);

        return rg::RoundSetup(
            _who_am_i,
            _round_duration,
            network,
            _events.items(),
            packets
        );
    }
};

enum class GameState {
    NotRunning = 0,
    Preparation,
    Running,
    Paused,
    Finished
};

inline const char *game_state_to_str(GameState state) {
    switch (state) {
        case GameState::NotRunning:
            return "Not running";
        case GameState::Preparation:
            return "Preparation";
        case GameState::Running:
            return "Running";
        case GameState::Paused:
            return "Paused";
        case GameState::Finished:
            return "Finished";
    }
    system_trap("Unknown game state");
}

class Game {
    GameState _state = GameState::NotRunning;
    Sha256 _current_round_hash = {0};
    std::optional<rg::RoundSetup> _round_setup = std::nullopt;
    uint32_t _time_offset = 0;
    uint32_t _current_time = 0;
public:
    void update(uint32_t time) {
        _current_time = time;

        if (_state == GameState::Running) {
            if (!_round_setup)
                system_trap("Round setup is not set");
            _round_setup->advance_time_to(game_time());
            if (_round_setup->is_finished())
                _state = GameState::Finished;
        }
    }

    bool has_old_round_setup(const Sha256& round_hash) const {
        return !_round_setup || _current_round_hash != round_hash;
    }

    void update_round_setup(rg::RoundSetup&& setup, Sha256 round_hash) {
        _current_round_hash = round_hash;
        _round_setup = std::move(setup);
    }

    GameState state() const {
        return _state;
    }

    void prepare_game() {
        if (!_round_setup) {
            rg_log_w("game", "Cannot enter preparation state without round setup");
            return;
        }
        _state = GameState::Preparation;
    }

    void start_game(uint32_t time_offset) {
        if (!_round_setup) {
            rg_log_w("game", "Cannot start game without round setup");
            return;
        }

        _time_offset = time_offset;
        _state = GameState::Running;
    }

    void pause_game() {
        _state = GameState::Paused;
    }

    void stop_game() {
        _state = GameState::NotRunning;
    }

    uint32_t game_time() const {
        if (_state == GameState::NotRunning)
            return 0;
        return (_current_time - _time_offset) / 1000;
    }

    rg::UiAction handle_packet_visit(rg::CardCommInterface& card) {
        if (!_round_setup) {
            rg_log_w("game", "Cannot handle packet visit without round setup");
            return {
                .result = rg::PacketVisitResult::Invalid
            };
        }

        rg_log_i("HANDLE", "Who am I: %d", _round_setup->who_am_i());
        return rg::handle_packet_visit(*_round_setup, card);
    }

    rg::RouterId who_am_i() const {
        if (!_round_setup) {
            rg_log_w("game", "Cannot get who am I without round setup");
            return -1;
        }

        return _round_setup->who_am_i();
    }

};




