#pragma once

#include "lvgl.h"
#include <memory>
#include <cstdio>
#include <array>

void setup_gui();

class BaseScreen {
protected:
    lv_obj_t *screen = nullptr;
public:
    BaseScreen() {
        screen = lv_obj_create(nullptr);
        lv_obj_clear_flag(screen, LV_OBJ_FLAG_SCROLLABLE);
    }

    BaseScreen(BaseScreen const &) = delete;
    BaseScreen &operator=(BaseScreen const &) = delete;

    BaseScreen(BaseScreen && o) {
        screen = o.screen;
        o.screen = nullptr;
    }

    BaseScreen &operator=(BaseScreen && o) {
        if (screen != nullptr)
            lv_obj_del(screen);
        screen = o.screen;
        o.screen = nullptr;
        return *this;
    }

    ~BaseScreen() {
        if (screen != nullptr)
            lv_obj_del(screen);
    }


    void activate() {
        lv_disp_load_scr(screen);
    }
};

class BaseStatusBarScreen : public BaseScreen {
    lv_obj_t *status_bar = nullptr;
    lv_obj_t *status_dot = nullptr;
    lv_obj_t *mac_label = nullptr;

public:
    BaseStatusBarScreen() {
        // Create the status bar
        status_bar = lv_obj_create(screen);
        lv_obj_set_width(status_bar, LV_PCT(100));
        lv_obj_set_height(status_bar, 15);
        lv_obj_set_align(status_bar, LV_ALIGN_TOP_MID);
        lv_obj_set_style_bg_color(status_bar, lv_color_hex(0x333333), LV_PART_MAIN);
        lv_obj_set_style_bg_opa(status_bar, LV_OPA_COVER, LV_PART_MAIN);

        // Create the status dot
        status_dot = lv_obj_create(status_bar);
        lv_obj_set_size(status_dot, 10, 10); // Size of the dot
        lv_obj_set_style_radius(status_dot, LV_RADIUS_CIRCLE, LV_PART_MAIN);
        lv_obj_set_align(status_dot, LV_ALIGN_LEFT_MID);
        lv_obj_set_style_bg_color(status_dot, lv_color_hex(0xFF0000), LV_PART_MAIN); // Default to red (offline)
        lv_obj_set_style_bg_opa(status_dot, LV_OPA_COVER, LV_PART_MAIN);

        // Create the MAC address label
        mac_label = lv_label_create(status_bar);
        lv_obj_set_align(mac_label, LV_ALIGN_RIGHT_MID);
        lv_label_set_text(mac_label, "00:00:00:00:00:00");
        lv_obj_set_style_text_color(mac_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    }

    void set_online_status(bool online) {
        if (online) {
            lv_obj_set_style_bg_color(status_dot, lv_color_hex(0x00FF00), LV_PART_MAIN); // Green for online
        } else {
            lv_obj_set_style_bg_color(status_dot, lv_color_hex(0xFF0000), LV_PART_MAIN); // Red for offline
        }
    }

    void set_mac_address(std::array<uint8_t, 6> mac) {
        char text_buffer[64];
        sprintf(text_buffer, "%02X:%02X:%02X:%02X:%02X:%02X",
                mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
        lv_label_set_text(mac_label, text_buffer);
    }
};


class ServiceMenuScreen : public BaseStatusBarScreen {
    lv_obj_t *text_label = nullptr;

public:
    ServiceMenuScreen() {
        lv_obj_set_style_bg_color(screen, lv_color_hex(0x004ac2), LV_PART_MAIN);

        text_label = lv_label_create(screen);
        lv_obj_set_width(text_label, LV_SIZE_CONTENT);
        lv_obj_set_height(text_label, LV_SIZE_CONTENT);
        lv_obj_center(text_label);

        lv_label_set_text(text_label, "");
        lv_obj_set_style_text_color(text_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
        lv_obj_set_style_text_font(text_label, &lv_font_montserrat_22, LV_PART_MAIN);
    }

    void set_label(const char *text) {
        lv_label_set_text(text_label, text);
    }
};
