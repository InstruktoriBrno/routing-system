#pragma once

#include "lvgl.h"
#include "gui.hpp"

class MainScreen: public BaseScreen {
    lv_obj_t *milliseconds_label = nullptr;

    int counter = 0;
    lv_obj_t *counter_label = nullptr;
    lv_obj_t *counter_button = nullptr;

    lv_obj_t *card_id_label = nullptr;
    lv_obj_t *network_status_label = nullptr;

    void _update_millis() {
        char text_buffer[64];
        sprintf(text_buffer, "Milliseconds: %lu", millis());
        lv_label_set_text(milliseconds_label, text_buffer);
    }

    void _on_counter_button_click() {
        counter++;
        char text_buffer[64];
        sprintf(text_buffer, "Counter: %d", counter);
        lv_label_set_text(counter_label, text_buffer);
    }
public:
    MainScreen() {
        milliseconds_label = lv_label_create(screen);
        lv_obj_set_width(milliseconds_label, LV_SIZE_CONTENT);
        lv_obj_set_height(milliseconds_label, 20);
        lv_obj_set_y(milliseconds_label, 10);
        lv_label_set_text(milliseconds_label, "Milliseconds: ");

        counter_label = lv_label_create(screen);
        lv_obj_set_width(counter_label, LV_SIZE_CONTENT);
        lv_obj_set_height(counter_label, 20);
        lv_obj_set_y(counter_label, 30);
        lv_label_set_text(counter_label, "Counter: 0");

        card_id_label = lv_label_create(screen);
        lv_obj_set_width(card_id_label, LV_SIZE_CONTENT);
        lv_obj_set_height(card_id_label, 20);
        lv_obj_set_y(card_id_label, 50);
        lv_label_set_text(card_id_label, "Card ID: No card detected");

        network_status_label = lv_label_create(screen);
        lv_obj_set_width(network_status_label, LV_SIZE_CONTENT);
        lv_obj_set_height(network_status_label, 100);
        lv_obj_set_y(network_status_label, 70);
        lv_label_set_text(network_status_label, "Network status: Not connected");

        counter_button = lv_btn_create(screen);
        lv_obj_set_width(counter_button, LV_SIZE_CONTENT);
        lv_obj_set_height(counter_button, 50);
        lv_obj_set_y(counter_button, 140);

        lv_obj_t *label = lv_label_create(counter_button);
        lv_label_set_text(label, "Bump counter");
        lv_obj_set_align(label, LV_ALIGN_CENTER);
        lv_obj_set_width(label, LV_SIZE_CONTENT);
        lv_obj_set_height(label, LV_SIZE_CONTENT);

        lv_obj_add_event_cb(counter_button, [](lv_event_t *e) {
            auto self = static_cast<MainScreen*>(lv_event_get_user_data(e));
            self->_on_counter_button_click();
        }, LV_EVENT_CLICKED, this);
    }

    void update() {
        _update_millis();
    }

    void set_card_id(char const *id) {
        if (id == nullptr)
            lv_label_set_text(card_id_label, "Card ID: No card detected");
        else {
            char text_buffer[64];
            sprintf(text_buffer, "Card ID: %s", id);
            lv_label_set_text(card_id_label, text_buffer);
        }
    }

    template <typename... Args>
    void set_network_message(char const *message, Args... args) {
        char text_buffer[256];
        sprintf(text_buffer, message, args...);
        lv_label_set_text(network_status_label, text_buffer);
    }
};
