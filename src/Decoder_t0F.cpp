#include "../include/Decoder.hpp"
#include "../include/Logger.hpp"
#include <cstdint>

void Decoder::decode_0F(X86_64& buf, uint8_t byte) {
    switch (byte) {
        case 0x05:
            buf.type = SYSCALL;
            break;
        case 0x1E:
            buf.type = EBR;
            fetch8();
            break;
        case 0x10:
            buf.type = MOVX;
            decode_r_rm_XMM(buf,1);
            break;
        case 0x11:
            buf.type = MOVX;
            decode_r_rm_XMM(buf,1);
            break;
        case 0x1F:
            buf.type = NOP;
            fetch8();
            break;
        case 0x28:
            buf.type = MOVAPX;
            decode_r_rm_XMM(buf,1);
            break;
        case 0x29:
            buf.type = MOVAPX;
            decode_r_rm_XMM(buf,1);
            break;
        case 0x2A:
            buf.type = CVTSI2X;
            decode_r_rm_XMM(buf,0);
            break;
        case 0x2C:
            buf.type = CVTX2SI;
            decode_rm_r_XMM(buf,0);
            break;
        case 0x2E:
            buf.type = UCOMIX;
            decode_r_rm_XMM(buf,1);
            break;
        case 0x2F:
            buf.type = COMIX;
            decode_r_rm_XMM(buf,1);
            break;
        case 0x40 ... 0x4F:
            buf.type = CMOVO + byte%0xF;
            decode_r_rm(buf);
            break;
        case 0x57:
        case 0xEF:
            buf.type = PXOR;
            decode_r_rm_XMM(buf,1);
            break;
        case 0x58:
            buf.type = ADDX;
            decode_r_rm_XMM(buf,1);
            break;
        case 0x59:
            buf.type = MULX;
            decode_r_rm_XMM(buf,1);
            break;
        case 0x5A:
            buf.type = CVTX;
            decode_r_rm_XMM(buf,1);
            break;
        case 0x5C:
            buf.type = SUBX;
            decode_r_rm_XMM(buf,1);
            break;
        case 0x5E:
            buf.type = DIVX;
            decode_r_rm_XMM(buf,1);
            break;
        case 0x6E:
            buf.type = MOVQ;
            decode_r_rm_XMM(buf,0);
            break;
        case 0x7E: {
            buf.type = MOVQ;
            uint8_t modrm = fetch8();
            decode_rm(buf.dst, modrm);
            buf.src.type = REG|XMM;
            buf.src.reg = (modrm>>3)&7;
        } break;
        case 0x80 ... 0x8F:
            buf.type = JO + (byte%0xF);
            buf.dst.type = IMM;
            buf.dst.imm = fetch32_imm();
            break;
        case 0x90 ... 0x9F:
            buf.size = 8;
            buf.type = SETO + (byte&0xF);
            decode_rm(buf.dst, fetch8());
            break;
        case 0xAF:
            buf.type = IMUL;
            decode_r_rm(buf);
            break;
        case 0xB6:
            buf.type = MOVZX8;
            decode_r_rm(buf);
            break;
        case 0xB7:
            buf.type = MOVZX16;
            decode_r_rm(buf);
            break;
        case 0xBE:
            buf.type = MOVSX8;
            decode_r_rm(buf);
            break;
        case 0xBF:
            buf.type = MOVSX16;
            decode_r_rm(buf);
            break;
        default:
            logger.err() << "Unknown instruction: 0x0F " << (int)byte << std::endl;
            exit(EXIT_FAILURE);
    }
}