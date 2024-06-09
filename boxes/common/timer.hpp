#pragma once

#include <Arduino.h>

class PeriodicTimer {
    uint32_t interval;
    uint32_t next_time;
public:
    PeriodicTimer(uint32_t interval) : interval(interval), next_time(millis() + interval) {}

    void reset() {
        next_time = millis() + interval;
    }

    operator bool() {
        return elapsed();
    }

    bool elapsed() {
        bool triggered = false;
        while (int32_t(next_time - millis()) <= 0){
            next_time += interval;
            triggered = true;
        }
        return triggered;
    }
};
