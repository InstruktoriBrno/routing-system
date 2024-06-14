#pragma once

#include "gui.hpp"

class GameScreen : public BaseStatusBarScreen {};

class UpdateScreen : public GameScreen {
    lv_obj_t *text_label = nullptr;

public:
    UpdateScreen() {
        lv_obj_set_style_bg_color(screen, lv_color_hex(0x000000), LV_PART_MAIN);

        text_label = lv_label_create(screen);
        lv_obj_set_width(text_label, LV_SIZE_CONTENT);
        lv_obj_set_height(text_label, LV_SIZE_CONTENT);
        lv_obj_center(text_label);

        lv_label_set_text(text_label, "Hra se aktualizuje...");
        lv_obj_set_style_text_color(text_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
        lv_obj_set_style_text_font(text_label, &lv_font_montserrat_22, LV_PART_MAIN);
    }

    void set_progress(int progress) {
        // TBA
    }
};

class NotRunningGameScreen : public GameScreen {
    lv_obj_t *text_label = nullptr;

public:
    NotRunningGameScreen() {
        lv_obj_set_style_bg_color(screen, lv_color_hex(0x000000), LV_PART_MAIN);

        text_label = lv_label_create(screen);
        lv_obj_set_width(text_label, LV_SIZE_CONTENT);
        lv_obj_set_height(text_label, LV_SIZE_CONTENT);
        lv_obj_center(text_label);

        lv_label_set_text(text_label, "Hra neni pripravena");
        lv_obj_set_style_text_color(text_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
        lv_obj_set_style_text_font(text_label, &lv_font_montserrat_22, LV_PART_MAIN);
    }
};

class PrepareGameScreen : public GameScreen {
    lv_obj_t *text_label = nullptr;

public:
    PrepareGameScreen() {
        lv_obj_set_style_bg_color(screen, lv_color_hex(0x000000), LV_PART_MAIN);

        text_label = lv_label_create(screen);
        lv_obj_set_width(text_label, LV_SIZE_CONTENT);
        lv_obj_set_height(text_label, LV_SIZE_CONTENT);
        lv_obj_center(text_label);

        lv_label_set_text(text_label, "Kolo je pripraveno");
        lv_obj_set_style_text_color(text_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
        lv_obj_set_style_text_font(text_label, &lv_font_montserrat_22, LV_PART_MAIN);
    }
};

class RunningGameScreen : public GameScreen {
    lv_obj_t *text_label = nullptr;

public:
    RunningGameScreen() {
        lv_obj_set_style_bg_color(screen, lv_color_hex(0x000000), LV_PART_MAIN);

        text_label = lv_label_create(screen);
        lv_obj_set_width(text_label, LV_SIZE_CONTENT);
        lv_obj_set_height(text_label, LV_SIZE_CONTENT);
        lv_obj_center(text_label);

        lv_label_set_text(text_label, "Kolo bezi");
        lv_obj_set_style_text_color(text_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
        lv_obj_set_style_text_font(text_label, &lv_font_montserrat_22, LV_PART_MAIN);
    }
};

class PausedGameScreen : public GameScreen {
    lv_obj_t *text_label = nullptr;

public:
    PausedGameScreen() {
        lv_obj_set_style_bg_color(screen, lv_color_hex(0x000000), LV_PART_MAIN);

        text_label = lv_label_create(screen);
        lv_obj_set_width(text_label, LV_SIZE_CONTENT);
        lv_obj_set_height(text_label, LV_SIZE_CONTENT);
        lv_obj_center(text_label);

        lv_label_set_text(text_label, "Kolo je zapauzovano");
        lv_obj_set_style_text_color(text_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
        lv_obj_set_style_text_font(text_label, &lv_font_montserrat_22, LV_PART_MAIN);
    }
};

class FinishedGameScreen : public GameScreen {
    lv_obj_t *text_label = nullptr;

public:
    FinishedGameScreen() {
        lv_obj_set_style_bg_color(screen, lv_color_hex(0x000000), LV_PART_MAIN);

        text_label = lv_label_create(screen);
        lv_obj_set_width(text_label, LV_SIZE_CONTENT);
        lv_obj_set_height(text_label, LV_SIZE_CONTENT);
        lv_obj_center(text_label);

        lv_label_set_text(text_label, "Kolo skoncilo");
        lv_obj_set_style_text_color(text_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
        lv_obj_set_style_text_font(text_label, &lv_font_montserrat_22, LV_PART_MAIN);
    }
};
