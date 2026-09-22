#pragma once

#include <mutex>

class Debugger {
    bool enabled = false;
    std::mutex mut;
    uint32_t* last_pc = nullptr;
    uint32_t last_instr = 0;
public:
    void set_brk(uint32_t* pc);
    void ret();
    void usage();
    void enable();
    bool is_enabled();
    void step(class Handler& handler);
};

extern Debugger debugger;