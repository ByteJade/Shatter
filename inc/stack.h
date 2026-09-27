#ifndef STACK_H
#define STACK_H

#include <stdlib.h>

#define STACK_SIZE 1024*1024

typedef struct {
    void* base;
    size_t* top;
    char* down;
} stack_t;

void push_str(stack_t* stack, const char* str);
void push_arg(stack_t* stack, size_t arg);

stack_t* stack_init(void);
void stack_setup(stack_t* stack, int argc, char** argv, char** envp);
void stack_fini(stack_t* stack);

#endif