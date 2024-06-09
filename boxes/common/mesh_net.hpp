#pragma once

#include <cstdint>
#include <esp_wifi.h>
#include <array>
#include <serdes.hpp>
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

struct BoxStatusMessage {
    static constexpr uint8_t MESSAGE_TYPE = 1;

    uint8_t network_depth;
    MacAddress parent;
    int active_round_id;
    int round_download_progress;

    template <typename Archive>
    void serialize(Archive& archive) const {
        archive.push(network_depth);
        archive.push(parent);
        archive.push(active_round_id);
        archive.push(round_download_progress);
    }

    template<typename Archive>
    static BoxStatusMessage deserialize(Archive& archive) {
        BoxStatusMessage msg;
        archive.pop(msg.network_depth);
        archive.pop(msg.parent);
        archive.pop(msg.active_round_id);
        archive.pop(msg.round_download_progress);

        return msg;
    }
};

class MessageHandler {
public:
    virtual void operator()(MacAddress source, const BoxStatusMessage& msg) {};
};

void initialize_mesh_network_as_peer();
void initialize_mesh_network_as_root();
void handle_incoming_messages(MessageHandler& handler);

uint32_t network_millis();

/**
 * Reports the status of the box to the system. This is also used to report that the device is
 */
void report_box_status(int active_round_id, int round_download_progress);



bool am_i_root();
const mesh_addr_t* root_address();

