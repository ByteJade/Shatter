#include "../inc/decoder.h"
#include "../inc/compiler.h"
#include "../inc/logger.h"
#include <unistd.h>

void decode_0F(compiler_t* compiler, X86_64* buf, uint8_t byte) {
    switch (byte) {
        case 0x05:
            buf->type = SYSCALL;
            break;
        case 0x1E:
            buf->type = EBR;
            fetch8(compiler);
            break;
        case 0x10:
            buf->type = MOVSS;
            decode_r_rm_XMM(compiler, buf,1);
            break;
        case 0x11:
            buf->type = MOVSS;
            decode_rm_r_XMM(compiler, buf,1);
            break;
        case 0x1F:
            buf->type = NOP;
            fetch8(compiler);
            break;
        case 0x28:
            buf->type = MOVAPD;
            decode_r_rm_XMM(compiler, buf,1);
            break;
        case 0x29:
            buf->type = MOVAPD;
            decode_rm_r_XMM(compiler, buf,1);
            break;
        case 0x2A:
            buf->type = CVTSI2SS;
            decode_r_rm_XMM(compiler, buf,0);
            break;
        case 0x2C:
            buf->type = CVTSS2SI;
            decode_rm_r_XMM(compiler, buf,0);
            break;
        case 0x2E:
            if (buf->type == 0x66) buf->type = UCOMISS;
            else buf->type = UCOMISD;
            decode_r_rm_XMM(compiler, buf,1);
            break;
        case 0x2F:
            if (buf->type == 0x66) buf->type = COMISS;
            else buf->type = COMISD;
            decode_r_rm_XMM(compiler, buf,1);
            break;
        case 0x40 ... 0x4F:
            buf->type = CMOVO + byte%0xF;
            decode_r_rm(compiler, buf);
            break;
        case 0x57:
        case 0xEF:
            buf->type = PXOR;
            decode_r_rm_XMM(compiler, buf,1);
            break;
        case 0x58:
            buf->type = ADDSS;
            decode_r_rm_XMM(compiler, buf,1);
            break;
        case 0x59:
            buf->type = MULSS;
            decode_r_rm_XMM(compiler, buf,1);
            break;
        case 0x5A:
            buf->type = CVTSS2SD;
            decode_r_rm_XMM(compiler, buf,1);
            break;
        case 0x5C:
            buf->type = SUBSS;
            decode_r_rm_XMM(compiler, buf,1);
            break;
        case 0x5E:
            buf->type = DIVSS;
            decode_r_rm_XMM(compiler, buf,1);
            break;
        case 0x6E:
            buf->type = MOVQ;
            decode_r_rm_XMM(compiler, buf,0);
            break;
        case 0x7E: {
            buf->type = MOVQ;
            uint8_t modrm = fetch8(compiler);
            decode_rm(compiler, &buf->dst, modrm);
            buf->src.type = REG|XMM;
            buf->src.reg = (modrm>>3)&7;
        } break;
        case 0x80 ... 0x8F:
            buf->type = JO + (byte&0xF);
            buf->dst.type = IMM;
            buf->dst.imm = fetch32_imm(compiler);
            break;
        case 0x90 ... 0x9F:
            buf->size = 8;
            buf->type = SETO + (byte&0xF);
            decode_rm(compiler, &buf->dst, fetch8(compiler));
            break;
        case 0xAF:
            buf->type = IMUL;
            decode_r_rm(compiler, buf);
            break;
        case 0xB6:
            buf->type = MOVZX8;
            decode_r_rm(compiler, buf);
            break;
        case 0xB7:
            buf->type = MOVZX16;
            decode_r_rm(compiler, buf);
            break;
        case 0xBE:
            buf->type = MOVSX8;
            decode_r_rm(compiler, buf);
            break;
        case 0xBF:
            buf->type = MOVSX16;
            decode_r_rm(compiler, buf);
            break;
        case 0xC1:
            buf->type = XADD;
            decode_rm_r(compiler, buf);
            break;
        default:
            logger_err("Unknown instruction: 0x0F 0x%X", byte);
            _exit(EXIT_FAILURE);
    }
}