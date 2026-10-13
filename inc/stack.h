#ifndef STACK_H
#define STACK_H

#include <stdlib.h>

#define STACK_SIZE 1024*1024

typedef struct {
    void* base;
    size_t* top;
    char* down;
} Stack;

void push_str(Stack* stack, const char* str);
void push_arg(Stack* stack, size_t arg);

Stack* stack_init(void);
void stack_setup(Stack* stack, int argc, char** argv, char** envp);
void stack_fini(Stack* stack);

#endif