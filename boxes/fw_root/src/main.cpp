#include "timer.hpp"
#include <Arduino.h>

#include <cstdint>
#include <optional>
#include <cstring>

#include <mesh_net.hpp>
#include <serdes.hpp>
#include <logging.hpp>
#include <timer.hpp>

static const char *TAG = "MAIN";

void log_handler(void *arg, const char* tag, uint32_t timestamp, Severity severity, const char* fmt, va_list args) {
    Serial.print("L:");
    rg_serial_log_handler(arg, tag, timestamp, severity, fmt, args);
}

void setup()
{
    Serial.begin(921600);
    rg_set_log_handler(log_handler, nullptr);

    rg_log_i(TAG, "Starting up...");
    initialize_mesh_network_as_root();
    rg_log_i(TAG, "Mesh network initialized");

    rg_log_i(TAG, "Free heap: %d", ESP.getFreeHeap());
    rg_log_i(TAG, "Here we go!");
}

class RootMessageHandler : public MessageHandler {
public:
    void operator()(MacAddress source, const BoxStatusMessage& msg) override {
        Serial.print("D:");
        Serial.print("BoxStatus");
        Serial.print(",");
        for (int i = 0; i < 6; i++) {
            Serial.print(source[i], HEX);
            if (i < 5)
                Serial.print(":");
        }
        Serial.print(",");
        for (int i = 0; i < 6; i++) {
            Serial.print(msg.parent[i], HEX);
            if (i < 5)
                Serial.print(":");
        }
        Serial.print(",");
        Serial.print(msg.network_depth);
        Serial.print(",");
        Serial.print(msg.active_round_id);
        Serial.print(",");
        Serial.println(msg.round_download_progress);
    }
};

PeriodicTimer timestamp_notification(1000);

void loop() {
    RootMessageHandler message_handler;
    handle_incoming_messages(message_handler);

    if (timestamp_notification.elapsed()) {
        Serial.print("T:");
        Serial.println(network_millis());
    }
}
