#pragma once

#include <cstdint>
#include <esp_wifi.h>
#include <array>
#include <optional>
#include <serdes.hpp>
#include <sys/types.h>
#include <variant>
#include "esp_mesh.h"
#include "nvs_flash.h"
#include "serdes.hpp"

class MacAddress: public std::array<uint8_t, 6> {
public:
    MacAddress() = default;
    MacAddress(const mesh_addr_t& addr) {
        for (int i = 0; i < 6; i++) {
            (*this)[i] = addr.addr[i];
        }
    }
};

using Sha256 = std::array<uint8_t, 32>;

enum class NodeType: uint8_t {
    Root = 0,
    Box = 1,
    Repeater = 2
};

struct NodeStatusMessage {
    static constexpr uint8_t MESSAGE_TYPE = 1;

    NodeType type;
    MacAddress parent;

    int active_round_id;
    Sha256 active_round_hash;
    uint8_t round_download_progress;
    uint8_t game_state;
    uint16_t game_time;
    int8_t router_id;

    template <typename Archive>
    void serialize(Archive& archive) const {
        archive.push(type);
        archive.push(parent);
        archive.push(active_round_id);
        archive.push(active_round_hash);
        archive.push(round_download_progress);
        archive.push(game_state);
        archive.push(game_time);
        archive.push(router_id);
    }

    template<typename Archive>
    static NodeStatusMessage deserialize(Archive& archive) {
        NodeStatusMessage msg;
        archive.pop(msg.type);
        archive.pop(msg.parent);
        archive.pop(msg.active_round_id);
        archive.pop(msg.active_round_hash);
        archive.pop(msg.round_download_progress);
        archive.pop(msg.game_state);
        archive.pop(msg.game_time);
        archive.pop(msg.router_id);

        return msg;
    }
};

struct RoundHeaderMessage {
    static constexpr uint8_t MESSAGE_TYPE = 2;

    int round_id;
    Sha256 hash;
    int duration;

    template <typename Archive>
    static RoundHeaderMessage deserialize(Archive& archive) {
        RoundHeaderMessage msg;

        archive.pop(msg.round_id);
        archive.pop(msg.hash);
        archive.pop(msg.duration);

        return msg;
    }
};

template <typename Self>
struct GenericNetworkItemDefinitionMessage {
    int round_id;
    Sha256 hash;
    int initial_idx;
    int final_idx;
    int total_count;
    tcb::span<uint8_t> data;

    template<typename Archive>
    static Self deserialize(Archive& archive) {
        Self msg;
        archive.pop(msg.round_id);
        archive.pop(msg.hash);
        archive.pop(msg.initial_idx);
        archive.pop(msg.final_idx);
        archive.pop(msg.total_count);
        msg.data = archive.remaining();
        return msg;
    }
};

struct RouterDefinitionMessage: public GenericNetworkItemDefinitionMessage<RouterDefinitionMessage> {
    static constexpr uint8_t MESSAGE_TYPE = 3;
};

struct LinkDefinitionMessage: public GenericNetworkItemDefinitionMessage<LinkDefinitionMessage> {
    static constexpr uint8_t MESSAGE_TYPE = 4;
};

struct PacketDefinitionMessage: public GenericNetworkItemDefinitionMessage<PacketDefinitionMessage> {
    static constexpr uint8_t MESSAGE_TYPE = 5;
};

struct EventDefinitionMessage: public GenericNetworkItemDefinitionMessage<EventDefinitionMessage> {
    static constexpr uint8_t MESSAGE_TYPE = 6;
};

struct PrepareGameMessage {
    static constexpr uint8_t MESSAGE_TYPE = 7;

    template <typename Archive>
    static PrepareGameMessage deserialize(Archive& archive) {
        return PrepareGameMessage();
    }
};

struct GameStateMessage {
    static constexpr uint8_t MESSAGE_TYPE = 8;

    uint8_t state;
    uint32_t time_offset;

    template <typename Archive>
    static GameStateMessage deserialize(Archive& archive) {
        GameStateMessage msg;
        archive.pop(msg.state);
        archive.pop(msg.time_offset);
        return msg;
    }
};

struct PacketVisitMessage {
    static constexpr uint8_t MESSAGE_TYPE = 9;

    uint16_t time;
    std::array<uint8_t, 7> physical_card_id;
    uint8_t team_id;
    uint8_t seq_num;
    uint8_t router_id;
    std::optional<uint8_t> score;

    template <typename Archive>
    void serialize(Archive& archive) const {
        archive.push(time);
        archive.push(physical_card_id);
        archive.push(team_id);
        archive.push(seq_num);
        archive.push(router_id);
        if (score.has_value()) {
            archive.push(uint8_t(1));
            archive.push(score.value());
        } else {
            archive.push(uint8_t(0));
            archive.push(uint8_t(0));
        }
        archive.push(score);
    }

    template <typename Archive>
    static PacketVisitMessage deserialize(Archive& archive) {
        PacketVisitMessage msg;
        archive.pop(msg.time);
        archive.pop(msg.physical_card_id);
        archive.pop(msg.team_id);
        archive.pop(msg.seq_num);
        archive.pop(msg.router_id);
        uint8_t has_score;
        archive.pop(has_score);
        if (has_score) {
            archive.pop(msg.score);
        } else {
            msg.score = std::nullopt;
        }

        return msg;
    }
};

class MessageHandler {
public:
    virtual void operator()(MacAddress source, const NodeStatusMessage& msg) {};
    virtual void operator()(MacAddress source, const RoundHeaderMessage& msg) {};
    virtual void operator()(MacAddress source, const RouterDefinitionMessage& msg) {};
    virtual void operator()(MacAddress source, const LinkDefinitionMessage& msg) {};
    virtual void operator()(MacAddress source, const PacketDefinitionMessage& msg) {};
    virtual void operator()(MacAddress source, const EventDefinitionMessage& msg) {};
    virtual void operator()(MacAddress source, const PrepareGameMessage& msg) {};
    virtual void operator()(MacAddress source, const GameStateMessage& msg) {};
    virtual void operator()(MacAddress source, const PacketVisitMessage& msg) {};
};

void initialize_mesh_network_as_peer();
void initialize_mesh_network_as_root();

void handle_incoming_messages(MessageHandler& handler);

uint32_t network_millis();

const MacAddress& my_mac_address();
bool is_mesh_connected();

/**
 * Reports the status of the box to the system. This is also used to report that the device is
 */
void report_box_status(int active_round_id, const Sha256& active_round_hash,
    uint8_t round_download_progress, uint8_t game_state, uint16_t game_time, int8_t router_id);
bool broadcast_raw_message(tcb::span<uint8_t> message);
bool send_message(const MacAddress& recipient, tcb::span<uint8_t> message);
bool send_packet_visit(uint16_t time, std::array<uint8_t, 7> physical_card_id, uint8_t team_id,
                       uint8_t seq_num, uint8_t router_id, std::optional<uint8_t> score);



bool am_i_root();
const mesh_addr_t* root_address();

