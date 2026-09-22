#pragma once

#include "Decoder.hpp"
#include <iostream>

extern const char* instr_types[];
extern const char* regs64[];
extern const char* regs32[];
extern const char* regs16[];
extern const char* regs8[];
extern const char* scale[];

void print(std::ostream& stream, X86_64& buf);