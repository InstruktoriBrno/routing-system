#include "audio.hpp"
#include <Arduino.h>

#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>
#include <cstring>

#include <esp32_smartdisplay.h>
#include <gui/gui.hpp>
#include <gui/main_screen.hpp>
#include <gui/game.hpp>
#include <cards.hpp>
#include <mesh_net.hpp>

#include <SPIFFS.h>

#include <audio.hpp>
#include "mesh_net.hpp"
#include "timer.hpp"
#include "logging.hpp"
#include "sys.hpp"
#include "game.hpp"
#include "service_interface.hpp"

static const char *TAG = "main";

std::unique_ptr<MainScreen> main_screen;

SET_LOOP_TASK_STACK_SIZE(10 * 1024);

CardReader card_reader({
    .miso = 19, // 35,
    .mosi = 18, // 22,
    .sck = 23, // 21,
    .ss = 5 // 27
});

void setup()
{
    Serial.begin(115200);

    rg_set_log_handler(rg_serial_log_handler, nullptr);
    rg_set_log_severity(Severity::DEBUG);
    rg::set_log_sink([](const char* fmt, va_list args) {
        static char buffer[512];
        vsnprintf(buffer, sizeof(buffer), fmt, args);
        Serial.println(buffer);
    });

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
    Game& game;
    GameUpdater& game_updater;
public:
    BoxMessageHandler(Game& game, GameUpdater& game_updater): game(game), game_updater(game_updater) {}

    void operator()(MacAddress source, const NodeStatusMessage& msg) override {
        rg_log_i(TAG, "Received NodeStatusMessage from %02x:%02x:%02x:%02x:%02x:%02x",
            source[0], source[1], source[2], source[3], source[4], source[5]);
    };

    void operator()(MacAddress source, const RoundHeaderMessage& msg) override {
        rg_log_i(TAG, "Received RoundHeaderMessage from %02x:%02x:%02x:%02x:%02x:%02x",
            source[0], source[1], source[2], source[3], source[4], source[5]);
        if (msg.round_id != game_updater.round_id() || msg.hash != game_updater.round_hash()) {
            rg_log_i(TAG, "Received new round header, updating game");
            game_updater.on_round_header(msg.round_id, msg.hash, msg.duration);
        }
    }

    void operator()(MacAddress source, const RouterDefinitionMessage& msg) override {
        rg_log_i(TAG, "Received RouterDefinitionMessage from %02x:%02x:%02x:%02x:%02x:%02x",
            source[0], source[1], source[2], source[3], source[4], source[5]);
        for (auto byte : msg.data) {
            Serial.print(byte, HEX);
        }
        Serial.println();
        for (auto byte : msg.data) {
            if (byte == 0)
                Serial.print("\\00");
            else
                Serial.print(char(byte));
        }
        Serial.println();
        if (msg.round_id == game_updater.round_id() && msg.hash == game_updater.round_hash()) {
            game_updater.on_router_definition_pack(msg.total_count, msg.initial_idx, msg.final_idx, msg.data);
        }
    }

    void operator()(MacAddress source, const LinkDefinitionMessage& msg) override {
        rg_log_i(TAG, "Received LinkDefinitionMessage from %02x:%02x:%02x:%02x:%02x:%02x",
            source[0], source[1], source[2], source[3], source[4], source[5]);
        if (msg.round_id == game_updater.round_id() && msg.hash == game_updater.round_hash()) {
            game_updater.on_link_definition_pack(msg.total_count, msg.initial_idx, msg.final_idx, msg.data);
        }
    }

    void operator()(MacAddress source, const PacketDefinitionMessage& msg) override {
        rg_log_i(TAG, "Received PacketDefinitionMessage from %02x:%02x:%02x:%02x:%02x:%02x",
            source[0], source[1], source[2], source[3], source[4], source[5]);
        if (msg.round_id == game_updater.round_id() && msg.hash == game_updater.round_hash()) {
            game_updater.on_packet_definition_pack(msg.total_count, msg.initial_idx, msg.final_idx, msg.data);
        }
    }

    void operator()(MacAddress source, const EventDefinitionMessage& msg) override {
        rg_log_i(TAG, "Received EventDefinitionMessage from %02x:%02x:%02x:%02x:%02x:%02x",
            source[0], source[1], source[2], source[3], source[4], source[5]);
        if (msg.round_id == game_updater.round_id() && msg.hash == game_updater.round_hash()) {
            game_updater.on_topology_event_pack(msg.total_count, msg.initial_idx, msg.final_idx, msg.data);
        }
    }

    void operator()(MacAddress source, const GameStateMessage& msg) override {
        rg_log_i(TAG, "Received PrepareGameMessage from %02x:%02x:%02x:%02x:%02x:%02x",
            source[0], source[1], source[2], source[3], source[4], source[5]);
        switch (GameState(msg.state)) {
            case GameState::NotRunning:
                game.stop_game();
                break;
            case GameState::Preparation:
                game.prepare_game();
                break;
            case GameState::Running:
                game.start_game(msg.time_offset);
                break;
            case GameState::Paused:
                game.pause_game();
                break;
            default:
                rg_log_e(TAG, "Unknown game state %d", msg.state);
                break;
        }
    }
};

PeriodicTimer status_timer(1000);
PeriodicTimer screen_timer(50);
PeriodicTimer card_clear_timer(2000);
OneShotTimer packet_screen_clear_timer(2000);

