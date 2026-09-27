#include "../inc/logger.h"
#include <stdarg.h>

level_t log_level;

const char* prefixes[] = {
    RED_COLOR"[ERROR] ",
    YELLOW_COLOR"[WARNING] ",
    BLUE_COLOR"[DEBUG] ",
    GRAY_COLOR"[LOG] ",
};

void logger_set_level(const char* s_level) {
    switch (*s_level) {
        case 'e': log_level = ERROR; break;
        case 'w': log_level = WARNING; break;
        case 'd': log_level = DEBUG; break;
        case 'l': log_level = LOG; break;
        default: logger_err("logger: unknown logger_log level");
    }
}
level_t logger_get_level(void) {
    return log_level;
}

void logger_lprintf(level_t level, const char* format, ...) {
    if (level > log_level) return;
    printf("%s", prefixes[level]);
    
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
    
    printf(RESET_COLOR"\n");
}