#include "audio.hpp"
#include <Arduino.h>

#include <cstdint>
#include <optional>
#include <cstring>

#include <esp32_smartdisplay.h>
#include <gui/gui.hpp>
#include <gui/main_screen.hpp>
#include <cards.hpp>
#include <mesh_net.hpp>

#include <SPIFFS.h>

#include <audio.hpp>
#include "mesh_net.hpp"
#include "timer.hpp"
#include "logging.hpp"
#include "sys.hpp"

static const char *TAG = "main";

std::unique_ptr<MainScreen> main_screen;

CardReader card_reader({
    .miso = 19, // 35,
    .mosi = 18, // 22,
    .sck = 23, // 21,
    .ss = 5 // 27
});

void setup()
{
    Serial.begin(921600);

    rg_set_log_handler(rg_serial_log_handler, nullptr);
    rg_set_log_severity(Severity::DEBUG);

    smartdisplay_init();
    setup_gui();

    if(!SPIFFS.begin(true)){
        rg_log_e(TAG, "An Error has occurred while mounting SPIFFS");
        system_trap("Failed to mount SPIFFS");
        return;
    }


    auto mac = my_mac_address();

    rg_log_i(TAG, "Board: %s", BOARD_NAME);
    rg_log_i(TAG, "CPU: %s rev%d, CPU Freq: %d Mhz, %d core(s)", ESP.getChipModel(), ESP.getChipRevision(), getCpuFrequencyMhz(), ESP.getChipCores());
    rg_log_i(TAG, "Free heap: %d bytes", ESP.getFreeHeap());
    rg_log_i(TAG, "Free PSRAM: %d bytes", ESP.getPsramSize());
    rg_log_i(TAG, "SDK version: %s", ESP.getSdkVersion());
    rg_log_i(TAG, "MAC address: %02x:%02x:%02x:%02x:%02x:%02x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

    initialize_mesh_network_as_peer();
    rg_log_i(TAG, "Mesh network initialized");

    card_reader.init();
    card_reader.debug_connection();

    setup_audio();

    main_screen = std::make_unique<MainScreen>();
    main_screen->activate();

    rg_log_i(TAG, "Free heap after setup: %d", ESP.getFreeHeap());
}

class BoxMessageHandler: public MessageHandler {
public:
    void operator()(MacAddress source, const NodeStatusMessage& msg) override {
        rg_log_i(TAG, "Received NodeStatusMessage from %02x:%02x:%02x:%02x:%02x:%02x",
            source[0], source[1], source[2], source[3], source[4], source[5]);
    };

    void operator()(MacAddress source, const RoundHeaderMessage& msg) override {
        rg_log_i(TAG, "Received RoundHeaderMessage from %02x:%02x:%02x:%02x:%02x:%02x",
            source[0], source[1], source[2], source[3], source[4], source[5]);
    }

    void operator()(MacAddress source, const RouterDefinitionMessage& msg) override {
        rg_log_i(TAG, "Received RouterDefinitionMessage from %02x:%02x:%02x:%02x:%02x:%02x",
            source[0], source[1], source[2], source[3], source[4], source[5]);
    }

    void operator()(MacAddress source, const LinkDefinitionMessage& msg) override {
        rg_log_i(TAG, "Received LinkDefinitionMessage from %02x:%02x:%02x:%02x:%02x:%02x",
            source[0], source[1], source[2], source[3], source[4], source[5]);
    }

    void operator()(MacAddress source, const PacketDefinitionMessage& msg) override {
        rg_log_i(TAG, "Received PacketDefinitionMessage from %02x:%02x:%02x:%02x:%02x:%02x",
            source[0], source[1], source[2], source[3], source[4], source[5]);
    }

    void operator()(MacAddress source, const EventDefinitionMessage& msg) override {
        rg_log_i(TAG, "Received EventDefinitionMessage from %02x:%02x:%02x:%02x:%02x:%02x",
            source[0], source[1], source[2], source[3], source[4], source[5]);
    }
};


uint32_t next_update = 0;
uint32_t clear_card_at = -1;

const int PING_INTERVAL = 1000;
uint32_t next_ping = millis() + PING_INTERVAL;

uint32_t ping_id_counter = 0;

uint32_t last_received_ping_id = 0;
uint32_t last_received_ping_duration = 0;
uint32_t last_received_ping_ts = 0;

PeriodicTimer status_timer(1000);
PeriodicTimer screen_timer(50);
PeriodicTimer card_clear_timer(2000);

void loop() {
    BoxMessageHandler msg_handler;
    handle_incoming_messages(msg_handler);

    if (screen_timer.elapsed()) {
        main_screen->update();
    }

    if (status_timer.elapsed()) {
        Sha256 round_id = {0};
        report_box_status(0, round_id, 0);
    }

    if (card_reader.has_new_card() && clear_card_at == -1)
    {
        auto start = millis();
        auto game_interface = card_reader.game_card_interface();
        Serial.println("Card detected");
        Serial.print("Card logical id: ");
        Serial.print(game_interface.get_id().team_id);
        Serial.print(":");
        Serial.println(game_interface.get_id().seq);
        Serial.print("Card metadata: ");
        Serial.println(game_interface.get_metadata().to_ulong());
        Serial.println("Visits:");
        for (int i = 0; i < game_interface.visit_count(); i++) {
            auto visit = game_interface.get_visit(i);
            Serial.print("  ");
            Serial.print(visit.where);
            Serial.print(" at ");
            Serial.print(visit.time);
            Serial.print(" points ");
            Serial.print(visit.points);
            Serial.println();
        }

        // if (!game_interface.reset_for_round(42)) {
        //     rg_log_e(TAG, "Failed to reset card for round");
        // }
        // if (!game_interface.write_logic_id({.team_id = 10, .seq = 11})) {
        //     rg_log_e(TAG, "Failed to write logic ID");
        // }
        // game_interface.mark_visit({
        //     .where = 13,
        //     .time = 42,
        //     .points_awarded = true,
        //     .points = 10
        // });
        // if (!game_interface.finish_transaction()) {
        //     rg_log_e(TAG, "Failed to finish transaction");
        // }
        auto end = millis();
        rg_log_i(TAG, "Card reset and written in %d ms", end - start);
        play_wav("/success.wav");

        delay(2000);

        main_screen->set_card_id(card_reader.card_uid_str());
        card_clear_timer.reset();
    }

    if (clear_card_at != -1 && card_clear_timer.elapsed())
    {
        clear_card_at = -1;
        main_screen->set_card_id(nullptr);
    }

    // card_reader.debug();

    // if (am_i_root()) {
    //     main_screen->set_network_message("Network status:\n  Role: root");
    // } else {
    //     main_screen->set_network_message("Network status:\n  Role: node\n  Root: %02x:%02x:%02x:%02x:%02x:%02x\n  Last ping ID: %d, %d ms, %d ms ago",
    //         root_address()->addr[0], root_address()->addr[1], root_address()->addr[2], root_address()->addr[3], root_address()->addr[4], root_address()->addr[5],
    //         last_received_ping_id, last_received_ping_duration, (now - last_received_ping_ts));
    // }

    // if (!am_i_root()) {
    //     if (now > next_ping) {
    //         next_ping = now + PING_INTERVAL;
    //         ping_id_counter++;

    //         uint8_t tx_buffer[8];
    //         memcpy(tx_buffer, &ping_id_counter, sizeof(ping_id_counter));
    //         auto ping_send_time = millis();
    //         memcpy(tx_buffer + 4, &ping_send_time, sizeof(ping_send_time));

    //         mesh_data_t data = {
    //             .data = tx_buffer,
    //             .size = sizeof(tx_buffer),
    //             .proto = MESH_PROTO_BIN,
    //             .tos = MESH_TOS_P2P
    //         };
    //         rg_log_i(TAG, "About to send ping %d to root", ping_id_counter);
    //         auto ret = esp_mesh_send(root_address(), &data, MESH_DATA_P2P | MESH_DATA_NONBLOCK, nullptr, 0);
    //         rg_log_i(TAG, "Sent ping %d to root, ret: %s", ping_id_counter, esp_err_to_name(ret));
    //     }
    // }

    // mesh_addr_t from;
    // uint8_t recv_buffer[128];
    // mesh_data_t data {
    //     .data = recv_buffer,
    //     .size = sizeof(recv_buffer)
    // };
    // int flag;
    // auto resp = esp_mesh_recv(&from, &data, 0, &flag, nullptr, 0);
    // if (resp == ESP_OK) {
    //     if (am_i_root()) {
    //         rg_log_i(TAG, "Received data from %02x:%02x:%02x:%02x:%02x:%02x", from.addr[0], from.addr[1], from.addr[2], from.addr[3], from.addr[4], from.addr[5]);
    //         esp_mesh_send(&from, &data, MESH_DATA_P2P, nullptr, 0);
    //     } else {
    //         rg_log_i(TAG, "Received data from %02x:%02x:%02x:%02x:%02x:%02x", from.addr[0], from.addr[1], from.addr[2], from.addr[3], from.addr[4], from.addr[5]);
    //         if (std::memcmp(from.addr, root_address()->addr, 5) == 0) {
    //             rg_log_i(TAG, "Updating ping", from.addr[0], from.addr[1], from.addr[2], from.addr[3], from.addr[4], from.addr[5]);

    //             uint32_t received_ping_id;
    //             uint32_t received_ping_time;
    //             memcpy(&received_ping_id, data.data, sizeof(received_ping_id));
    //             memcpy(&received_ping_time, data.data + 4, sizeof(received_ping_time));

    //             if (received_ping_id > last_received_ping_id) {
    //                 last_received_ping_id = received_ping_id;
    //                 last_received_ping_duration = now - received_ping_time;
    //                 last_received_ping_ts = millis();
    //             }
    //         }
    //     }
    // }

    lv_timer_handler();
}
