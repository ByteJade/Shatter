#pragma once

#include <signal.h>

void segv_handler(int sig, siginfo_t* info, void* ucontext);
void brk_handler(int sig, siginfo_t* info, void* ucontext);
void segi_handler(int sig, siginfo_t* info, void* ucontext);

class Handler {
    struct sigcontext* sc;
public:
    Handler(struct sigcontext* n_sc);
    void start_memory_chech();
    bool end_memory_check();

    void print_flags();
    void print_native_cpu();
    void print_guest_cpu();

    bool get_flag(char f);
    int get_reg(const char* name);
    void set_pc(size_t pc);
    size_t get_pc();
};