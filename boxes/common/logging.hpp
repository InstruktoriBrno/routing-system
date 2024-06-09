#pragma once

#include <cstring>
#include <cstdint>
#include <Arduino.h>

enum class Severity {
    DEBUG = 0,
    INFO,
    WARNING,
    ERROR
};

void rg_log_generic(const char* tag, Severity severity, uint32_t timestamp, const char* fmt, va_list args);
void rg_log_i(const char* tag, const char* fmt, ...);
void rg_log_e(const char* tag, const char* fmt, ...);
void rg_log_w(const char* tag, const char* fmt, ...);
void rg_log_d(const char* tag, const char* fmt, ...);

void rg_set_log_handler(void (*handler)(void *arg, const char* tag, uint32_t timestamp, Severity severity, const char* fmt, va_list args), void *arg);
void rg_set_log_severity(Severity severity);

void rg_serial_log_handler(void *arg, const char* tag, uint32_t timestamp, Severity severity, const char* fmt, va_list args);
