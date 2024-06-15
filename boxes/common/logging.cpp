#include "logging.hpp"

static void (*log_handler)(void *arg, const char* tag, uint32_t timestamp, Severity severity, const char* fmt, va_list args) = nullptr;
static void *log_handler_arg = nullptr;
static Severity log_severity = Severity::INFO;

void rg_set_log_handler(void (*handler)(void *arg, const char* tag, uint32_t timestamp, Severity severity, const char* fmt, va_list args), void *arg) {
    log_handler = handler;
    log_handler_arg = arg;
}

void rg_set_log_severity(Severity severity) {
    log_severity = severity;
}

void rg_log_generic(uint32_t timestamp, Severity severity, const char* tag, const char* fmt, va_list args) {
    if (severity < log_severity) {
        return;
    }

    if (log_handler) {
        log_handler(log_handler_arg, tag, timestamp, severity, fmt, args);
    }
}

void rg_log_i(const char* tag, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    rg_log_generic(millis(), Severity::INFO, tag, fmt, args);
    va_end(args);
}

void rg_log_e(const char* tag, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    rg_log_generic(millis(), Severity::ERROR, tag, fmt, args);
    va_end(args);
}

void rg_log_w(const char* tag, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    rg_log_generic(millis(), Severity::WARNING, tag, fmt, args);
    va_end(args);
}

void rg_log_d(const char* tag, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    rg_log_generic(millis(), Severity::DEBUG, tag, fmt, args);
    va_end(args);
}

static const int TAG_WIDTH = 10;
static const int TIME_WIDTH = 10;
static const int SEVERITY_WIDTH = 7;

static const char* severity_to_string(Severity severity) {
    switch (severity) {
        case Severity::DEBUG: return "DEBUG";
        case Severity::INFO: return "INFO";
        case Severity::WARNING: return "WARN";
        case Severity::ERROR: return "ERROR";
        default: return "UNKNOWN";
    }
}

static void print_column(const char* str, int width) {
    int len = strlen(str);
    Serial.print(str);
    for (int i = len; i < width; i++) {
        Serial.print(' ');
    }
}

void rg_serial_log_handler(void *arg, const char* tag, uint32_t timestamp, Severity severity, const char* fmt, va_list args) {
    print_column(tag, TAG_WIDTH);

    char time_str[TIME_WIDTH];
    snprintf(time_str, sizeof(time_str), "%" PRIu32, timestamp);
    print_column(time_str, TIME_WIDTH);

    print_column(severity_to_string(severity), SEVERITY_WIDTH);

    static char message[512];
    vsnprintf(message, sizeof(message), fmt, args);
    Serial.println(message);
}
