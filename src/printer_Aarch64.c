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
const char* shift[] = {
    "lsl", "lsr", "asr", "ror",
};
int32_t get_imm9(uint32_t buf) {
    int32_t n = (buf >> 12) & 0x1FF;
    return (n << 23) >> 23;
}
int32_t get_imm10(uint32_t buf) {
    int32_t n = (buf >> 12) & 0x3F8;
    return (n << 22) >> 22;
}
int32_t get_imm12(uint32_t buf) {
    int32_t n = (buf >> 10) & 0xFFF;
    return (n << 20) >> 20;
}
int32_t get_imm16(uint32_t buf) {
    int16_t n = (buf >> 5) & 0xFFFF;
    return (int32_t)n;
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
        case 3: ret = buf>>10; break;
    }
    return ret&0x1F;
}
void print_r_r_r(uint32_t buf) {
    char reg = 'W';
    if (buf&ASF) reg = 'X';
    printf(GREEN_COLOR" %c%i, %c%i, %c%i",
        reg, get_reg(buf,0),
        reg, get_reg(buf,1),
        reg, get_reg(buf,2)
    );
}
void print_r_r_i(uint32_t buf) {
    char reg = 'W';
    if (buf&ASF) reg = 'X';
    printf(GREEN_COLOR" %c%i, %c%i, %i",
        reg, get_reg(buf,0),
        reg, get_reg(buf,1),
        get_imm12(buf)
    );
}
void print_r_i(uint32_t buf) {
    char reg = 'W';
    if (buf&ASF) reg = 'X';
    printf(GREEN_COLOR" %c%i, %i",
        reg, get_reg(buf,0),
        get_imm16(buf)
    );
}
void print_r_m(uint32_t buf) {
    char reg = 'W';
    if (buf&MSF) reg = 'X';
    printf(GREEN_COLOR" %c%i, [X%i",
        reg, get_reg(buf, 0),
        get_reg(buf, 1)
    );
    if (!((buf >> 24)&1)) {
        int imm = get_imm9(buf);
        int addent = (buf>>10)&0x3;
        if (addent == 1)
            printf("], %i", imm);
        else printf(", %i]", imm);
        if (addent == 3) printf("!");
    } else {
        int imm = get_imm12(buf) * ((buf&MSF)+1)*4;
        if (imm) printf(", %i]", imm);
        else printf("]");
    }
}
void print_r_r_m(uint32_t buf) {
    char reg = 'W';
    if (buf&ASF) reg = 'X';
    printf(GREEN_COLOR" %c%i, %c%i, [X%i",
        reg, get_reg(buf, 0),
        reg, get_reg(buf, 3),
        get_reg(buf, 1)
    );
    int imm = get_imm10(buf);
    if ((buf >> 24)&1) {
        printf(", %i]!", imm);
    } else printf("], %i", imm);
}
void decode_aarch64(uint32_t buf) {
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
        printf("movk (%i),", (buf >> 21)&3);
        print_r_i(buf);
        return;
    }
    if ((buf&B_M) == B) {
        printf("b "GREEN_COLOR"%i", get_imm26(buf));
        return;
    }
    if ((buf&B_M) == BL) {
        printf("bl "GREEN_COLOR"%i", get_imm26(buf));
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
    if ((buf&MEM_M) == STUR) {
        printf("str");
        print_r_m(buf);
        return;
    }
    if ((buf&MEM_M) == LDUR) {
        printf("ldr");
        print_r_m(buf);
        return;
    }
    if ((buf&MEMP_M) == STP_POST) {
        printf("stp");
        print_r_r_m(buf);
        return;
    }
    if ((buf&MEMP_M) == LDP_POST) {
        printf("ldp");
        print_r_r_m(buf);
        return;
    }
    if ((buf&AS_M) == LSL_R) {
        printf("%s", shift[(buf>>10)&3]);
        print_r_r_r(buf);
        return;
    }
    if ((buf&ADRP_M) == ADRP) {
        printf("adrp "GREEN_COLOR"X%i", get_reg(buf, 0));
        return;
    }
    if ((buf&BR_M) == BLR) {
        printf("blr "GREEN_COLOR"X%i", get_reg(buf, 1));
        return;
    }
    if ((buf&BR_M) == BR) {
        printf("br "GREEN_COLOR"X%i", get_reg(buf, 1));
        return;
    }
    if ((buf&BR_M) == BRK) {
        printf("brk "GREEN_COLOR"%i", get_imm16(buf));
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