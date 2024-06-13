#include "io_routing_game.hpp"
#include <stdexcept>
#include <string_view>

std::string rg::io::to_dot(const rg::Network& network) {
    std::string dot = "graph {\n";

    for (const auto& router : network.routers()) {
        dot += std::string(1, router) + ";\n";
    }

    for (const auto& router : network.routers()) {
        for (const auto& neighbor : network.neighbors(router)) {
            if (router < neighbor) {
                dot += std::string(1, router) + " -- " + std::string(1, neighbor) + ";\n";
            }
        }
    }

    dot += "}";

    return dot;
}

template <>
rg::RouterId rg::io::from_json(const nlohmann::json& json) {
    if (!json.is_string() || json.get<std::string>().size() != 1) {
        throw std::invalid_argument("RouterId has to be a single character, got '" + json.dump() + "'");
    }

    std::string s;
    json.get_to(s);

    return s[0];
}

std::tuple<rg::RouterId, rg::RouterId> rg::io::link_from_json(const nlohmann::json& json) {
    if (!json.is_string() || json.get<std::string>().size() != 2) {
        throw std::invalid_argument("Link has to be a string of length 2, got '" + json.dump() + "'");
    }

    std::string s;
    json.get_to(s);

    return std::make_tuple(s[0], s[1]);
}


template<>
rg::Network rg::io::from_json(const nlohmann::json& json) {
    if (!json.is_object()) {
        throw std::invalid_argument("Invalid JSON - expected object");
    }
    if (!json.contains("routers") || !json.contains("links")) {
        throw std::invalid_argument("Invalid JSON - expected 'routers' and 'links' keys");
    }
    if (!json["routers"].is_object()) {
        throw std::invalid_argument("Invalid JSON - expected 'routers' to be an object");
    }
    if (!json["links"].is_array()) {
        throw std::invalid_argument("Invalid JSON - expected 'links' to be an array");
    }

    rg::Network network;

    for (auto it : json["routers"].items()) {
        if (it.key().size() != 1) {
            throw std::invalid_argument("Invalid JSON - router ID must be a single character, got '" + it.key() + "'");
        }
        rg::RouterId id = it.key()[0];
        network.add_router(id);
    }

    for (auto it : json["links"].items()) {
        auto [from, to] = link_from_json(it.value());

        if (!network.routers().contains(from)) {
            throw std::invalid_argument("Invalid JSON - router ID " + std::to_string(from) + " not found in edge '" + it.value().dump() + "'");
        }
        if (!network.routers().contains(to)) {
            throw std::invalid_argument("Invalid JSON - router ID " + std::to_string(to) + " not found in edge '" + it.value().dump() + "'");
        }

        network.add_connection(from, to);
    }

    return network;
}

template<>
rg::PacketType rg::io::from_json(const nlohmann::json& json) {
    using namespace std::literals;

    static const std::map<std::string_view, rg::PacketType> TYPES = {
        {"nonexistent"sv, rg::PacketType::Nonexistent},
        {"standard"sv, rg::PacketType::Standard},
        {"visitall"sv, rg::PacketType::VisitAll},
        {"priority"sv, rg::PacketType::Priority},
        {"hopper"sv, rg::PacketType::Hopper},
    };

    if (!json.is_string()) {
        throw std::invalid_argument("PacketType has to be string");
    }

    std::string n;
    json.get_to(n);

    for (auto [name, type] : TYPES) {
        if (name == n)
            return type;
    }

    throw std::invalid_argument("Unknown packet type '" + n + "'");
}

template<>
rg::PacketInfo rg::io::from_json(const nlohmann::json& json) {
    if (!json.is_object()) {
        throw std::invalid_argument("Invalid JSON - expected object");
    }
    if (!json.contains("type")) {
        throw std::invalid_argument("Invalid JSON - expected 'type' key");
    }
    if (!json.contains("source") || !json["source"].is_string()) {
        throw std::invalid_argument("Invalid JSON - expected 'source' to be a single letter");
    }

    rg::PacketInfo info;
    info.type = from_json<rg::PacketType>(json["type"]);
    info.source = from_json<rg::RouterId>(json["source"]);

    if (json.contains("destination")) {
        info.destination = from_json<rg::RouterId>(json["destination"]);
    }

    if (json.contains("points")) {
        if (!json["points"].is_number()) {
            throw std::invalid_argument("Invalid JSON - expected 'points' to be a number");
        }
        info.points = json["points"].get<int>();
    }

    return info;
}

template<>
rg::TopologyEventType rg::io::from_json(const nlohmann::json& json) {
    using namespace std::literals;

    static const std::map<std::string_view, rg::TopologyEventType> TYPES = {
        {"linkup"sv, rg::TopologyEventType::LinkUp},
        {"linkdown"sv, rg::TopologyEventType::LinkDown}
    };

    if (!json.is_string()) {
        throw std::invalid_argument("TopologyEventType has to be string");
    }

    std::string n;
    json.get_to(n);

    for (auto [name, type] : TYPES) {
        if (name == n)
            return type;
    }

    throw std::invalid_argument("Unknown topology event type '" + n + "'");
}

template<>
rg::TopologyEvent rg::io::from_json(const nlohmann::json& json) {
    if (!json.is_object()) {
        throw std::invalid_argument("Invalid JSON - expected object");
    }
    if (!json.contains("type")) {
        throw std::invalid_argument("Invalid JSON - expected 'type' key");
    }
    if (!json.contains("time") || !json["time"].is_number()) {
        throw std::invalid_argument("Invalid JSON - expected 'time' to be a number");
    }

    rg::TopologyEvent event;
    event.type = from_json<rg::TopologyEventType>(json["type"]);
    event.time = json["time"].get<int>();

    if (json.contains("link")) {
        event.link = link_from_json(json["link"]);
    }

    return event;
}

rg::RoundSetup rg::io::round_setup_from_json(rg::RouterId who_am_i, const nlohmann::json& json) {
    if (!json.is_object()) {
        throw std::invalid_argument("Invalid JSON - expected object");
    }

    if (!json.contains("duration") || !json["duration"].is_number()) {
        throw std::invalid_argument("Invalid JSON - expected 'duration' to be a number");
    }

    int duration = json["duration"].get<int>();

    auto network = from_json<rg::Network>(json);

    if (!json.contains("events") || !json["events"].is_array()) {
        throw std::invalid_argument("Invalid JSON - expected 'events' to be an array");
    }
    std::vector<rg::TopologyEvent> events;
    for (const auto& event_json : json["events"]) {
        events.push_back(from_json<rg::TopologyEvent>(event_json));
    }

    if (!json.contains("packets") || !json["packets"].is_object()) {
        throw std::invalid_argument("Invalid JSON - expected 'packets' to be an object");
    }
    std::map<rg::CardSeqNum, rg::PacketInfo> packet_infos;
    for (auto it : json["packets"].items()) {
        if (!it.value().is_object()) {
            throw std::invalid_argument("Invalid JSON - expected packet info to be an object");
        }
        rg::CardSeqNum seq;
        try {
            seq = std::stoull(it.key());
        }
        catch (std::invalid_argument& e) {
            throw std::invalid_argument("Invalid JSON - packet sequence number must be an integer, got '" + it.key() + "'");
        }
        packet_infos[seq] = from_json<rg::PacketInfo>(it.value());
    }

    return rg::RoundSetup(who_am_i, duration, network, events, packet_infos);
}
