#include "../inc/printer_X86_64.h"
#include "../inc/logger.h"

const char* instr_types[] = {
    "test", "err", "not", "neg",
    "mul", "imul", "div", "idiv",

    "add", "or", "adc", "sbb",
    "and", "sub", "xor", "cmp",

    "rol", "ror", "rcl", "rcr",
    "shl", "shr", "sal", "sar",

    "inc", "dec", "call", "calf",
    "jmp", "jmpf", "push", "pop",
    
    "jo", "jno", "jb", "jae",
    "je", "jne", "jbe", "ja",
    "js", "jns", "jp", "jpo",
    "jl", "jge", "jle", "jg",

    "mov", "lea", "ret", "movsx",

    "leave", "nop", "cltq", "cltd",

    "syscall", "movx", "ebr", "hlt",
    "movapx", "cvtsi2x", "cvtx2si",
    "ucomix", "cvtx", "movq",

    "cmovo", "cmovno", "cmovb", "cmovae",
    "cmove", "cmovne", "cmovbe", "cmova",
    "cmovs", "cmovns", "cmovp", "cmovpo",
    "cmovl", "cmovge", "cmovle", "cmovg",

    "seto", "setno", "setb", "setae",
    "sete", "setne", "setbe", "seta",
    "sets", "setns", "setp", "setpo",
    "setl", "setge", "setle", "setg",

    "pxor", "addx", "mulx", 
    "subx", "divx", "comix",
    "movzx8", "movzx16", "movsx8", "movsx16",
};
const char* regs64[] = {
    "rax", "rcx", "rdx", "rbx",
    "rsp", "rbp", "rsi", "rdi",
    "r8", "r9", "r10", "r11",
    "r12", "r13", "r14", "r15",
};
const char* regs32[] = {
    "eax", "ecx", "edx", "ebx",
    "esp", "ebp", "esi", "edi",
    "r8d", "r9d", "r10d", "r11d",
    "r12d", "r13d", "r14d", "r15d",
};
const char* regs16[] = {
    "ax", "cx", "dx", "bx",
    "sp", "bp", "si", "di",
    "r8w", "r9w", "r10w", "r11w",
    "r12w", "r13w", "r14w", "r15w",
};
const char* regs8[] = {
    "al", "cl", "dl", "bl",
    "ah", "ch", "dh", "bh",
    "r8b", "r9b", "r10b", "r11b",
    "r12b", "r13b", "r14b", "r15b",
};
const char* scale[] = {
    "", "* 2 ", "* 4 ", "* 8 ",
};
void print_imm(int64_t imm) {
    if (imm >= 0) printf("+ %lx ", imm);
    else printf("- %lx ",(~imm) + 1);
}

void print_op(X86_64* buf, operand_t* op) {
    if (op->type == REG) {
        if (buf->size == 64) 
            printf("%s ", regs64[op->reg]);
        else if (buf->size == 32) 
            printf("%s ", regs32[op->reg]);
        else if (buf->size == 16) 
            printf("%s ", regs16[op->reg]);
        else printf("%s ", regs8[op->reg]);
    } else if (op->type == IMM) {
        if (op->imm >= 0) printf("%lx ", op->imm);
        else printf("-%lx ",(~op->imm) + 1);
    } else if (op->type == (REG|XMM)) {
        printf("xmm%i ", op->reg);
    } else {
        printf("[ ");
        if (op->type&REG) {
            printf("%s ", regs64[op->reg]);
            if (op->type&IDX) printf("+ ");
        }
        if (op->type&IDX) {
            if (buf->prefix == FS) printf("rip ");
            else printf("%s ", regs64[op->idx]);
            printf("%s", scale[op->scale]);
        }
        if (op->type&IMM) {
            if (op->type == (MEM|IMM)) {
                if (buf->prefix == FS)
                    printf("fs ");
                else printf("rip ");
            }
            if (op->imm > 0) printf("+ %lx ", op->imm);
            else if (op->imm < 0) printf("- %lx ",(~op->imm) + 1);
        }
        printf("] ");
    }
}

void print_x86_64(X86_64* buf) {
    printf(BLUE_COLOR"%s "GREEN_COLOR, instr_types[buf->type]);
    if (buf->dst.type) {
        print_op(buf, &buf->dst);
        if (buf->src.type) {
            print_op(buf, &buf->src);
        }
    }
    printf(RESET_COLOR"\n");
}