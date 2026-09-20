#pragma once

#include <cstdlib>

class Stack {
    void* stack = nullptr;
    size_t* stack_top = nullptr;
    char* stack_down = nullptr;

    void push_str(const char* str);
    void push_arg(size_t arg);
public:
    Stack();
    ~Stack();
    
    void setup(int argc, char** argv, char** envp);
    size_t* get();
};