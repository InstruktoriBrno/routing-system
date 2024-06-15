#pragma once

#include "cards.hpp"
#include "audio.hpp"
#include "routing_game.hpp"
#include <string>

class ServiceInterface {
    bool _activated = false;
    bool _busy = false;
    String _command;
    String _display_message;

    CardReader &_card_reader;

    void _command_finished() {
        _command = "";
         _display_message = "";
        _busy = false;
    }

    void _command_update() {
        if (_command.startsWith("activate")) {
            _activated = true;
            _command_finished();
            Serial.println("OK");
            return;
        }

        if (!_activated) {
            Serial.println("NOK");
            _command_finished();
            return;
        }

        if ((Serial.available() > 0 && Serial.peek() == 'q') || _command == "q") {
            if (Serial.peek() == 'q')
                Serial.read();
            _command_finished();
            return;
        }

        if (_command.startsWith("deactivate")) {
            _activated = false;
            _command_finished();
            Serial.println("OK");
        } else if (_command.startsWith("read_card")) {
            _display_message = "Cekam na prilozeni karty";
            if (_card_reader.has_new_card()) {
                delay(300); // Wait for the card to be fully inserted
                _display_message = "Karta prectena";
                auto game_interface = _card_reader.game_card_interface();
                Serial.print("phys_id:");
                for (uint8_t byte : game_interface.get_physical_id()) {
                    Serial.print(byte, HEX);
                }
                Serial.println();

                Serial.print("log_id:");
                Serial.print(game_interface.get_id().team_id);
                Serial.print(":");
                Serial.println(game_interface.get_id().seq);

                Serial.print("round:");
                Serial.println(game_interface.get_round_id());

                Serial.print("metadata:");
                Serial.println(game_interface.get_metadata().to_ulong());

                for (int i = 0; i < game_interface.visit_count(); i++) {
                    auto visit = game_interface.get_visit(i);
                    Serial.print("visit:");
                    // { "time": 15, "card": "A015", "bearer": "fe:d3:4c:aa:72:11:23", "roundId": 10, "routerId": "A", points: 10 }
                    Serial.print("{\"time\": ");
                    Serial.print(visit.time);
                    Serial.print(", \"card\": \"");
                    Serial.print(char(game_interface.get_id().team_id));
                    if (game_interface.get_id().seq < 100) {
                        Serial.print("0");
                    }
                    if (game_interface.get_id().seq < 10) {
                        Serial.print("0");
                    }
                    Serial.print(int(game_interface.get_id().seq));
                    Serial.print("\", \"bearer\": \"");
                    for (uint8_t byte : game_interface.get_physical_id()) {
                        Serial.print(byte, HEX);
                    }
                    Serial.print("\", \"roundId\": ");
                    Serial.print(game_interface.get_round_id());
                    Serial.print(", \"routerId\": \"");
                    Serial.print(char(visit.where));
                    Serial.print("\", \"points\": ");
                    Serial.print(visit.points);
                    Serial.println("}");
                }
                Serial.println("OK");
                play_wav("/beep.wav");
                _command_finished();
                delay(500);
            }
        } else if (_command.startsWith("write_card")) {
            _display_message = "Cekam na prilozeni karty";

            int first_colon = _command.indexOf(':');
            int second_colon = _command.indexOf(':', first_colon + 1);

            String team_id_str = _command.substring(first_colon + 1, second_colon);
            String card_id_str = _command.substring(second_colon + 1);

            int team_id = team_id_str.toInt();
            int card_id = card_id_str.toInt();

            if (_card_reader.has_new_card()) {
                delay(300); // Wait for the card to be fully inserted
                auto game_interface = _card_reader.game_card_interface();
                game_interface.write_logic_id({.team_id = rg::TeamId(team_id), .seq = rg::TeamId(card_id)});
                game_interface.reset_for_round(0);
                game_interface.finish_transaction();

                _display_message = "Karta zapsana";
                play_wav("/beep.wav");

                Serial.println("OK");
                _command_finished();
                delay(500);
            }
        }
        else {
            Serial.println("NOK");
            _command_finished();
        }
    }

    void _read_command() {
        while (Serial.available() > 0) {
            char c = Serial.read();
            if (c == '\n') {
                _busy = true;
                return;
            }
            _command += c;
        }
    }
public:
    ServiceInterface(CardReader &card_reader)
        : _card_reader(card_reader) {}

    bool is_active() const {
        return _activated;
    }

    const char *display_message() const {
        return _display_message.c_str();
    }

    void update() {
        if (!_busy) {
            _read_command();
        }
        if (_busy) {
            _command_update();
        }
    }
};
