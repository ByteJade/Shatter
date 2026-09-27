#ifndef DEBUGGER_H
#define DEBUGGER_H

#include <stdint.h>

struct sigcontext;

void debugger_init(void);
void debugger_fini(void);
void debugger_brk(uint32_t* host);
void debugger_ret(uint32_t* host);
void debugger_step(struct sigcontext* sc);
int debugger_enabled(void);

#endif