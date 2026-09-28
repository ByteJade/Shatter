#ifndef PRINTER_AARCH64_H
#define PRINTER_AARCH64_H

#include <stdint.h>

#define B_M 0xFF000000
#define BC_M 0xFF000010
#define AR_M 0x0FE0FC00
#define AI_M 0x1FC00000
#define MOV_M 0x7FF00000
#define MEM_M 0xBEC00000
#define MEMP_M 0x7EC00000
#define ADRP_M 0x9F000000
#define BR_M 0xFFFF0000
#define AS_M 0x7FE0F000

int32_t get_imm12(uint32_t buf);
int32_t get_imm16(uint32_t buf);
int32_t get_imm19(uint32_t buf);
int32_t get_imm26(uint32_t buf);

void print_aarch64(uint32_t buf);

#endif