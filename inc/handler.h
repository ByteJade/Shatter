#ifndef HANDLER_H
#define HANDLER_H

#include <stdlib.h>

struct sigcontext;

#define NF(pstate) ((pstate >> 31) & 1)
#define ZF(pstate) ((pstate >> 30) & 1)
#define CF(pstate) ((pstate >> 29) & 1)
#define VF(pstate) ((pstate >> 28) & 1)

void handler_init(void);

size_t sc_get_pc(struct sigcontext* sc);
void sc_set_pc(struct sigcontext* sc, size_t pc);
int sc_get_flags(struct sigcontext* sc);
size_t sc_get_reg_host(struct sigcontext* sc, const char* reg);
void sc_print_flags(struct sigcontext* sc);
void sc_print_regs_host(struct sigcontext* sc);

#endif
