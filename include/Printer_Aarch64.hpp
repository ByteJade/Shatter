#pragma once

#include <iostream>
#include <cstdint>

#define B_M 0xFF000010
#define AR_M 0x0FE0FC00
#define AI_M 0x1FC00000

int32_t get_imm19(uint32_t buf);
int32_t get_imm26(uint32_t buf);

void print(std::ostream& stream, uint32_t buf);