#ifndef LAUNCHER_H
#define LAUNCHER_H

#include "stack.h"

void execute(void* start);
void launch(void* start, stack_t* stack);

#endif