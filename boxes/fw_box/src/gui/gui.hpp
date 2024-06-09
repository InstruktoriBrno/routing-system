#pragma once

#include "lvgl.h"
#include <memory>

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
