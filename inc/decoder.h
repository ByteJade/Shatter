#ifndef DECODER_H
#define DECODER_H

#include <stdint.h>

enum Registers {
    RAX,RCX,RDX,RBX,
    RSP,RBP,RSI,RDI,
    R8, R9, R10,R11,
    R12,R13,R14,R15,
};

enum Types {
    TEST, ERR, NOT, NEG,
    MUL, IMUL, DIV, IDIV,

    ADD, OR, ADC, SBB,
    AND, SUB, XOR, CMP,

    ROL, ROR, RCL, RCR,
    SHL, SHR, SAL, SAR,

    INC, DEC, CALL, CALF,
    JMP, JMPF, PUSH, POP,
    
    JO, JNO, JB, JAE,
    JE, JNE, JBE, JA,
    JS, JNS, JP, JPO,
    JL, JGE, JLE, JG,

    MOV, LEA, RET, MOVSX,

    LEAVE, NOP, CLTQ, CLTD,

    SYSCALL, MOVX, EBR, HLT,
    MOVAPX, CVTSI2X, CVTX2SI,
    UCOMIX, MOVQ,

    CMOVO, CMOVNO, CMOVB, CMOVAE,
    CMOVE, CMOVNE, CMOVBE, CMOVA,
    CMOVS, CMOVNS, CMOVP, CMOVPO,
    CMOVL, CMOVGE, CMOVLE, CMOVG,

    SETO, SETNO, SETB, SETAE,
    SETE, SETNE, SETBE, SETA,
    SETS, SETNS, SETP, SETPO,
    SETL, SETGE, SETLE, SETG,

    PXOR, ADDX, MULX,
    SUBX, DIVX, COMIX, CVTX,
    MOVZX8, MOVZX16, MOVSX8, MOVSX16,
};

enum prefixes {
    FS = 0x64,
    GS = 0x65,
    OS = 0x66,
    //LOCK = 0xF0,
    REPN = 0xF2,
    REPE = 0xF3,
};
enum OperandType {
    NONE = 0,
    REG = 1 << 0,
    MEM = 1 << 1,
    IDX = 1 << 2,
    IMM = 1 << 3,
    XMM = 1 << 4,
};

typedef struct {
    uint8_t type;
    uint8_t reg;
    uint8_t idx;
    uint8_t scale;
    int64_t imm;
} operand_t;
typedef struct {
    uint8_t type;
    uint8_t size;
    uint8_t prefix;
    uint8_t reverse;

    operand_t dst;
    operand_t src;
} X86_64;

struct compiler_t;

uint8_t fetch8(struct compiler_t* compiler);
uint16_t fetch16(struct compiler_t* compiler);
uint32_t fetch32(struct compiler_t* compiler);
uint64_t fetch64(struct compiler_t* compiler);

int64_t fetch8_imm(struct compiler_t* compiler);
int64_t fetch16_imm(struct compiler_t* compiler);
int64_t fetch32_imm(struct compiler_t* compiler);

void fetch_imm(struct compiler_t* compiler, X86_64* buf);

void decode_rm(struct compiler_t* compiler, operand_t* op, uint8_t modrm);
void decode_rm_r(struct compiler_t* compiler, X86_64* buf);
void decode_r_rm(struct compiler_t* compiler, X86_64* buf);
void decode_rex(X86_64* buf, uint8_t rex);

void decode_GRP0(struct compiler_t* compiler, X86_64* buf, uint8_t byte);
void decode_rm_r_XMM(struct compiler_t* compiler, X86_64* buf, int xmm);
void decode_r_rm_XMM(struct compiler_t* compiler, X86_64* buf, int xmm);

void decode_00(struct compiler_t* compiler, X86_64* buf, uint8_t byte);
void decode_0F(struct compiler_t* compiler, X86_64* buf, uint8_t byte);

void decode(struct compiler_t* compiler, X86_64* buf);

#endif