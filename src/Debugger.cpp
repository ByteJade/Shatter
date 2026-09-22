#include "../include/Debugger.hpp"
#include "../include/Handler.hpp"
#include "../include/Encoder.hpp"
#include "../include/Logger.hpp"
#include "../include/Printer_Aarch64.hpp"
#include <cstdint>
#include <cstdio>
#include <readline/readline.h>
#include <readline/history.h>
#include <unistd.h>

Debugger debugger;

char* skip(char* src) {
    while (*src != ' ') {
        if (*src == '\0') return src;
        src++;
    }
    return ++src;
}

#define BCC_M 0xFF00001F

void emulate(Handler& handler) {
    uint32_t* pc = (uint32_t*)handler.get_pc();
    uint32_t instr = *(uint32_t*)pc;
    logger.log() << "Emulate: ";
    print(logger.log(), instr);
    int imm = 1;
    switch (instr&BCC_M) {
    case BEQ:
        if (handler.get_flag('Z'))
            imm = get_imm19(instr);
        break;
    case BNE:
        if (!handler.get_flag('Z'))
            imm = get_imm19(instr);
        break;
    case BCS:
        if (handler.get_flag('C'))
            imm = get_imm19(instr);
        break;
    case BLS:
        if (!handler.get_flag('C') || handler.get_flag('Z'))
            imm = get_imm19(instr);
        break;
    case BGE:
        if (handler.get_flag('N') == handler.get_flag('V'))
            imm = get_imm19(instr);
        break;
    case BLT:
        if (handler.get_flag('N') != handler.get_flag('V'))
            imm = get_imm19(instr);
        break;
    case BGT:
        if (!handler.get_flag('Z') && handler.get_flag('N') == handler.get_flag('V'))
            imm = get_imm19(instr);
        break;
    case BLE:
        if (handler.get_flag('Z') || handler.get_flag('N') != handler.get_flag('V'))
            imm = get_imm19(instr);
        break;
    case B:
        imm = get_imm26(instr);
        break;
    }
    debugger.set_brk(pc + imm);
}

void Debugger::set_brk(uint32_t* pc) {
    logger.force() << "Set break at " << (size_t)pc << std::endl;
    ret();
    last_pc = pc;
    last_instr = *pc;
    *pc = BRK;
    __builtin___clear_cache(pc, pc+1);
}
void Debugger::ret() {
    if (last_pc) {
        *last_pc = last_instr;
        __builtin___clear_cache(last_pc, last_pc+1);
        last_pc = nullptr;
    }
}

void Debugger::usage() {
    logger.force() << "Commands:" << std::endl
        << "exit - return to execution" << std::endl
        << "si - skip instruction" << std::endl
        << "print - print current instruction" << std::endl
        << "brk <imm> - set break at pc+imm" << std::endl
        << "level \"level\" - set logger level" << std::endl;
}
void Debugger::enable() {
    enabled = true;
}
bool Debugger::is_enabled() {
    return enabled;
}
void Debugger::step(Handler& handler) {
    if (!enabled) return;
    mut.lock();
    ret();
    bool run = true;
    char* line = nullptr;
    while (run) {
        line = readline("> ");
        if (!line) break;
        if (*line) add_history(line);
        switch (*line) {
        case 's':
            emulate(handler);
            [[fallthrough]];
        case 'e':
            run = false;
            break;
        case 'p': { // print
            char* arg = skip(line);
            if (!*arg) break;
            if (*arg == 'i') { // instruction
                uint32_t instr = *(uint32_t*)handler.get_pc();
                logger.force() << std::hex << instr << std::dec << ": ";
                print(logger.force(), instr);
            } else if (*arg == 'r') { // regs
                handler.print_native_cpu();
            } else if (*arg == 'g') { // guest_regs
                handler.print_guest_cpu();
            }
        } break;
        case 'b': {
            char* arg = skip(line);
            if (!*arg) break;
            int imm;
            sscanf(arg, "%i", &imm);
            set_brk((uint32_t*)(handler.get_pc()+imm));  
        } break;
        case 'l':
            logger.set_level(skip(line));  
            break;
        default: usage();
        }
        free(line);
    }
    mut.unlock();
}