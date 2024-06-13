#include "sys.hpp"
#include "gui/red_screen.hpp"
#include <Arduino.h>

void system_trap(const char* reason) {
    RedScreen red_screen(reason);
    red_screen.activate();
    while (true) {
        lv_timer_handler();
        delay(5);
    }
}
