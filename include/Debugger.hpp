#pragma once

class Debugger {
    bool enabled = false;
public:
    void enable();
    bool is_enabled();
    void step();
};

extern Debugger debugger;