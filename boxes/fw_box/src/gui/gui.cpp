#include "lvgl.h"

void setup_gui() {
    lv_disp_t * dispp = lv_disp_get_default();
    lv_disp_set_rotation(dispp, LV_DISP_ROT_270);
    lv_theme_t * theme = lv_theme_default_init(dispp, lv_palette_main(LV_PALETTE_BLUE), lv_palette_main(LV_PALETTE_RED),
                                               false, LV_FONT_DEFAULT);
    lv_disp_set_theme(dispp, theme);
}
