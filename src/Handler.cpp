#include "../include/Handler.hpp"
#include "../include/Logger.hpp"
#include "../include/Debugger.hpp"
#include "../include/Cache.hpp"
#include "../include/Compiler.hpp"
#include "../include/Printer_X86_64.hpp"
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <stdint.h>

bool memory_check;

uint32_t* check_code(size_t pc) {
    uint32_t* target = cache.search((uint8_t*)pc);
    if (target == nullptr) {
        logger.warn() << "Not found " << pc << std::endl << RESET_COLOR;
        Compiler compiler;
        compiler.compile((uint8_t*)pc);
        target = cache.search((uint8_t*)pc);
    }
    return target;
}

void segv_handler(int sig, siginfo_t* info, void* ucontext) {
    ucontext_t* ctx = (ucontext_t*)ucontext;
    Handler handler((struct sigcontext*)&ctx->uc_mcontext);
    size_t pc = (size_t)info->si_addr;
    if (memory_check) {
        handler.set_pc(pc + 4);
        memory_check = 0;
        return;
    }
    if (info->si_code == SEGV_ACCERR || pc%4 != 0) {
        handler.set_pc((size_t)check_code(pc));
        if (logger.get_level() <= DEBUG) {
            logger.deb() << "found unhandled jump" << std::endl;
            logger.force() << std::endl;
        }
        return;
    }
    const char* name;
    if (sig == SIGBUS) name = "SIGBUS";
    else name = "segfault";
    logger.err() << name << std::endl;
    if (debugger.is_enabled()) {
        debugger.step();
    }
    handler.print_guest_cpu();
    exit(EXIT_FAILURE);
}
void brk_handler(int sig, siginfo_t* info, void* ucontext) {
    (void)sig;
    (void)info;
    ucontext_t* ctx = (ucontext_t*)ucontext;
    Handler handler((struct sigcontext*)&ctx->uc_mcontext);
    
    uint32_t* pc = (uint32_t*)handler.get_pc();
    uint16_t id = (*pc >> 5) & 0xFFFF;
    uint32_t* target = check_code((size_t)cache.get_patch(id));
    int32_t offset = target - pc;
    *pc = 0x94000000 | (offset & 0x3FFFFFF);
    __builtin___clear_cache(pc, pc+1);
    if (logger.get_level() <= DEBUG) {
        logger.deb() << "found patch: " << id << std::endl;
        logger.force() << std::endl;
    }
}
void segi_handler(int sig, siginfo_t* info, void* ucontext) {
    (void)sig;
    (void)info;
    ucontext_t* ctx = (ucontext_t*)ucontext;
    Handler handler((struct sigcontext*)&ctx->uc_mcontext);
    if (debugger.is_enabled()) {
        debugger.step();
    } else _exit(0);
}

Handler::Handler(struct sigcontext* n_sc) {
    sc = n_sc;
}
void Handler::start_memory_chech() {
    memory_check = 1;
}
bool Handler::end_memory_check() {
    if (memory_check) {
        memory_check = 0;
        return false;
    }
    return true;
}

void Handler::print_flags() {
    #ifdef __aarch64__
    int N = (sc->pstate >> 31) & 1;
    int Z = (sc->pstate >> 30) & 1;  
    int C = (sc->pstate >> 29) & 1;
    int V = (sc->pstate >> 28) & 1;
    #else
    int N = (sc->eflags >> 7) & 1;
    int Z = (sc->eflags >> 6) & 1;  
    int C = (sc->eflags >> 0) & 1;
    int V = (sc->eflags >> 11) & 1;
    #endif
    logger.force() << "Flags: N"
                 << N << " Z" << Z<< " C"
                 << C << " V" << V << std::endl;
}
void Handler::print_native_cpu() {
    #ifdef __aarch64__
    uint32_t* pc = (uint32_t*)get_pc();
    printf("PC:  %p\n", pc);
    for (int i = 0; i < 31; i++) {
        printf("X%i: %llX\n", i, sc->regs[i]);
    }
    printf("sp: %llX\n", sc->sp);
    #endif
    print_flags();
}
void Handler::print_guest_cpu() {
    #ifdef __aarch64__
    printf("PC:  %lX\n", get_pc());
    for (int i = 0; i < 16; i++) {
        printf("%s: %llX\n", regs64[i], sc->regs[x86_regs[i]]);
    }
    #endif
    print_flags();
}

int Handler::get_reg(const char* name) {
    #ifdef __aarch64__
    if (strcmp(name, "rsp") == 0) return sc->sp;
    for (int i = 0; i < 16; i++) {
        if (strcmp(name, regs64[i]) == 0)
            return sc->regs[x86_regs[i]];
    }
    #endif
    return 0;
}
void Handler::set_pc(size_t pc) {
    #ifdef __aarch64__
    sc->pc = pc;
    #else
    sc->rip = pc;
    #endif
}
size_t Handler::get_pc() {
    #ifdef __aarch64__
    return sc->pc;
    #else
    return sc->rip;
    #endif
}