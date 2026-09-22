#pragma once

#include <mutex>

class Debugger {
    bool enabled = false;
    std::mutex mut;
public:
    void usage();
    void enable();
    bool is_enabled();
    void step(class Handler& handler);
};

extern Debugger debugger;