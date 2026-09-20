#include "../include/Launcher.hpp"
#include "../include/Logger.hpp"

void execute_with_save(void* address) {
    logger.force() << std::endl;
    #ifdef __aarch64__ 
    asm volatile (
        "blr %0\n"
        : : "r" (address)
        : "memory", "x24", "x25", "x26", "x30"
    );
    #else
    asm volatile (
        "call *%0\n"
        : : "r" (address)
        : "memory"
    );
    #endif
}
void execute_with_stack(void* address, void* stack) {
    logger.deb() << "Entry to _start()" << std::endl;
    logger.force() << std::endl;
    #ifdef __aarch64__
    asm volatile (
        "mov x27, sp\n"

        "mov sp, %0\n"
        "blr %1\n"
        "mov sp, x27\n"

        "ldp x26, x30, [sp], #16\n"
        "ldp x24, x25, [sp], #16\n"
        : : "r" (stack), "r" (address)
        : "memory", "x24", "x25", "x26", "x27", "x30"
    );
    #else
    asm volatile (
        "mov %%rsp, %%r9\n"
        "mov %0, %%rsp\n"
        "call *%1\n"
        "mov %%r9, %%rsp\n"
        : : "r" (stack), "r" (address)
        : "memory", "r9"
    );
    #endif
}