#include "../include/Stack.hpp"
#include "../include/Logger.hpp"
#include <cstring>

#define STACK_SIZE 1024*1024

void Stack::push_str(const char* str) {
    push_arg((size_t)stack_down);
    size_t len = strlen(str) + 1;
    memcpy(stack_down, str, len);
    stack_down += len;
}
void Stack::push_arg(size_t arg) {
    stack_top--;
    *stack_top = arg;
}

Stack::Stack() {
    stack = malloc(STACK_SIZE);
    stack_down = (char*)stack;
    stack_top = (size_t*)(stack_down + STACK_SIZE);
}
Stack::~Stack() {
    free(stack);
}
void Stack::setup(int argc, char** argv, char** envp) {
    // Stack allign
    int envpc = 0;
    while (envp[envpc]) {envpc++;}
    if ((envpc + argc)%2 == 0) push_arg(0);
    // Stack setup
    push_arg(0);
    while (*envp) push_str(*envp++);
    push_arg(0);
    for (int i = argc-1; i >= 0; i--) push_str(argv[i]);
    push_arg(argc);
    logger.deb() << "Stack setup finish" << std::endl;
}
size_t* Stack::get() {
    if ((size_t)stack_top % (sizeof(size_t)*2) != 0)
        logger.err() << "Stack unalligned!" << std::endl;
    return stack_top;
}