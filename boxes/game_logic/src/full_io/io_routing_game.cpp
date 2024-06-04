#include "io_routing_game.hpp"
#include <stdexcept>
#include <string_view>

std::string rg::io::to_dot(const rg::Network& network) {
    std::string dot = "graph {\n";

    for (const auto& router : network.routers()) {
        dot += std::to_string(router) + ";\n";
    }

    for (const auto& router : network.routers()) {
        for (const auto& neighbor : network.neighbors(router)) {
            if (router < neighbor) {
                dot += std::to_string(router) + " -- " + std::to_string(neighbor) + ";\n";
            }
        }
    }

    dot += "}";

    return dot;
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
        try {
            rg::RouterId id = std::stoi(it.key());
            network.add_router(id);
        }
        catch (std::invalid_argument& e) {
            throw std::invalid_argument("Invalid JSON - router ID must be an integer, got '" + it.key() + "'");
        }
    }

    for (auto it : json["links"].items()) {
        if (!it.value().is_array() || it.value().size() != 2 || !it.value()[0].is_number() || !it.value()[1].is_number()) {
            throw std::invalid_argument("Invalid JSON - expected link to be an array of 2 integers, got '" + it.value().dump() + "'");
        }
        rg::RouterId from, to;
        it.value()[0].get_to(from);
        it.value()[1].get_to(to);

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
        {"priority"sv, rg::PacketType::Priority}
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
    if (!json.contains("source") || !json["source"].is_number()) {
        throw std::invalid_argument("Invalid JSON - expected 'source' to be a number");
    }

    rg::PacketInfo info;
    info.type = from_json<rg::PacketType>(json["type"]);
    info.source = json["source"].get<rg::RouterId>();

    if (json.contains("destination")) {
        if (!json["destination"].is_number()) {
            throw std::invalid_argument("Invalid JSON - expected 'destination' to be a number");
        }
        info.destination = json["destination"].get<rg::RouterId>();
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
    if (!json.contains("timestamp") || !json["timestamp"].is_number()) {
        throw std::invalid_argument("Invalid JSON - expected 'timestamp' to be a number");
    }

    rg::TopologyEvent event;
    event.type = from_json<rg::TopologyEventType>(json["type"]);
    event.timestamp = json["timestamp"].get<int>();

    if (json.contains("edge")) {
        if (!json["edge"].is_array() || json["edge"].size() != 2 || !json["edge"][0].is_number() || !json["edge"][1].is_number()) {
            throw std::invalid_argument("Invalid JSON - expected 'edge' to be an array of 2 numbers");
        }
        event.edge = std::make_tuple(json["edge"][0].get<rg::RouterId>(), json["edge"][1].get<rg::RouterId>());
    }

    return event;
}

rg::RoundSetup rg::io::round_setup_from_json(rg::RouterId who_am_i, const nlohmann::json& json) {
    if (!json.is_object()) {
        throw std::invalid_argument("Invalid JSON - expected object");
    }

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

    return rg::RoundSetup(who_am_i, network, events, packet_infos);
}
