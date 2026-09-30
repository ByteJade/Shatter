#include "../inc/debugger.h"
#include "../inc/dynarray.h"
#include "../inc/handler.h"
#include "../inc/encoder.h"
#include "../inc/logger.h"
#include "../inc/cache.h"
#include "../inc/printer_Aarch64.h"
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>

#define BCC_M 0xFF00001F

typedef struct {
    uint32_t* pos;
    uint32_t instr;
} break_t;

int enabled = 0;
pthread_mutex_t mut;
break_t* breaks = NULL;

void debugger_init(void) {
    enabled = 1;
    breaks = dynarray_init(sizeof(break_t));
}
void debugger_fini(void) {
    enabled = 0;
    dynarray_fini(breaks);
}
void debugger_brk(uint32_t* host) {
    size_t break_p = dynarray_push((void**)&breaks);
    break_t* b = breaks + break_p;
    b->pos = host;
    b->instr = *host;
    *host = BRK;
    __builtin___clear_cache(host, host+1);
}
void debugger_ret(uint32_t* host) {
    for (size_t i = 0; i < dynarray_size(breaks); i++) {
        break_t* b = breaks + i;
        if (b->pos == host) {
            *host = b->instr;
            *b = breaks[dynarray_pop(breaks)];
            __builtin___clear_cache(host, host + 1);
            return;
        }
    }
}
void emulate_jump(struct sigcontext* sc) {
    uint32_t* pc = (uint32_t*)sc_get_pc(sc);
    uint32_t instr = *pc;
    print_aarch64(instr);
    if (instr == (RET_R | (30<<5))) return;
    int imm = 1;
    switch (instr&BCC_M) {
    case BEQ:
        if (sc_get_flag(sc, "Z"))
            imm = get_imm19(instr);
        break;
    case BNE:
        if (!sc_get_flag(sc, "Z"))
            imm = get_imm19(instr);
        break;
    case BCS:
        if (sc_get_flag(sc, "C"))
            imm = get_imm19(instr);
        break;
    case BLS:
        if (!sc_get_flag(sc, "C") || sc_get_flag(sc, "Z"))
            imm = get_imm19(instr);
        break;
    case BGE:
        if (sc_get_flag(sc, "N") == sc_get_flag(sc, "V"))
            imm = get_imm19(instr);
        break;
    case BLT:
        if (sc_get_flag(sc, "N") != sc_get_flag(sc, "V"))
            imm = get_imm19(instr);
        break;
    case BGT:
        if (!sc_get_flag(sc, "Z") && sc_get_flag(sc, "N") == sc_get_flag(sc, "V"))
            imm = get_imm19(instr);
        break;
    case BLE:
        if (sc_get_flag(sc, "Z") || sc_get_flag(sc, "N") != sc_get_flag(sc, "V"))
            imm = get_imm19(instr);
        break;
    case BHI:
        if (sc_get_flag(sc, "Z") == 0 && sc_get_flag(sc, "C"))
            imm = get_imm19(instr);
        break;
    case BMI:
        if (sc_get_flag(sc, "N"))
            imm = get_imm19(instr);
        break;
    }
    if ((instr&B_M) == B)
        imm = get_imm26(instr);
    debugger_brk(pc + imm);
}
void print_memory(struct sigcontext* sc, char* buf) {
    char com[32];
    char base[32];
    sscanf(buf, "%s %s", com, base);
    int imm = 0;
    size_t reg = sc_get_reg_host(sc, base+1);
    char* sep = strpbrk(base, "+-");
    if (sep) {
        imm = strtol(sep, NULL, 0);
    }
    reg += imm;
    switch (*com) {
        case 'b': printf("%s: %x\n", base, *(uint8_t*)(reg)); break;
        case 'h': printf("%s: %x\n", base, *(uint16_t*)(reg)); break;
        case 'w': printf("%s: %x\n", base, *(uint32_t*)(reg)); break;
        case 'd': printf("%s: %lx\n", base, *(uint64_t*)(reg)); break;
    }
}
void debugger_help() {
    printf("debugger commands:\n");
    printf("step - single step\n");
    printf("exit - continue execution\n");
    printf("abort - exit from programm\n");
    printf("flags - print cpu flags\n");
    printf("regs - print cpu regs\n");
    printf("memory [<reg>+<imm>] - print data in this location\n");
    printf("help - print this message\n");
    printf("level <log_level> - set log level (log, deb, warn, err)\n");
    printf("cache - print last compiled block\n");
    printf("usage - print cache memory usage\n");
}
void debugger_step(struct sigcontext* sc) {
    pthread_mutex_lock(&mut);
    debugger_ret((uint32_t*)sc_get_pc(sc));
    int run = 1;
    while (run) {
        char buf[256];
        printf("> ");
        fgets(buf, sizeof(buf), stdin);
        switch (buf[0]) {
            case 's':
                emulate_jump(sc);
                [[fallthrough]];
            case 'e':
                run = 0;
                break;
            case 'a':
                exit(EXIT_SUCCESS);
                break;
            case 'f':
                sc_print_flags(sc);
                break;
            case 'r':
                sc_print_regs_host(sc);
                break;
            case 'b': case 'h':
            case 'w': case 'd':
                print_memory(sc, buf);
                break;
            case 'l':
                logger_set_level(strpbrk(buf, " ")+1);
                break;
            case 'c':
                cache_print();
                break;
            case 'u':
                cache_usage();
                break;
            default:
                debugger_help();
        }
    }
    pthread_mutex_unlock(&mut);
}
int debugger_enabled(void) {
    return enabled;
}
