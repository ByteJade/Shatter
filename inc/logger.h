#ifndef LOGGER_H
#define LOGGER_H

#include <stdlib.h>
#include <stdio.h>

typedef enum {
    ERROR,
    WARNING,
    DEBUG,
    LOG,
} level_t;

#define GRAY_COLOR "\x1b[90m"
#define BLUE_COLOR "\x1b[34m"
#define YELLOW_COLOR "\x1b[33m"
#define GREEN_COLOR "\x1b[32m"
#define RED_COLOR "\x1b[31m"
#define BOLD "\x1b[1m"
#define RESET_COLOR "\x1b[0m"

void logger_set_level(const char* s_level);
level_t logger_get_level(void);
void logger_lprintf(level_t level, const char* format, ...);

#define logger_err(...)  logger_lprintf(ERROR, __VA_ARGS__)
#define logger_warn(...) logger_lprintf(WARNING, __VA_ARGS__)
#define logger_deb(...)  logger_lprintf(DEBUG, __VA_ARGS__)
#define logger_log(...)  logger_lprintf(LOG, __VA_ARGS__)

#endif