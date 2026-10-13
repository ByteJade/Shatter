#include "../inc/launcher.h"

void* execute(void* start) {
    void* ret = 0;
    #ifdef __aarch64__
    asm volatile (
        "blr %1\n"
        : "=r"(ret)
        : "r"(start)
        : "memory", "x24", "x25", "x26", "x28", "x30"
    );
    #else
    asm volatile (
        "call *%0\n"
        : "=r"(ret)
        : "r" (start)
        : "memory"
    );
    #endif
    return ret;
}
void launch(void* start, stack_t* stack) {
    #ifdef __aarch64__ 
    asm volatile (
        "mov sp, %0\n"
        "br %1\n"

        : : "r" (stack->top), "r" (start)
        : "memory"
    );
    #else
    asm volatile (
        "mov %0, %%rsp\n"
        "jmp *%1\n"
        : : "r" (stack->top), "r" (start)
        : "memory"
    );
    #endif
}