#pragma once

#include <routing_game.hpp>
#include <string>
#include <nlohmann/json.hpp>

namespace rg::io {

std::string to_dot(const rg::Network& network);

template <typename T>
T from_json(const nlohmann::json& json);

template <>
RouterId from_json(const nlohmann::json& json);

std::tuple<rg::RouterId, rg::RouterId> link_from_json(const nlohmann::json& json);

template<>
Network from_json(const nlohmann::json& json);

template<>
PacketType from_json(const nlohmann::json& json);

template<>
PacketInfo from_json(const nlohmann::json& json);

template<>
TopologyEventType from_json(const nlohmann::json& json);

template<>
TopologyEvent from_json(const nlohmann::json& json);

RoundSetup round_setup_from_json(rg::RouterId who_am_i, const nlohmann::json& json);


} // namespace rg::io
