#include "../include/Decoder.hpp"
#include <cstdint>

uint8_t Decoder::fetch8() {
    return *(uint8_t*)guest++;
}
uint16_t Decoder::fetch16() {
    uint16_t* src = (uint16_t*)guest;
    guest += 2;
    return *src;
}
uint32_t Decoder::fetch32() {
    uint32_t* src = (uint32_t*)guest;
    guest += 4;
    return *src;
}
uint64_t Decoder::fetch64() {
    uint64_t* src = (uint64_t*)guest;
    guest += 8;
    return *src;
}

int64_t Decoder::fetch8_imm() {
    return (int64_t)fetch8();
}
int64_t Decoder::fetch16_imm() {
    return (int64_t)fetch16();
}
int64_t Decoder::fetch32_imm() {
    return (int64_t)fetch32();
}

void Decoder::fetch_imm(X86_64& buf) {
    buf.src.type = IMM;
    if (buf.size == 8) {
        buf.src.imm = fetch8_imm();
    } else if (buf.prefix == OS) {
        buf.size = 16;
        buf.src.imm = fetch16_imm();
    } else if (buf.size == 32) {
        buf.src.imm = fetch32_imm();
    } else buf.src.imm = fetch64();
}

void Decoder::decode_rm(Operand& op, uint8_t modrm) {
    uint8_t mod = modrm >> 6;
    uint8_t rm = modrm & 7;
    if (mod == 3) {
        op.type = REG;
        op.reg = rm;
        return;
    }
    op.type = MEM;
    if (rm == 4) {
        uint8_t sib = fetch8();
        op.reg = sib&7;
        op.idx = (sib>>3)&7;
        op.scale = sib>>6;
        if (op.idx != 4) op.type |= IDX;
        if (op.reg == 5 && mod == 0) {
            op.type |= IMM;
            op.imm = fetch32_imm();
        } else op.type |= REG;
    } else if (mod == 0 && rm == 5) {
        op.type |= IMM;
        op.imm = fetch32_imm();
    } else {
        op.type |= REG;
        op.reg = rm;
    }
    if (mod) {
        op.type |= IMM;
        if (mod == 2) op.imm = fetch32_imm();
        else op.imm = fetch8_imm();
    }
}
void Decoder::decode_rm_r(X86_64& buf) {
    uint8_t modrm = fetch8();
    buf.reverse = 1;
    buf.src.type = REG;
    buf.src.reg = (modrm >> 3) & 7;
    decode_rm(buf.dst, modrm);
}
void Decoder::decode_r_rm(X86_64& buf) {
    uint8_t modrm = fetch8();
    buf.dst.type = REG;
    buf.dst.reg = (modrm >> 3) & 7;
    decode_rm(buf.src, modrm);
}
void Decoder::decode_rex(X86_64& buf, uint8_t rex) {
    if (buf.reverse) {
        if (rex&4) buf.src.reg += 8;
        if (rex&2) buf.dst.idx += 8;
        if (rex&1) buf.dst.reg += 8;
    } else {
        if (rex&4) buf.dst.reg += 8;
        if (rex&2) buf.src.idx += 8;
        if (rex&1) buf.src.reg += 8;
    }
}

void Decoder::decode_GRP0(X86_64& buf, uint8_t byte) {
    if (!(byte&1)) buf.size = 8;
    uint8_t grp = byte&7;
    if (grp < 2) {
        decode_rm_r(buf);
    } else if (grp < 4) {
        decode_r_rm(buf);
    } else {
        buf.dst.type = REG;
        buf.dst.reg = 0;
        buf.src.type = IMM;
        fetch_imm(buf);
    }
}
// F3 - SS
// F2 - SD
// 66 - PD
// none - PS

void Decoder::decode_rm_r_XMM(X86_64& buf, bool xmm) {
    uint8_t modrm = fetch8();
    buf.reverse = 1;
    buf.src.type = REG;
    if (xmm) buf.src.type |= XMM;
    buf.src.reg = (modrm >> 3) & 7;
    decode_rm(buf.dst, modrm);
    buf.dst.type |= XMM;
}
void Decoder::decode_r_rm_XMM(X86_64& buf, bool xmm) {
    uint8_t modrm = fetch8();
    buf.dst.type = REG|XMM;
    buf.dst.reg = (modrm >> 3) & 7;
    decode_rm(buf.src, modrm);
    if (xmm && buf.src.type&REG) buf.src.type |= XMM;
}

void Decoder::set_guest(uint8_t* start) {
    guest = start;
}
void Decoder::decode(X86_64& buf) {
    buf.prefix = 0;
    buf.dst.type = NONE;
    buf.src.type = NONE;
    uint8_t rex = 0;
    uint8_t byte = fetch8();
    if ((byte >= FS && byte <= OS) ||
    (byte >= REPN && byte <= REPE)) {
        buf.prefix = byte;
        byte = fetch8();
    }
    if ((byte&0xF0) == 0x40) {
        rex = byte & 0xF;
        byte = fetch8();
    }
    if (rex&8) buf.size = 64;
    else buf.size = 32;
    if (byte != 0x0F) decode_00(buf, byte);
    else decode_0F(buf, fetch8());
    if (rex) decode_rex(buf, rex);
}
uint8_t* Decoder::get_guest() {
    return guest;
}