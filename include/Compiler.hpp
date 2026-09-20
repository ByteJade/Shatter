#pragma once

#include "Decoder.hpp"
#include <cstdint>
#include <vector>

extern const uint8_t x86_regs[];

struct Block {
    uint8_t* start;
    uint32_t buffer;
    uint32_t size;
};
struct Point {
    uint8_t* point;
    uint32_t* host;
};
struct Patch {
    uint32_t* host;
    uint8_t* guest;
};

class Compiler {
    Decoder decoder;
    std::vector<Block> blocks;
    std::vector<uint8_t> sizes;
    std::vector<X86_64> buffer;
    std::vector<Point> points;
    std::vector<Patch> patches;
    uint32_t reader;
    uint8_t* guest;
    bool need_entry;

    void jump(uint8_t* dst);
    bool has_block(uint8_t* p);
    bool forward();
    void decode(uint8_t* code);
    void iterate(Block& block);
    void patch();

    void emit_imm(int64_t imm, uint8_t dst);
    void emit_address(uint8_t dst, Operand& op, X86_64& buf);
    void emit_load(uint8_t dst, Operand& op, X86_64& buf, bool fast);
    void emit_store(uint8_t src, Operand& op, X86_64& buf, bool fast);
    void emit_math(X86_64& buf, uint32_t opcode, bool unsafe);
    void emit_branch(X86_64& buf, uint32_t opcode);
    void emit_mov(X86_64& buf);
    void emit_push(X86_64& buf);
    void emit_pop(X86_64& buf);
    void emit_patch(X86_64& buf);
    void emit_entry();
    void emit_ret();
    void encode(X86_64& buf);
    X86_64& next(int i);
    void skip(int i);
public:
    void compile(uint8_t* code);
};