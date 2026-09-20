#pragma once

#include <iostream>

#define GRAY_COLOR "\x1b[90m"
#define BLUE_COLOR "\x1b[34m"
#define YELLOW_COLOR "\x1b[33m"
#define GREEN_COLOR "\x1b[32m"
#define RED_COLOR "\x1b[31m"
#define RESET_COLOR "\x1b[0m"
#define BOLD "\x1b[1m"

enum LogLevel{
    LOG     = 0,
    DEBUG   = 1,
    WARNING = 2,
    ERROR   = 3,
};

class Logger {
    std::ostream empty_stream{nullptr};
    LogLevel level = ERROR;
public:
    std::ostream& log();
    std::ostream& deb();
    std::ostream& warn();
    std::ostream& err();
    std::ostream& force();

    void set_level(const char* s_level);
    LogLevel get_level();
};

extern Logger logger;