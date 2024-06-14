#pragma once

#include "gui.hpp"

LV_FONT_DECLARE(roboto_bold_120);
LV_FONT_DECLARE(roboto_bold_180);
LV_FONT_DECLARE(roboto_bold_80);

class GameScreen : public BaseStatusBarScreen {
public:
    virtual void set_game_time(int secs) {}
    virtual void set_identity(char c) {}
};

class UpdateScreen : public GameScreen {
    lv_obj_t *text_label = nullptr;

public:
    UpdateScreen() {
        lv_obj_set_style_bg_color(screen, lv_color_hex(0x000000), LV_PART_MAIN);

        text_label = lv_label_create(screen);
        lv_obj_set_width(text_label, LV_SIZE_CONTENT);
        lv_obj_set_height(text_label, LV_SIZE_CONTENT);
        lv_obj_center(text_label);

        lv_label_set_text(text_label, "Hra se\naktualizuje...");
        lv_obj_set_style_text_color(text_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
        lv_obj_set_style_text_font(text_label, &lv_font_montserrat_44, LV_PART_MAIN);
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

        lv_label_set_text(text_label, "The Routing\nGame");
        lv_obj_set_style_text_align(text_label, LV_TEXT_ALIGN_CENTER, 0);
        lv_obj_set_style_text_color(text_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
        lv_obj_set_style_text_font(text_label, &lv_font_montserrat_48, LV_PART_MAIN);
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

        lv_label_set_text(text_label, "Hra za chvili\nzacne");
        lv_obj_set_style_text_align(text_label, LV_TEXT_ALIGN_CENTER, 0);
        lv_obj_set_style_text_color(text_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
        lv_obj_set_style_text_font(text_label, &lv_font_montserrat_48, LV_PART_MAIN);
    }
};

class RunningGameScreen : public GameScreen {
    lv_obj_t *text_label = nullptr;

    bool identity_sane = false;
    char identity[2] = {0};
    int game_time = 0;

    void update_label() {
        static char buffer[64] = {0};
        int minutes = game_time / 60;
        int seconds = game_time % 60;

        snprintf(buffer, 32, "%s\n%02d:%02d", identity_sane ? identity : "Unknown", minutes, seconds);
        lv_label_set_text(text_label, buffer);
        lv_obj_set_style_text_align(text_label, LV_TEXT_ALIGN_CENTER, 0);
    }
public:
    RunningGameScreen() {
        lv_obj_set_style_bg_color(screen, lv_color_hex(0x000000), LV_PART_MAIN);

        text_label = lv_label_create(screen);
        lv_obj_set_width(text_label, LV_SIZE_CONTENT);
        lv_obj_set_height(text_label, LV_SIZE_CONTENT);
        lv_obj_center(text_label);

        lv_label_set_text(text_label, "");
        lv_obj_set_style_text_color(text_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
        lv_obj_set_style_text_font(text_label, &roboto_bold_80, LV_PART_MAIN);
    }

    void set_game_time(int secs) override {
        game_time = secs;
        update_label();
    }

    void set_identity(char c) override {
        identity_sane = (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
        identity[0] = c;
        update_label();
    }
};

class PacketFailScreen : public GameScreen {
    lv_obj_t *text_label = nullptr;

public:
    PacketFailScreen() {
        lv_obj_set_style_bg_color(screen, lv_color_hex(0xFF0000), LV_PART_MAIN);

        text_label = lv_label_create(screen);
        lv_obj_set_width(text_label, LV_SIZE_CONTENT);
        lv_obj_set_height(text_label, LV_SIZE_CONTENT);
        lv_obj_center(text_label);

        lv_label_set_text(text_label, "Neplatna\noperace");
        lv_obj_set_style_text_align(text_label, LV_TEXT_ALIGN_CENTER, 0);
        lv_obj_set_style_text_color(text_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
        lv_obj_set_style_text_font(text_label, &lv_font_montserrat_48, LV_PART_MAIN);
    }
};

class PacketContinueNoPoints: public GameScreen {
    lv_obj_t *text_label = nullptr;
public:
    PacketContinueNoPoints(const char *target) {
        lv_obj_set_style_bg_color(screen, lv_color_hex(0x00FF00), LV_PART_MAIN);

        text_label = lv_label_create(screen);
        lv_obj_set_width(text_label, LV_SIZE_CONTENT);
        lv_obj_set_height(text_label, LV_SIZE_CONTENT);
        lv_obj_center(text_label);

        lv_label_set_text(text_label, target);
        lv_obj_set_style_text_color(text_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
        lv_obj_set_style_text_font(text_label, &roboto_bold_180, LV_PART_MAIN);
    }
};

class PacketContinueWithPoints: public GameScreen {
    lv_obj_t *text_label = nullptr;
    lv_obj_t *points_label = nullptr;
    lv_obj_t *points_container = nullptr;
public:
    PacketContinueWithPoints(const char* target, int points) {
        lv_obj_set_style_bg_color(screen, lv_color_hex(0x00FF00), LV_PART_MAIN);

        text_label = lv_label_create(screen);
        lv_obj_set_width(text_label, 180);
        lv_obj_set_x(text_label, 30);
        lv_obj_set_y(text_label, 30);
        lv_obj_set_height(text_label, LV_SIZE_CONTENT);

        lv_label_set_text(text_label, target);
        lv_obj_set_style_text_color(text_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
        lv_obj_set_style_text_font(text_label, &roboto_bold_120, LV_PART_MAIN);

        points_container = lv_obj_create(screen);
        lv_obj_set_size(points_container, 150, 80); // Adjust the size as needed
        lv_obj_set_x(points_container, 320 - 150);
        lv_obj_set_y(points_container, 240 - 80);
        lv_obj_set_style_bg_color(points_container, lv_color_hex(0xFFA500), LV_PART_MAIN);

        lv_obj_set_style_pad_all(points_container, 10, LV_PART_MAIN);

        points_label = lv_label_create(points_container);
        lv_obj_set_width(points_label, LV_SIZE_CONTENT);
        lv_obj_set_height(points_label, LV_SIZE_CONTENT);
        lv_obj_center(points_label);


        char points_buffer[4] = {0};
        snprintf(points_buffer, 4, "%d", points);
        lv_label_set_text(points_label, points_buffer);
        lv_obj_set_style_text_color(points_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
        lv_obj_set_style_text_font(points_label, &roboto_bold_80, LV_PART_MAIN);
    }
};


class PacketFinishNoPoints: public GameScreen {
public:
    PacketFinishNoPoints() {
        lv_obj_set_style_bg_color(screen, lv_color_hex(0xFFA500), LV_PART_MAIN);
    }
};

class PacketFinishWithPoints: public GameScreen {
    lv_obj_t *points_label = nullptr;
public:
    PacketFinishWithPoints(int points) {
        lv_obj_set_style_bg_color(screen, lv_color_hex(0xFFA500), LV_PART_MAIN);

        points_label = lv_label_create(screen);
        lv_obj_set_width(points_label, LV_SIZE_CONTENT);
        lv_obj_set_height(points_label, LV_SIZE_CONTENT);
        lv_obj_center(points_label);

        char points_buffer[4] = {0};
        snprintf(points_buffer, 4, "%d", points);
        lv_label_set_text(points_label, points_buffer);
        lv_obj_set_style_text_color(points_label, lv_color_hex(0x000000), LV_PART_MAIN);
        lv_obj_set_style_text_font(points_label, &roboto_bold_180, LV_PART_MAIN);
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

        lv_label_set_text(text_label, "Pauza");
        lv_obj_set_style_text_color(text_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
        lv_obj_set_style_text_font(text_label, &lv_font_montserrat_48, LV_PART_MAIN);
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

        lv_label_set_text(text_label, "Kolo\nskoncilo");
        lv_obj_set_style_text_align(text_label, LV_TEXT_ALIGN_CENTER, 0);
        lv_obj_set_style_text_color(text_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
        lv_obj_set_style_text_font(text_label, &lv_font_montserrat_48, LV_PART_MAIN);
    }
};
