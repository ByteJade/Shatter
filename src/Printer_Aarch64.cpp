#include "../include/Printer_Aarch64.hpp"
#include "../include/Encoder.hpp"
#include <cstdint>

const char* jumps[] = {
    "b.eq", "b.ne", "b.cs", "b.cc",
    "b.mi", "b.pl", "b.vs", "b.vc",
    "b.hi", "b.ls", "b.ge", "b.lt",
    "b.gt", "b.le", "b.al", "b.nv",
};
const char* arythm[] = {
    "and", "adc", "orr", "adcs",
    "eor", "sbc", "ands", "sbcs",
};
const char* math[] = {
    "add", "adds", "sub", "subs",
};

int32_t get_imm12(uint32_t buf) {
    int32_t n = (buf >> 10) & 0xFFF;
    return (n << 20) >> 20;
}
int32_t get_imm19(uint32_t buf) {
    int32_t n = (buf >> 5) & 0x7FFFF;
    return (n << 13) >> 13;
}
int32_t get_imm26(uint32_t buf) {
    int32_t n = buf & 0x3FFFFFF;
    return (n << 6) >> 6;
}
int get_reg(uint32_t buf, int reg) {
    uint8_t ret = 0;
    switch (reg) {
        case 0: ret = buf; break;
        case 1: ret = buf>>5; break;
        case 2: ret = buf>>16; break;
    }
    return ret&0x1F;
}
void print_r_r_r(std::ostream& stream, uint32_t buf) {
    const char* reg = " W";
    if (buf&ASF) reg = " X";
    stream << reg << get_reg(buf,0);
    stream << reg << get_reg(buf,1);
    stream << reg << get_reg(buf,2);
}
void print_r_r_i(std::ostream& stream, uint32_t buf) {
    const char* reg = " W";
    if (buf&ASF) reg = " X";
    stream << reg << get_reg(buf,0);
    stream << reg << get_reg(buf,1);
    stream << get_imm12(buf);
}

void print(std::ostream& stream, uint32_t buf) {
    if ((buf&B_M) == B) {
        stream << "b " << get_imm26(buf) << std::endl;
        return;
    }
    if ((buf&B_M) == BEQ) {
        stream << jumps[buf&0xF] << " " << get_imm19(buf) << std::endl;
        return;
    }
    if ((buf&AR_M) == AND_R) {
        stream << arythm[buf>>28];
        print_r_r_r(stream, buf);
        stream << std::endl;
        return;
    }
    if ((buf&AR_M) == ADD_R) {
        stream << math[buf>>29];
        print_r_r_r(stream, buf);
        stream << std::endl;
        return;
    }
    if ((buf&AI_M) == ADD_I) {
        stream << math[buf>>29];
        print_r_r_i(stream, buf);
        stream << std::endl;
        return;
    }
    stream << "unk" << std::endl;
}