#pragma once
#include <vector>
#include <map>
#include <set>
#include <cstdint>
#include <algorithm>
#include <string>
#include <array>
#include <optional>
#include <cassert>
#include <bitset>
#include <tuple>

namespace rg {

void log(const char* fmt, ...);
void set_log_sink(void (*sink)(const char*, va_list));

using TeamId = uint16_t;

using RouterId = char;

using CardPhysicalId = std::array<uint8_t, 7>;

using CardSeqNum = uint16_t;

struct CardLogicalId {
    TeamId team_id;
    CardSeqNum seq;
};

class Network {
private:
    std::set<RouterId> _routers;
    std::map<RouterId, std::set<RouterId>> _edges;
public:
    void add_router(RouterId router_id) {
        _routers.insert(router_id);
    }

    void add_connection(RouterId from, RouterId to) {
        assert(_routers.count(from));
        assert(_routers.count(to));

        _edges[from].insert(to);
        _edges[to].insert(from);
    }

    void remove_connection(RouterId from, RouterId to) {
        assert(_routers.count(from));
        assert(_routers.count(to));

        _edges[from].erase(to);
        _edges[to].erase(from);
    }

    const std::set<RouterId>& routers() const {
        return _routers;
    }

    const std::set<RouterId>& neighbors(RouterId router_id) const {
        assert(_routers.count(router_id));

        return _edges.find(router_id)->second;
    }

    const bool are_neighbors(RouterId from, RouterId to) const {
        rg::log("Checking if %d is neighbor of %d", from, to);
        assert(_routers.count(from));
        assert(_routers.count(to));

        return from == to || _edges.find(from)->second.count(to);
    }
};

enum class PacketType {
    Admin,
    Standard, // deprecated
    Priority,
    Hopper, // deprecated
    VisitAll,
    Locator,
    TCP,
    Chat,
};

struct PacketInfo {
    PacketType type;
    RouterId source;

    std::optional<RouterId> destination = std::nullopt;
    int points = 10;
    int pointsPerMinuteLeft = 4;
    int minutesToDeliver = 5;
    int pointsPerHop = 1;
};

enum class TopologyEventType {
    LinkDown,
    LinkUp
};

struct TopologyEvent {
    TopologyEventType type;
    int time;
    std::optional<std::tuple<RouterId, RouterId>> link;
};

void apply_topology_event(Network& network, const TopologyEvent& event);

/**
 * The Routing game is played in independent rounds. This class represents the
 * game setup of a single round. Therefore, it contains the initial network, the
 * events to topology, and the packet information.
 *
 * The round setup also carries a time information and this should be the time
 * source for the game logic. The time can be externally advanced by calling the
 * advance_time_to method.
 */
class RoundSetup {
private:
    RouterId _who_am_i;

    Network _initial_network;
    Network _current_network;

    std::vector<TopologyEvent> _topology_events;
    std::map<CardSeqNum, PacketInfo> _packet_infos;

    uint32_t _current_time = 0;
    int _current_event_idx = 0;

    int _duration = 0;
public:
    RoundSetup(RouterId who_am_i, int duration, const Network& network,
        const std::vector<TopologyEvent>& events, const std::map<CardSeqNum,
        PacketInfo> _packet_infos)
    : _who_am_i(who_am_i), _initial_network(network), _current_network(_initial_network),
      _topology_events(events), _packet_infos(_packet_infos), _duration(duration)
    {
        std::sort(_topology_events.begin(), _topology_events.end(),
            [](const auto& a, const auto& b){ return a.time < b.time; });
    }

    const Network& initial_network() const {
        return _initial_network;
    }

    const Network& network() const {
        return _current_network;
    }

    const std::vector<TopologyEvent>& topology_events() const {
        return _topology_events;
    }

    PacketInfo packet_info(CardSeqNum seq) const {
        auto info_it = _packet_infos.find(seq);
        if (info_it == _packet_infos.end())
            return { .type = PacketType::Locator };
        else
            return info_it->second;
    }

    int time() const {
        return _current_time;
    }

    int duration() const {
        return _duration;
    }

    RouterId who_am_i() const {
        return _who_am_i;
    }

    void advance_time_to(uint32_t target_time) {
        if (target_time < _current_time) {
            _current_event_idx = 0;
            _current_network = _initial_network;
        }

        _current_time = target_time;
        while (_current_event_idx < _topology_events.size()) {
            const TopologyEvent& event = _topology_events[_current_event_idx];
            if (event.time > target_time)
                return;
            apply_topology_event(_current_network, event);
            _current_event_idx++;
        }
    }

    bool is_finished() const {
        return _current_time >= _duration;
    }
};

/**
 * Structure describing a visit of a card to a router. This structure is
 * preserved on the card's memory.
 */
struct PacketVisit {
    RouterId where;
    int time;
    bool flag1;
    bool flag2;
    bool flag3;
    bool flag4;
    int points;
};

enum class PacketVisitResult {
    Invalid = 0,
    Continue = 1,
    Finished = 2
};

/**
 * Structure describing UI screen/action after a card was processed.
 */
struct UiAction {
    PacketVisitResult result;
    std::optional<std::string> instructions;
    std::optional<int> points;
    std::string log;
};

/**
 * Interface for communication with a card.
 *
 * The card:
 * - has a logical logical ID,
 * - carries a list of router visits; you can index it from the first to the
 *   last (when using positive index) or from last to the first (when using
 *   negative index),
 * - has a 32-bit metadata field that can be read and written.
 */
class CardCommInterface {
public:
    virtual ~CardCommInterface() = default;

    virtual CardLogicalId get_id() = 0;
    virtual CardSeqNum get_seq() = 0;
    virtual int visit_count() = 0;
    virtual PacketVisit get_visit(int idx) = 0;
    virtual void mark_visit(PacketVisit) = 0;

    virtual std::bitset<32> get_metadata() = 0;
    virtual void set_metadata(std::bitset<32>) = 0;
};

/**
 * The main entrypoint of the game logic. This function is called when a card
 * visits a router. The game logic can then decide what to do with the card.
 *
 * It is expected that the game logic will:
 * - mark the visit on the card (if the visit is valid), and
 * - update the card's metadata if necessary.
 *
 * The function returns a description of the action that the UI should take.
 */
UiAction handle_packet_visit(const RoundSetup& setup, CardCommInterface& card);

} // namespace rg
