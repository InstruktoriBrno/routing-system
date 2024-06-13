#pragma once

#include "lvgl.h"
#include "gui.hpp"

class RedScreen : public BaseScreen {
    lv_obj_t *error_label = nullptr;

public:
    RedScreen(const char *error_message) {
        // Set the screen's background color to red
        lv_obj_set_style_bg_color(screen, lv_color_hex(0xFF0000), LV_PART_MAIN);

        // Create the error label
        error_label = lv_label_create(screen);
        lv_obj_set_width(error_label, LV_SIZE_CONTENT);
        lv_obj_set_height(error_label, LV_SIZE_CONTENT);
        lv_obj_center(error_label);

        // Set the error message text and style
        lv_label_set_text(error_label, error_message);
        lv_obj_set_style_text_color(error_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
        lv_obj_set_style_text_font(error_label, &lv_font_montserrat_22, LV_PART_MAIN);
    }
};
