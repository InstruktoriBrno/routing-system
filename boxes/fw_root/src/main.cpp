#include "timer.hpp"
#include "mesh_net.hpp"
#include "serdes.hpp"
#include "logging.hpp"
#include <Arduino.h>

#include <cstdint>
#include <optional>
#include <cstring>
#include <memory>

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
    template <typename Msg>
    void forward_to_serial(const MacAddress& source, const Msg& msg) {
        auto encoder = Base64Encoder([](uint8_t byte) {
            Serial.write(byte);
        });

        for (auto b : source) {
            encoder.push_byte(b);
        }
        encoder.push(Msg::MESSAGE_TYPE);
        msg.serialize(encoder);
        encoder.finalize();
    }
public:
    void operator()(MacAddress source, const NodeStatusMessage& msg) override {
        Serial.print("D:");
        forward_to_serial(source, msg);
        Serial.print("\n");
    }
};

class SerialMessageHandler {
private:
    static constexpr int MAX_BUFFER_SIZE = 1500;
    std::unique_ptr<uint8_t[]> _buffer = std::make_unique<uint8_t[]>(MAX_BUFFER_SIZE);
    int _buffer_pos = 0;
    int _command = -1;

    void consume_line() {
        while (true) {
            while (!Serial.available())
                delay(1);
            auto c = Serial.read();
            if (c == '\n') {
                return;
            }
        }
    }

    bool read_base64() {
        auto decoder = Base64Decoder([&](uint8_t byte) {
            _buffer[_buffer_pos++] = byte;
        });

        while (true) {
            while (!Serial.available())
                delay(1);
            auto c = Serial.read();
            if (c == '\n') {
                decoder.finalize();
                return true;
            }

            if (_buffer_pos >= MAX_BUFFER_SIZE) {
                rg_log_e(TAG, "Buffer overflow");
                consume_line();
                return false;
            }
            decoder.push_byte(c);
        }
    }
public:
    SerialMessageHandler() = default;

    void run() {
        if (Serial.available() < 2) {
            return;
        }

        char command = Serial.read();
        _buffer_pos = 0;
        if (command == 'B') {
            Serial.read();
            if (!read_base64())
                return;
            broadcast_raw_message(tcb::span<uint8_t>(_buffer.get(), _buffer_pos));
            return;
        }
    }
};

PeriodicTimer timestamp_notification(1000);
SerialMessageHandler serial_message_handler;

void loop() {
    serial_message_handler.run();

    RootMessageHandler message_handler;
    handle_incoming_messages(message_handler);

    if (timestamp_notification.elapsed()) {
        Serial.print("T:");
        Serial.println(network_millis());
    }
}
