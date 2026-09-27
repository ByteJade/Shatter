#include "../inc/handler.h"
#include "../inc/debugger.h"
#include "../inc/logger.h"
#include "../inc/cache.h"
#include "../inc/compiler.h"
#include <stdint.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>

uint32_t* compile(size_t pc) {
    uint32_t* target = cache_search((uint8_t*)pc);
    if (target == NULL) {
        compiler_t* compiler = compiler_init();
        target = compiler_step(compiler, (uint8_t*)pc);
    }
    return target;
}

void brk_handler(int sig, siginfo_t* info, void* ucontext) {
    ucontext_t* ctx = (ucontext_t*)ucontext;
    struct sigcontext* sc = (struct sigcontext*)&ctx->uc_mcontext;
    
    uint32_t* pc = (uint32_t*)sc_get_pc(sc);
    uint32_t instruction = *pc;
    uint16_t ret = (instruction >> 5) & 0xFFFF;
    if (ret == 0) {
        debugger_step(sc);
        return;
    }
    uint8_t* patch = cache_get_patch(ret);
    uint32_t* target = compile((size_t)patch);
    int32_t offset = target - pc;
    *pc = 0x94000000 | (offset & 0x3FFFFFF);
    __builtin___clear_cache(pc, pc+1);
    logger_deb("found patch: %i", ret);
}
void segv_handler(int sig, siginfo_t* info, void* ucontext) {
    ucontext_t* ctx = (ucontext_t*)ucontext;
    struct sigcontext* sc = (struct sigcontext*)&ctx->uc_mcontext;

    size_t pc = sc_get_pc(sc);
    if (info->si_code == SEGV_ACCERR || pc%4 != 0) {
        logger_deb("Found unhandled jump %p", pc);
        uint32_t* target = compile(pc);
        sc_set_pc(sc, (size_t)target);
        return;
    }
    const char* name;
    if (sig == SIGBUS) name = "SIGBUS";
    else name = "segfault";
        logger_err("%s", name);
    if (debugger_enabled()) {
        debugger_step(sc);
    }
    sc_print_regs_host(sc);
    _exit(0);
}
void segi_handler(int sig, siginfo_t* info, void* ucontext) {
    ucontext_t* ctx = (ucontext_t*)ucontext;
    struct sigcontext* sc = (struct sigcontext*)&ctx->uc_mcontext;
    if (debugger_enabled()) {
        debugger_step(sc);
    } else _exit(0);
}
void handler_init(void) {
    struct sigaction sa_trap = {0};
    sa_trap.sa_sigaction = brk_handler;
    sa_trap.sa_flags = SA_SIGINFO;
    struct sigaction sa_segv = {0};
    sa_segv.sa_sigaction = segv_handler;
    sa_segv.sa_flags = SA_SIGINFO;
    struct sigaction sa_segi = {0};
    sa_segi.sa_sigaction = segi_handler;
    sa_segi.sa_flags = SA_SIGINFO;
    sigaction(SIGTRAP, &sa_trap, NULL);
    sigaction(SIGSEGV, &sa_segv, NULL);
    sigaction(SIGILL, &sa_segv, NULL);
    sigaction(SIGBUS, &sa_segv, NULL);
    sigaction(SIGINT, &sa_segi, NULL);
}

size_t sc_get_pc(struct sigcontext* sc) {
    #ifdef __aarch64__
    return sc->pc;
    #else
    return 0;
    #endif
}
void sc_set_pc(struct sigcontext* sc, size_t pc) {
    #ifdef __aarch64__
    sc->pc = pc;
    #endif
}
int sc_get_flag(struct sigcontext* sc, const char* flag) {
    #ifdef __aarch64__
    switch (*flag) {
        case 'N': return (sc->pstate >> 31) & 1;
        case 'Z': return (sc->pstate >> 30) & 1;
        case 'C': return (sc->pstate >> 29) & 1;
        case 'V': return (sc->pstate >> 28) & 1;
    }
    #endif
    return 0;
}
size_t sc_get_reg_host(struct sigcontext* sc, const char* reg) {
    int num = 0;
    sscanf(reg+1, "%i", &num);
    #ifdef __aarch64__
    if (strcmp(reg, sp) == 0) return sc->sp;
    return sc->regs[num];
    #else
    return 0;
    #endif
}
size_t sc_get_reg_guest(struct sigcontext* sc, const char* reg) {
    return 0;
}
void sc_print_flags(struct sigcontext* sc) {
    printf("Flags: N%i Z%i C%i V%i\n",
        sc_get_flag(sc, "N"),
        sc_get_flag(sc, "Z"),
        sc_get_flag(sc, "C"),
        sc_get_flag(sc, "V")
    );
}
void sc_print_regs_host(struct sigcontext* sc) {
    #ifdef __aarch64__
    for (int i = 0; i < 31; i++) {
        printf("X%i: %llX\n", i, sc->regs[i]);
    }
    printf("sp: %llX\n", sc->sp);
    #endif
}
void sc_print_regs_guest(struct sigcontext* sc) {

}
