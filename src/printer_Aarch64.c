#include "../inc/printer_Aarch64.h"
#include "../inc/encoder.h"
#include "../inc/logger.h"

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
int32_t get_imm16(uint32_t buf) {
    int32_t n = (buf >> 10) & 0xFFF;
    return (int32_t)(int16_t)n;
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
void print_r_r_r(uint32_t buf) {
    char reg = 'W';
    if (buf&ASF) reg = 'X';
    printf(GREEN_COLOR" %c%i %c%i %c%i",
        reg, get_reg(buf,0),
        reg, get_reg(buf,1),
        reg, get_reg(buf,2)
    );
}
void print_r_r_i(uint32_t buf) {
    char reg = 'W';
    if (buf&ASF) reg = 'X';
    printf(GREEN_COLOR" %c%i %c%i %x",
        reg, get_reg(buf,0),
        reg, get_reg(buf,1),
        get_imm12(buf)
    );
}
void print_r_i(uint32_t buf) {
    char reg = 'W';
    if (buf&ASF) reg = 'X';
    printf(GREEN_COLOR" %c%i %x",
        reg, get_reg(buf,1),
        get_imm16(buf)
    );
}
void decode_aarch64(uint32_t buf) {
    if ((buf&B_M) == B) {
        printf("b "GREEN_COLOR"%i", get_imm26(buf));
        return;
    }
    if ((buf&BC_M) == BEQ) {
        printf("%s "GREEN_COLOR"%i", jumps[buf&0xF], get_imm19(buf));
        return;
    }
    if ((buf&AR_M) == AND_R) {
        printf("%s", arythm[(buf>>28)&7]);
        print_r_r_r(buf);
        return;
    }
    if ((buf&AR_M) == ADD_R) {
        printf("%s", math[(buf>>29)&3]);
        print_r_r_r(buf);
        return;
    }
    if ((buf&AI_M) == ADD_I) {
        printf("%s", math[(buf>>29)&3]);
        print_r_r_i(buf);
        return;
    }
    if ((buf&MOV_M) == MOVZ_I) {
        printf("movz");
        print_r_i(buf);
        return;
    }
    if ((buf&MOV_M) == MOVN_I) {
        printf("movn");
        print_r_i(buf);
        return;
    }
    if ((buf&MOV_M) == MOVK_I) {
        printf("movk");
        print_r_i(buf);
        return;
    }
    if (buf == (RET_R|(30<<5))) {
        printf("ret");
        return;
    }
    printf("unk");
}
void print_aarch64(uint32_t buf) {
    printf(BLUE_COLOR);
    decode_aarch64(buf);
    printf(RESET_COLOR"\n");
}