Game game;
GameUpdater game_updater;
BoxMessageHandler msg_handler(game, game_updater);

ServiceInterface service_interface(card_reader);

void game_step() {
    static std::unique_ptr<GameScreen> packet_screen;

    if (game_updater.is_update_in_progress()) {
        static UpdateScreen update_screen;

        update_screen.set_progress(game_updater.update_percents());
        update_screen.set_online_status(is_mesh_connected());
        update_screen.set_mac_address(my_mac_address());
        update_screen.activate();

        return;
    }

    if (game.has_old_round_setup(game_updater.round_hash())) {
        game.update_round_setup(game_updater.build_round_setup(), game_updater.round_hash());
        rg_log_i("MAIN", "Who am I: %d", game.who_am_i());
    }

    game.update(network_millis());

    GameScreen *activated_game_screen = nullptr;
    if (game.state() == GameState::NotRunning) {
        static NotRunningGameScreen not_running_screen;
        activated_game_screen = &not_running_screen;
    }
    if (game.state() == GameState::Preparation) {
        static PrepareGameScreen prepare_screen;
        activated_game_screen = &prepare_screen;
    }
    if (game.state() == GameState::Running) {
        static RunningGameScreen running_screen;

        if (card_reader.has_new_card()) {
            auto card_interface = card_reader.game_card_interface();
            if (card_interface.is_correctly_initialized()) {
                rg_log_i(TAG, "Card round ID %d, game round ID %d", card_interface.get_round_id(), game_updater.round_id());
                if (card_interface.get_round_id() != game_updater.round_id()) {
                    // There is a card from another round, clear it
                    rg_log_i(TAG, "Card from another round detected, clearing");
                    card_interface.reset_for_round(game_updater.round_id());
                }

                auto result = game.handle_packet_visit(card_interface);
                card_interface.finish_transaction();

                rg_log_i(TAG, "Game logic log: %s", result.log.c_str());
                rg_log_i(TAG, "Game logic result: %d", result.result);
                rg_log_i(TAG, "Was visited: %d", card_interface.had_new_visit());

                auto visit_time = game.game_time();
                auto visit_where = game.who_am_i();
                std::optional<uint8_t> visit_points = std::nullopt;

                if (card_interface.had_new_visit()) {
                    auto visit = card_interface.get_new_visit();
                    rg_log_i(TAG, "Sending log to the network");
                    // Log the information written to the card
                    visit_time = visit.time;
                    visit_where = visit.where;
                    visit_points = visit.points;
                }

                for (int i = 0; i != 4; i++) {
                    bool result = send_packet_visit(
                        visit_time,
                        card_interface.get_physical_id(),
                        card_interface.get_id().team_id,
                        card_interface.get_id().seq,
                        visit_where,
                        visit_points);
                    rg_log_i(TAG, "Send result: %d", result);
                    if (result)
                        break;
                }

                std::unique_ptr<GameScreen> new_packet_screen;
                if (result.result == rg::PacketVisitResult::Finished) {
                    play_wav("/finished.wav");
                    if (result.points) {
                        new_packet_screen = std::make_unique<PacketFinishWithPoints>(*result.points);
                    } else {
                        new_packet_screen = std::make_unique<PacketFinishNoPoints>();

                    }
                }
                if (result.result == rg::PacketVisitResult::Continue) {
                    play_wav("/accept.wav");
                    if (result.points) {
                        new_packet_screen = std::make_unique<PacketContinueWithPoints>((*result.instructions).c_str(), *result.points);
                    } else {
                        new_packet_screen = std::make_unique<PacketContinueNoPoints>((*result.instructions).c_str());
                    }
                }
                if (result.result == rg::PacketVisitResult::Invalid) {
                    play_wav("/error.wav");
                    new_packet_screen = std::make_unique<PacketFailScreen>();
                }

                new_packet_screen->activate();
                packet_screen = std::move(new_packet_screen);
                packet_screen_clear_timer.arm();
            }
        }

        activated_game_screen = &running_screen;
    }
    if (game.state() == GameState::Paused) {
        static PausedGameScreen paused_screen;
        activated_game_screen = &paused_screen;
    }
    if (game.state() == GameState::Finished) {
        static FinishedGameScreen finished_screen;
        activated_game_screen = &finished_screen;
    }

    if (packet_screen_clear_timer.elapsed()) {
        activated_game_screen->activate();
        packet_screen = nullptr;
        rg_log_i(TAG, "Clearing screen ");
    }
    if (packet_screen) {
        activated_game_screen = packet_screen.get();
    }

    activated_game_screen->set_online_status(is_mesh_connected());
    activated_game_screen->set_mac_address(my_mac_address());
    activated_game_screen->set_game_time(game.game_time());
    activated_game_screen->set_identity(game.who_am_i());
    activated_game_screen->activate();
}

void loop() {
    delay(1);
    lv_timer_handler();

    service_interface.update();
    if (service_interface.is_active()) {
        static ServiceMenuScreen service_menu_screen;
        service_menu_screen.set_label(service_interface.display_message());
        service_menu_screen.set_online_status(is_mesh_connected());
        service_menu_screen.set_mac_address(my_mac_address());
        service_menu_screen.activate();
        return;
    }

    handle_incoming_messages(msg_handler);

    if (status_timer.elapsed()) {
        report_box_status(game_updater.round_id(), game_updater.round_hash(),
                game_updater.update_percents(), uint8_t(game.state()), game.game_time(),
                game_updater.who_am_i());
    }

    game_step();
}

