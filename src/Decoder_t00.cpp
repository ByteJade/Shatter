#include "../include/Decoder.hpp"
#include "../include/Logger.hpp"
#include <cstdlib>

void Decoder::decode_00(X86_64& buf, uint8_t byte) {
    switch (byte) {
        case 0x00 ... 0x3F:
            if ((byte&7) > 4) goto error;
            buf.type = ADD + ((byte >> 3) & 7);
            decode_GRP0(buf, byte);
            break;
        case 0x50 ... 0x57:
            buf.size = 64;
            buf.reverse = 1;
            buf.type = PUSH;
            buf.dst.type = REG;
            buf.dst.reg = byte&7;
            break;
        case 0x58 ... 0x5F:
            buf.size = 64;
            buf.reverse = 1;
            buf.type = POP;
            buf.dst.type = REG;
            buf.dst.reg = byte&7;
            break;
        case 0x63:
            buf.type = MOVSX;
            decode_r_rm(buf);
            break;
        case 0x68:
            buf.type = PUSH;
            buf.dst.type = IMM;
            buf.dst.imm = fetch32_imm();
            break;
        case 0x69:
            buf.type = IMUL;
            decode_r_rm(buf);
            buf.src.imm = fetch32_imm();
            break;
        case 0x6A:
            buf.type = PUSH;
            buf.dst.type = IMM;
            buf.dst.imm = fetch8_imm();
            break;
        case 0x70 ... 0x7F:
            buf.type = JO + byte - 0x70;
            buf.dst.type = IMM;
            buf.dst.imm = fetch8_imm();
            break;
        case 0x80: case 0x82:
            buf.size = 8;
            [[fallthrough]];
        case 0x81: case 0x83:{
            uint8_t modrm = fetch8();
            buf.reverse = 1;
            buf.type = ADD + ((modrm >> 3) & 7);
            decode_rm(buf.dst, modrm);
            if (byte == 0x83) {
                buf.src.type = IMM;
                buf.src.imm = fetch8_imm();
            } else fetch_imm(buf);
        } break;
        case 0x84:
            buf.size = 8;
            [[fallthrough]];
        case 0x85:
            buf.type = TEST;
            decode_rm_r(buf);
            break;
        case 0x88 ... 0x8B:
            buf.type = MOV;
            decode_GRP0(buf, byte);
            break;
        case 0x8D:
            buf.type = LEA;
            decode_r_rm(buf);
            break;
        case 0x8F:
            buf.size = 64;
            buf.reverse = 1;
            buf.type = POP;
            decode_rm(buf.dst, fetch8());
            break;
        case 0x90: 
            buf.type = NOP;
            break;
        case 0x98:
            buf.type = CLTQ;
            break;
        case 0x99:
            buf.type = CLTD;
            break;
        case 0xB0 ... 0xB7:
            buf.size = 8;
            [[fallthrough]];
        case 0xB8 ... 0xBF:
            buf.reverse = 1;
            buf.type = MOV;
            buf.dst.type = REG;
            buf.dst.reg = byte&7;
            fetch_imm(buf);
            break;
        case 0xC0: case 0xC1:
        case 0xD0: case 0xD1:
        case 0xD2: case 0xD3:{
            if (!(byte&1)) buf.size = 8;
            uint8_t modrm = fetch8();
            buf.reverse = 1;
            buf.type = ROL + ((modrm >> 3) & 7);
            decode_rm(buf.dst, modrm);
            if (byte < 0xD2) {
                buf.src.type = IMM;
                if (byte > 0xC1) buf.src.imm = 1;
                else buf.src.imm = fetch8_imm();
            } else {
                buf.src.type = REG;
                buf.src.reg = RCX;
            }
        } break;
        case 0xF4:
        case 0xC3:
            buf.type = RET;
            break;
        case 0xC6:
            buf.size = 8;
            buf.reverse = 1;
            buf.type = MOV;
            decode_rm(buf.dst, fetch8());
            buf.src.type = IMM;
            buf.src.imm = fetch8_imm();
            break;
        case 0xC7:
            buf.reverse = 1;
            buf.type = MOV;
            decode_rm(buf.dst, fetch8());
            fetch_imm(buf);
            break;
        case 0xC9:
            buf.type = LEAVE;
            break;
        case 0xE8:
        case 0xE9:
            buf.type = CALL + (byte - 0xE8);
            buf.dst.type = IMM;
            buf.dst.imm = fetch32_imm();
            break;
        case 0xEB:
            buf.type = JMP;
            buf.dst.type = IMM;
            buf.dst.imm = fetch8_imm();
            break;
        case 0xF7: {
            uint8_t modrm = fetch8();
            uint8_t code = (modrm >> 3) & 7;
            buf.type = TEST + code;
            buf.dst.type = REG;
            buf.dst.reg = RAX;
            decode_rm(buf.src, modrm);
        } break;
        case 0xFE:
            buf.size = 8;
            [[fallthrough]];
        case 0xFF: {
            buf.reverse = 1;
            if (byte != 0xFE) buf.size = 64;
            uint8_t modrm = fetch8();
            uint8_t code = (modrm >> 3) & 7;
            decode_rm(buf.dst, modrm);
            buf.type = INC + code;
        } break;
        default: error:
            logger.err() << "Unknown instruction: " << (int)byte << std::endl;
            exit(EXIT_FAILURE);
    }
}