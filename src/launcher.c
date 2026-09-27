#include "../inc/launcher.h"

void execute(void* start) {
    #ifdef __aarch64__ 
    asm volatile (
        "blr %0\n"
        : : "r" (start)
        : "memory", "x24", "x25", "x26", "x27", "x28"
    );
    #else
    asm volatile (
        "call *%0\n"
        : : "r" (start)
        : "memory"
    );
    #endif
}
void launch(void* start, stack_t* stack) {
    #ifdef __aarch64__ 
    asm volatile (
        "mov sp, %0\n"
        "br %1\n"

        : : "r" (stack->top), "r" (start)
        : "memory", "x24", "x25", "x26", "x27", "x28"
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