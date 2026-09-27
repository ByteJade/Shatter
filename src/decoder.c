#include "../inc/decoder.h"
#include "../inc/compiler.h"

uint8_t fetch8(compiler_t* compiler) {
    return *(uint8_t*)compiler->guest++;
}
uint16_t fetch16(compiler_t* compiler) {
    uint16_t* src = (uint16_t*)compiler->guest;
    compiler->guest += 2;
    return *src;
}
uint32_t fetch32(compiler_t* compiler) {
    uint32_t* src = (uint32_t*)compiler->guest;
    compiler->guest += 4;
    return *src;
}
uint64_t fetch64(compiler_t* compiler) {
    uint64_t* src = (uint64_t*)compiler->guest;
    compiler->guest += 8;
    return *src;
}

int64_t fetch8_imm(compiler_t* compiler) {
    return (int64_t)(int8_t)fetch8(compiler);
}
int64_t fetch16_imm(compiler_t* compiler) {
    return (int64_t)(int16_t)fetch16(compiler);
}
int64_t fetch32_imm(compiler_t* compiler) {
    return (int64_t)(int32_t)fetch32(compiler);
}

void fetch_imm(compiler_t* compiler, X86_64* buf) {
    buf->src.type = IMM;
    if (buf->size == 8) {
        buf->src.imm = fetch8_imm(compiler);
    } else if (buf->prefix == OS) {
        buf->size = 16;
        buf->src.imm = fetch16_imm(compiler);
    } else if (buf->size == 32) {
        buf->src.imm = fetch32_imm(compiler);
    } else buf->src.imm = fetch64(compiler);
}

void decode_rm(compiler_t* compiler, operand_t* op, uint8_t modrm) {
    uint8_t mod = modrm >> 6;
    uint8_t rm = modrm & 7;
    if (mod == 3) {
        op->type = REG;
        op->reg = rm;
        return;
    }
    op->type = MEM;
    if (rm == 4) {
        uint8_t sib = fetch8(compiler);
        op->reg = sib&7;
        op->idx = (sib>>3)&7;
        op->scale = sib>>6;
        if (op->idx != 4) op->type |= IDX;
        if (op->reg == 5 && mod == 0) {
            op->type |= IMM;
            op->imm = fetch32_imm(compiler);
        } else op->type |= REG;
    } else if (mod == 0 && rm == 5) {
        op->type |= IMM;
        op->imm = fetch32_imm(compiler);
    } else {
        op->type |= REG;
        op->reg = rm;
    }
    if (mod) {
        op->type |= IMM;
        if (mod == 2) op->imm = fetch32_imm(compiler);
        else op->imm = fetch8_imm(compiler);
    }
}
void decode_rm_r(compiler_t* compiler, X86_64* buf) {
    uint8_t modrm = fetch8(compiler);
    buf->reverse = 1;
    buf->src.type = REG;
    buf->src.reg = (modrm >> 3) & 7;
    decode_rm(compiler, &buf->dst, modrm);
}
void decode_r_rm(compiler_t* compiler, X86_64* buf) {
    uint8_t modrm = fetch8(compiler);
    buf->dst.type = REG;
    buf->dst.reg = (modrm >> 3) & 7;
    decode_rm(compiler, &buf->src, modrm);
}
void decode_rex(X86_64* buf, uint8_t rex) {
    if (buf->reverse) {
        if (rex&4) buf->src.reg += 8;
        if (rex&2) buf->dst.idx += 8;
        if (rex&1) buf->dst.reg += 8;
    } else {
        if (rex&4) buf->dst.reg += 8;
        if (rex&2) buf->src.idx += 8;
        if (rex&1) buf->src.reg += 8;
    }
}

void decode_GRP0(compiler_t* compiler, X86_64* buf, uint8_t byte) {
    if (!(byte&1)) buf->size = 8;
    uint8_t grp = byte&7;
    if (grp < 2) {
        decode_rm_r(compiler, buf);
    } else if (grp < 4) {
        decode_r_rm(compiler, buf);
    } else {
        buf->dst.type = REG;
        buf->dst.reg = 0;
        buf->src.type = IMM;
        fetch_imm(compiler, buf);
    }
}
void decode_rm_r_XMM(compiler_t* compiler, X86_64* buf, int xmm) {
    uint8_t modrm = fetch8(compiler);
    buf->reverse = 1;
    buf->src.type = REG;
    if (xmm) buf->src.type |= XMM;
    buf->src.reg = (modrm >> 3) & 7;
    decode_rm(compiler, &buf->dst, modrm);
    buf->dst.type |= XMM;
}
void decode_r_rm_XMM(compiler_t* compiler, X86_64* buf, int xmm) {
    uint8_t modrm = fetch8(compiler);
    buf->dst.type = REG|XMM;
    buf->dst.reg = (modrm >> 3) & 7;
    decode_rm(compiler, &buf->src, modrm);
    if (xmm && buf->src.type&REG) buf->src.type |= XMM;
}

void decode(compiler_t* compiler, X86_64* buf) {
    buf->prefix = 0;
    buf->reverse = 0;
    buf->dst.type = NONE;
    buf->src.type = NONE;
    uint8_t rex = 0;
    uint8_t byte = fetch8(compiler);
    if ((byte >= FS && byte <= OS) ||
    (byte >= REPN && byte <= REPE)) {
        buf->prefix = byte;
        byte = fetch8(compiler);
    }
    if ((byte&0xF0) == 0x40) {
        rex = byte & 0xF;
        byte = fetch8(compiler);
    }
    if (rex&8) buf->size = 64;
    else buf->size = 32;
    if (byte != 0x0F) decode_00(compiler, buf, byte);
    else decode_0F(compiler, buf, fetch8(compiler));
    if (rex) decode_rex(buf, rex);
}
