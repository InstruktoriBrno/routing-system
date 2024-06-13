#pragma once

#include <cstdint>
#include <esp_wifi.h>
#include <array>
#include <serdes.hpp>
#include <sys/types.h>
#include <variant>
#include "mdf_common.h"
#include "mwifi.h"
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
    int round_download_progress;

    template <typename Archive>
    void serialize(Archive& archive) const {
        archive.push(type);
        archive.push(parent);
        archive.push(active_round_id);
        archive.push(active_round_hash);
        archive.push(round_download_progress);
    }

    template<typename Archive>
    static NodeStatusMessage deserialize(Archive& archive) {
        NodeStatusMessage msg;
        archive.pop(msg.type);
        archive.pop(msg.parent);
        archive.pop(msg.active_round_id);
        archive.pop(msg.active_round_hash);
        archive.pop(msg.round_download_progress);

        return msg;
    }
};

struct RoundHeaderMessage {
    static constexpr uint8_t MESSAGE_TYPE = 2;

    int round_id;
    Sha256 hash;
    int items;

    template <typename Archive>
    static RoundHeaderMessage deserialize(Archive& archive) {
        RoundHeaderMessage msg;

        archive.pop(msg.round_id);
        archive.pop(msg.hash);
        archive.pop(msg.items);

        return msg;
    }
};

template <typename Self>
struct GenericNetworkItemDefinitionMessage {
    int round_id;
    Sha256 hash;
    int initial_idx;
    int final_idx;
    tcb::span<uint8_t> data;

    template<typename Archive>
    static Self deserialize(Archive& archive) {
        Self msg;
        archive.pop(msg.round_id);
        archive.pop(msg.hash);
        archive.pop(msg.initial_idx);
        archive.pop(msg.final_idx);
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

class MessageHandler {
public:
    virtual void operator()(MacAddress source, const NodeStatusMessage& msg) {};
    virtual void operator()(MacAddress source, const RoundHeaderMessage& msg) {};
    virtual void operator()(MacAddress source, const RouterDefinitionMessage& msg) {};
    virtual void operator()(MacAddress source, const LinkDefinitionMessage& msg) {};
    virtual void operator()(MacAddress source, const PacketDefinitionMessage& msg) {};
    virtual void operator()(MacAddress source, const EventDefinitionMessage& msg) {};
};

void initialize_mesh_network_as_peer();
void initialize_mesh_network_as_root();
void handle_incoming_messages(MessageHandler& handler);

uint32_t network_millis();

const MacAddress& my_mac_address();

/**
 * Reports the status of the box to the system. This is also used to report that the device is
 */
void report_box_status(int active_round_id, const Sha256& active_round_hash, int round_download_progress);
bool broadcast_raw_message(tcb::span<uint8_t> message);



bool am_i_root();
const mesh_addr_t* root_address();

