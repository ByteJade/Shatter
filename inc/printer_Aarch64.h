#ifndef PRINTER_AARCH64_H
#define PRINTER_AARCH64_H

#include <stdint.h>

#define B_M 0xFC000000
#define BC_M 0xFC000010
#define AR_M 0x0FE0FC00
#define AI_M 0x1FC00000
#define MOV_M 0x7FC00000
#define MEM_M 0x0EC00000
#define MEMP_M 0x2EC00000
#define ADRP_M 0x9F000000
#define BR_M 0xFFFF0000
#define AS_M 0x7FE0E000
#define FS_M 0xFFFFFFF0
#define BFM_M 0xFFC00000
#define MUL_M 0xFFF00000
#define MSUB_M 0xFFC08000
#define SDIV_M 0xFFC00C00

int32_t get_imm12(uint32_t buf);
int32_t get_imm16(uint32_t buf);
int32_t get_imm19(uint32_t buf);
int32_t get_imm26(uint32_t buf);

void print_aarch64(uint32_t buf);

#endif