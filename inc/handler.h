#ifndef HANDLER_H
#define HANDLER_H

#include <stdlib.h>

struct sigcontext;

void handler_init(void);

size_t sc_get_pc(struct sigcontext* sc);
void sc_set_pc(struct sigcontext* sc, size_t pc);
int sc_get_flag(struct sigcontext* sc, const char* flag);
size_t sc_get_reg_host(struct sigcontext* sc, const char* reg);
void sc_print_flags(struct sigcontext* sc);
void sc_print_regs_host(struct sigcontext* sc);

#endif
