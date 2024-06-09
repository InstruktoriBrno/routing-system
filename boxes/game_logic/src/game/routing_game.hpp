#pragma once
#include <vector>
#include <map>
#include <set>
#include <cstdint>
#include <algorithm>
#include <string>
#include <optional>
#include <cassert>
#include <bitset>

namespace rg {

using TeamId = uint8_t;

using RouterId = int;

using CardPhysicalId = uint64_t;

using CardSeqNum = uint8_t;

struct CardLogicalId {
    TeamId color;
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
        assert(_routers.count(from));
        assert(_routers.count(to));

        return _edges.find(from)->second.count(to);
    }
};

enum class PacketType {
    Nonexistent,
    Standard,
    VisitAll,
    Priority
};

struct PacketInfo {
    PacketType type;
    RouterId source;

    std::optional<RouterId> destination = std::nullopt;
    std::optional<int> points = std::nullopt;
};

enum class TopologyEventType {
    LinkDown,
    LinkUp
};

struct TopologyEvent {
    TopologyEventType type;
    int timestamp;
    std::optional<std::tuple<RouterId, RouterId>> edge;
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
    int _who_am_i;

    Network _initial_network;
    Network _current_network;

    std::vector<TopologyEvent> _topology_events;
    std::map<CardSeqNum, PacketInfo> _packet_infos;

    int _current_timestamp = 0;
    int _current_event_idx = 0;
public:
    RoundSetup(int who_am_i, const Network& network,
        const std::vector<TopologyEvent>& events, const std::map<CardSeqNum,
        PacketInfo> _packet_infos)
    : _who_am_i(who_am_i), _initial_network(network), _current_network(_initial_network),
      _topology_events(events), _packet_infos(_packet_infos)
    {
        std::sort(_topology_events.begin(), _topology_events.end(),
            [](const auto& a, const auto& b){ return a.timestamp < b.timestamp; });
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
            return { .type = PacketType::Nonexistent };
        else
            return info_it->second;
    }

    int time() const {
        return _current_timestamp;
    }

    int who_am_i() const {
        return _who_am_i;
    }

    void advance_time_to(int target_timestamp) {
        assert(target_timestamp > _current_timestamp);
        _current_timestamp = target_timestamp;
        while (_current_event_idx < _topology_events.size()) {
            const TopologyEvent& event = _topology_events[_current_event_idx];
            if (event.timestamp > target_timestamp)
                return;
            apply_topology_event(_current_network, event);
            _current_event_idx++;
        }
    }
};

/**
 * Structure describing a visit of a card to a router. This structure is
 * preserved on the card's memory.
 */
struct PacketVisit {
    RouterId where;
    int timestamp;
    bool any_points_awarded;
    int points;
};

/**
 * Structure describing UI screen/action after a card was processed.
 */
struct UiAction {
    bool valid;
    // TBA add more expressions
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
