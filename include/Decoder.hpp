#pragma once

#include <cstdint>

struct DecCell {
    uint8_t table;
    uint8_t type;
};

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
    UCOMIX, COMIX,

    CMOVO, CMOVNO, CMOVB, CMOVAE,
    CMOVE, CMOVNE, CMOVBE, CMOVA,
    CMOVS, CMOVNS, CMOVP, CMOVPO,
    CMOVL, CMOVGE, CMOVLE, CMOVG,

    PXOR, ADDX, MULX, CVTX,
    SUBX, DIVX, MOVQ,
    MOVZX8, MOVZX16, MOVSX8, MOVSX16,

    SETO, SETNO, SETB, SETAE,
    SETE, SETNE, SETBE, SETA,
    SETS, SETNS, SETP, SETPO,
    SETL, SETGE, SETLE, SETG,
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

struct Operand {
    uint8_t type;
    uint8_t reg;
    uint8_t idx;
    uint8_t scale;
    int64_t imm;
};
struct X86_64 {
    uint8_t type;
    uint8_t size;
    uint8_t prefix;
    uint8_t reverse;

    Operand dst;
    Operand src;
};

class Decoder {
    uint8_t* guest;

    uint8_t fetch8();
    uint16_t fetch16();
    uint32_t fetch32();
    uint64_t fetch64();

    int64_t fetch8_imm();
    int64_t fetch16_imm();
    int64_t fetch32_imm();
    void fetch_imm(X86_64& buf);

    void decode_rm(Operand& op, uint8_t modrm);
    void decode_rm_r(X86_64& buf);
    void decode_r_rm(X86_64& buf);
    void decode_rex(X86_64& buf, uint8_t rex);

    void decode_GRP0(X86_64& buf, uint8_t byte);
    void decode_rm_r_XMM(X86_64& buf, bool xmm);
    void decode_r_rm_XMM(X86_64& buf, bool xmm);

    void decode_00(X86_64& buf, uint8_t byte);
    void decode_0F(X86_64& buf, uint8_t byte);
public:
    void set_guest(uint8_t* start);
    void decode(X86_64& buf);
    uint8_t* get_guest();
};