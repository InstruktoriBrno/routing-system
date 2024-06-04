#pragma once
#include <vector>
#include <map>
#include <set>
#include <cstdint>
#include <string>
#include <optional>
#include <cassert>

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
        assert(_routers.contains(from));
        assert(_routers.contains(to));

        _edges[from].insert(to);
        _edges[to].insert(from);
    }

    void remove_connection(RouterId from, RouterId to) {
        assert(_routers.contains(from));
        assert(_routers.contains(to));

        _edges[from].erase(to);
        _edges[to].erase(from);
    }

    const std::set<RouterId>& routers() const {
        return _routers;
    }

    const std::set<RouterId>& neighbors(RouterId router_id) const {
        assert(_routers.contains(router_id));

        return _edges.find(router_id)->second;
    }

    const bool are_neighbors(RouterId from, RouterId to) const {
        assert(_routers.contains(from));
        assert(_routers.contains(to));

        return _edges.find(from)->second.contains(to);
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

    const Network& network() const {
        return _current_network;
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

struct PacketVisit {
    RouterId where;
    int timestamp;
    bool any_points_awarded;
    int points;
};

struct UiAction {
    bool valid;
    // TBA add more expressions
};

class CardCommInterface {
public:
    virtual ~CardCommInterface() = default;

    virtual CardLogicalId get_id() = 0;
    virtual int visit_count() = 0;
    virtual PacketVisit get_visit(int idx) = 0;
    virtual void mark_visit(PacketVisit) = 0;
};

UiAction handle_packet_visit(const RoundSetup& setup, CardCommInterface& card);

} // namespace rg
