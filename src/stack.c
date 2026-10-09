#include "../inc/stack.h"
#include "../inc/logger.h"
#include <string.h>
#include <sys/mman.h>

void push_str(stack_t* stack, const char* str) {
    push_arg(stack, (size_t)stack->down);
    size_t len = strlen(str) + 1;
    memcpy(stack->down, str, len);
    stack->down += len;
}
void push_arg(stack_t* stack, size_t arg) {
    stack->top--;
    *stack->top = arg;
}

stack_t* stack_init(void) {
    stack_t* stack = (stack_t*)malloc(sizeof(stack_t));
    stack->base = mmap(
        NULL, STACK_SIZE,
        PROT_READ | PROT_WRITE,
        MAP_STACK | MAP_ANON | MAP_PRIVATE,
        -1, 0
    );
    stack->down = (char*)stack->base;
    stack->top = (size_t*)(stack->down + STACK_SIZE);
    return stack;
}
void stack_setup(stack_t* stack, int argc, char** argv, char** envp) {
    int envpc = 0;
    while (envp[envpc]) {envpc++;}
    if ((envpc + argc)%2 == 0) push_arg(stack, 0);
    
    push_arg(stack, 0);
    while (*envp) push_str(stack, *envp++);
    push_arg(stack, 0);
    for (int i = argc-1; i >= 0; i--) push_str(stack, argv[i]);
    push_arg(stack, argc);

    if ((size_t)stack->top % (sizeof(size_t)*2) != 0)
        logger_err("Stack unalligned!");
    else logger_deb("Stack setup finish");
}
void stack_fini(stack_t* stack) {
    if (stack) {
        munmap(stack->base, STACK_SIZE);
        free(stack);
    }
}