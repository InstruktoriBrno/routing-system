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

class OneShotTimer {
    uint32_t timeout;
    uint32_t start_time;
    bool armed;
public:
    OneShotTimer(uint32_t timeout) : timeout(timeout), start_time(millis()), armed(false) {}

    void arm() {
        start_time = millis();
        armed = true;
    }

    operator bool() {
        return elapsed();
    }

    bool elapsed() {
        if (!armed) return false;
        if (int32_t(millis() - start_time) >= timeout) {
            armed = false;
            return true;
        }
        return false;
    }
};
