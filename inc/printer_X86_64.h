#ifndef PRINTER_X86_64_H
#define PRINTER_X86_64_H

#include "decoder.h"

extern const char* instr_types[];
extern const char* regs64[];
extern const char* regs32[];
extern const char* regs16[];
extern const char* regs8[];
extern const char* scale[];

void print_x86_64(X86_64* buf);

#endif