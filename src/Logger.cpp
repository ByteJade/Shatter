#include "../include/Logger.hpp"

Logger logger;

std::ostream& Logger::log() {
    if (level > LOG) return empty_stream;
    return std::cout << GRAY_COLOR"[LOG] ";
}
std::ostream& Logger::deb() {
    if (level > DEBUG) return empty_stream;
    return std::cout << BLUE_COLOR"[DEB] ";
}
std::ostream& Logger::warn() {
    if (level > WARNING) return empty_stream;
    return std::cout << BOLD YELLOW_COLOR"[WARN] ";
}
std::ostream& Logger::err() {
    return std::cout << BOLD RED_COLOR"[ERROR] ";
}
std::ostream& Logger::force() {
    return std::cout << RESET_COLOR;
}

void Logger::set_level(const char* s_level) {
    switch (s_level[0]) {
        case 'l': level = LOG; break;
        case 'd': level = DEBUG; break;
        case 'w': level = WARNING; break;
        case 'e': level = ERROR; break;
        default: err() << "Unknown log level" << std::endl;
    }
}
LogLevel Logger::get_level() {
    return level;
}