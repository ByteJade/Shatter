#include "../inc/encoder.h"
#include "../inc/compiler.h"
#include "../inc/dynarray.h"
#include "../inc/cache.h"
#include "../inc/logger.h"
#include "../inc/printer_X86_64.h"
#include <stdint.h>

#define SC1R 10
#define SC2R 11
#define SC3R 12
#define XZR 31
#define TSP 28

const uint8_t x86_regs[] = {
    8, 3, 2, 19,
    TSP, 29, 1, 0,
    4, 5, 6, 7,
    20, 21, 22, 23,
};
uint32_t math_group[] = {
    ADDS_R, ORR_R, ADCS_R, SBCS_R,
    ANDS_R, SUBS_R, EOR_R, SUBS_R,
};
uint32_t neon_group[] = {
    EOR_N,
    ADD_N, NSF|ADD_N,
    MUL_N, NSF|MUL_N,
    SUB_N, NSF|SUB_N,
    DIV_N, NSF|DIV_N,
    FCVTU_N, FCVT_N,
    CMP_N, NSF|CMP_N,
    CMPE_N, NSF|CMPE_N,
};
uint32_t cset_group[] = {
    CSETB, CSETAE,
    CSETE, CSETNE,
    CSETBE, CSETA,
    CSETS, CSETNS,
    UNK, UNK,
    CSETL, CSETGE,
};
uint32_t csel_group[] = {
    CSELB, CSELAE,
    CSELE, CSELNE,
    CSELBE, CSELA,
    CSELS, CSELNS,
    UNK, UNK,
    CSELL, CSELGE,
};
uint32_t jcc_group[] = {
    UNK, UNK,
    BLO, BCS,
    BEQ, BNE,
    BLS, BHI,
    BMI, BPL,
    UNK, UNK,
    BLT, BGE,
    BLE, BGT,
};

void emit_imm(int64_t imm, uint8_t dst) {
    if (imm >= 0 && imm <= INT16_MAX) {
        cache_emit(ASF|MOVZ_I | (imm << 5) | dst);
    } else if (imm < 0 && ~imm <= INT16_MAX) {
        cache_emit(ASF|MOVN_I | ((((~imm)&0xFFFF) << 5)) | dst);
    } else {
        uint16_t a = imm & 0xFFFF;
        uint16_t b = (imm>>16) & 0xFFFF;
        cache_emit(ASF|MOVZ_I | (a << 5) | dst);
        if (b) cache_emit(ASF|MOVK_I | (1 << 21) | (b << 5) | dst);
        if (imm >= INT32_MIN && imm <= INT32_MAX) {
            if (imm < 0) cache_emit(SXTW | (dst << 5) | dst);
            return;
        }
        uint16_t c = (imm>>32) & 0xFFFF;
        uint16_t d = imm>>48;
        if (c) cache_emit(ASF|MOVK_I | (2 << 21) | (c << 5) | dst);
        if (d) cache_emit(ASF|MOVK_I | (3 << 21) | (d << 5) | dst);
    }
}
void emit_add_signed(uint8_t dst, uint8_t src, int64_t imm) {
    if (imm > 4095 || imm < -4096) {
        emit_imm(imm, SC2R);
        cache_emit(ASF|ADD_R | (dst) | (src<<5) | (SC2R<<16));
    } else {
        if (imm > 0)
            cache_emit(ASF|ADD_I | (dst) | (src<<5) | (imm<<10));
        else cache_emit(ASF|SUB_I | (dst) | (src<<5) | (-imm<<10));
    }
}
uint16_t emit_rip(compiler_t* compiler, operand_t* op, uint8_t dst) {
    uint64_t full = (uint64_t)(compiler->guest + op->imm);
    int64_t target = full & ~0xFFF;
    int64_t current = (uint64_t)(cache_get_host()) & ~0xFFF;
    int64_t delta = (target - current) >> 12;
    if (delta < -4294967296LL || delta > 4294967296LL) {
        logger_err("Too large rip distance");
    }
    cache_emit(ADRP | ((delta & 0x3) << 29) | (((delta >> 2) & 0x7FFFF) << 5) | dst);
    return full & 0xFFF;
}
void emit_address(compiler_t* compiler, uint8_t dst, operand_t* op, X86_64* buf) {
    uint8_t t = op->type;
    if (op->type == (MEM|IMM)) {
        int full;
        if (buf->prefix == FS) {
            cache_emit(GET_FS | dst);
            full = op->imm;
        } else full = emit_rip(compiler, op, dst);
        if (full) emit_add_signed(dst, dst, full);
    } else if (op->type&IDX) {
        if (op->scale != 0) {
            cache_emit(UBFM | ((-(op->scale) & 0x3F) << 16) |
            (((63 - op->scale) & 0x3F) << 10) | (x86_regs[op->idx] << 5) | dst);
        } else cache_emit(ASF|ADD_I | (dst) | (x86_regs[op->idx]<<5));
        if (t&REG) {
            cache_emit(ASF|ADD_R | (dst) | (dst<<5) | (x86_regs[op->reg]<<16));
        }
        if ((t&IMM) && op->imm != 0) {
            emit_add_signed(dst, dst, op->imm);
        }
    }else {
        if (t&IMM) {
            emit_add_signed(dst, x86_regs[op->reg], op->imm);
        } else {
            cache_emit(ASF|ADD_I | (dst) | (x86_regs[op->reg]<<5));
        }
    }
}
void emit_mem(compiler_t* compiler, uint32_t grp, operand_t* op, X86_64* buf, int fast) {
    switch (buf->size) {
        case 64: grp |= D_GRP; break;
        case 32: grp |= W_GRP; break;
        case 16: grp |= H_GRP; break;
        case 8:  grp |= B_GRP; break;
    }
    if (op->type == (MEM|REG|IMM) &&
        op->imm >= -256 && op->imm <= 255) {
        cache_emit(grp | ((op->imm&0x1FF)<<12)|(x86_regs[op->reg]<<5));
        return;
    }
    grp |= U_GRP;
    if (op->type == (MEM|REG)) {
        cache_emit(grp | (x86_regs[op->reg]<<5));
    }  else if (op->type == (MEM|IMM)) {
        int full;
        if (buf->prefix == FS) {
            if (!fast) cache_emit(GET_FS | SC1R);
            full = op->imm;
        } else {
            if (!fast) full = emit_rip(compiler, op, SC1R);
            else full = (uint64_t)(compiler->guest + op->imm) & 0xFFF;
        }
        full /= buf->size/8;
        cache_emit(grp | (full << 10) | (SC1R<<5));
    } else {
        if (!fast) emit_address(compiler, SC1R, op, buf);
        cache_emit(grp | (SC1R<<5));
    }
}
void emit_math(compiler_t* compiler, X86_64* buf, uint32_t opcode) {
    // ADD, SUB, OR, XOR, AND
    uint8_t src;
    uint8_t dst;
    if (buf->src.type == IMM) {
        if (buf->src.imm) {
            emit_imm(buf->src.imm, SC3R);
            src = SC3R;
        } else src = XZR;
    } else if (buf->src.type == REG) {
        src = x86_regs[buf->src.reg];
    } else {
        emit_mem(compiler, LD_GRP|SC2R, &buf->src, buf, 0);
        src = SC2R;
    }
    if (buf->dst.type&MEM) {
        emit_mem(compiler, LD_GRP|SC2R, &buf->dst, buf, 0);
        if (buf->type == XADD) cache_emit(ADD_I | x86_regs[buf->dst.reg] | SC2R);
        dst = SC2R;
    } else dst = x86_regs[buf->dst.reg];
    
    uint32_t sf = (buf->size == 64) * ASF;
    if (buf->size < 32) logger_err("TODO: 8, 16 bit instructions");
    if (buf->type == CMP || buf->type == TEST) {
        cache_emit(sf | opcode | XZR | (dst<<5) | (src<<16));
    } else if (buf->type == NEG) {
        cache_emit(sf | opcode | (dst) | XZR | (src<<16));
    } else {
        cache_emit(sf | opcode | dst | (dst<<5) | (src<<16));
        if (buf->dst.type&MEM) emit_mem(compiler, ST_GRP|dst, &buf->dst, buf, 1);
    }
}
void emit_shift(compiler_t* compiler, X86_64* buf, uint32_t opcode) {
    uint8_t src;
    uint8_t dst = x86_regs[buf->dst.reg];
    if (buf->src.type == IMM) {
        emit_imm(buf->src.imm, SC3R);
        src = SC3R;
    } else src = x86_regs[buf->src.reg];
    uint32_t sf = (buf->size == 64) * ASF;
    cache_emit(sf | opcode | dst | (dst<<5) | (src<<16));
}
void emit_neon(compiler_t* compiler, X86_64* buf, uint32_t opcode) {
    uint8_t dst = buf->dst.reg;
    uint8_t src = buf->src.reg;
    if (buf->type%2) buf->size = 64;
    if (buf->dst.type & MEM) {
        emit_mem(compiler, LDN_GRP|16, &buf->dst, buf, 0);
        dst = 16;
    } else if (buf->src.type & MEM) {
        emit_mem(compiler, LDN_GRP|16, &buf->src, buf, 0);
        src = 16;
    }
    if (buf->type >= UCOMISS)
        cache_emit(opcode | (dst<<5) | (src<<16));
    else if (buf->type >= CVTSS2SD)
        cache_emit(opcode | (dst) | (src<<5));
    else cache_emit(opcode | (dst) | (dst<<5) | (src<<16));
    if (buf->dst.type & MEM) emit_mem(compiler, STN_GRP|16, &buf->dst, buf, 1);
}
void emit_patch(compiler_t* compiler, X86_64* buf) {
    size_t patch_p = dynarray_push((void**)&compiler->patches);
    patch_t* patch = compiler->patches + patch_p;
    patch->host = cache_get_host();
    patch->guest = compiler->guest + buf->dst.imm;
    cache_emit(buf->type);
}
void emit_branch(compiler_t* compiler, X86_64* buf, uint32_t opcode) {
    if (buf->dst.type == IMM) {
        if (buf->type == CALL) {
            if (prev(compiler)->type != CALL) cache_emit(ASF|ADD_I | 31 | (TSP<<5));
            cache_emit(BRK | (cache_set_patch(compiler->guest + buf->dst.imm)<<5));
        } else emit_patch(compiler, buf);
    } else {
        uint8_t dst;
        if (buf->dst.type&MEM) {
            emit_mem(compiler, LD_GRP|SC1R, &buf->dst, buf, 0);
            dst = SC1R;
        } else dst = x86_regs[buf->dst.reg];
        cache_emit(opcode | (dst << 5));
    }
}
void emit_mov(compiler_t* compiler, X86_64* buf, int sx) {
    if (buf->dst.type == REG) {
        uint8_t dst = x86_regs[buf->dst.reg];
        if (buf->src.type == IMM) {
            emit_imm(buf->src.imm, dst);
        } else if (buf->src.type&MEM) {
            emit_mem(compiler, LD_GRP|(SX_GRP*sx)|dst, &buf->src, buf, 0);
        } else if (sx) {
            cache_emit(SBFM | dst | (x86_regs[buf->src.reg]<<5) | ((buf->size-1) << 10));
        } else {
            cache_emit(ASF|ADD_I | dst | (x86_regs[buf->src.reg]<<5));
        }
    } else {
        uint8_t src;
        if (buf->src.type == IMM) {
            if (buf->src.imm) {
                emit_imm(buf->src.imm, SC3R);
                src = SC3R;
            } else src = XZR;
        } else src = x86_regs[buf->src.reg];
        emit_mem(compiler, ST_GRP|src, &buf->dst, buf, 0);
    }
}
int emit_pre_push(compiler_t* compiler, X86_64* buf, int shift) {
    int dst = SC2R + shift;
    if (buf->dst.type == IMM) {
        emit_imm(buf->dst.imm, dst);
        return dst;
    } if (buf->dst.type&MEM) {
        emit_mem(compiler, LD_GRP|dst, &buf->dst, buf, 0);
        return dst;
    } if (buf->dst.reg == RSP) {
        cache_emit(ADD_I | dst | (TSP<<5));
        return dst;
    }
    return x86_regs[buf->dst.reg];
}
void emit_push(compiler_t* compiler, X86_64* buf) {
    uint8_t dst = emit_pre_push(compiler, buf, 0);
    X86_64* n = next(compiler);
    if (n->type == PUSH) {
        skip(compiler);
        uint8_t dst1 = emit_pre_push(compiler, n, 1);
        cache_emit(ASF|STP_PRE | ((-16&0x3FE)<<12) | dst1 | (TSP<<5) | (dst<<10));
        return;
    }else cache_emit(MSF|STR_PRE | ((-8&0x1FF)<<12) | dst | (TSP<<5));
}
void emit_pop(compiler_t* compiler, X86_64* buf) {
    if (buf->dst.type&MEM) {
        logger_err("pop []; not supported");
    } else {
        uint8_t dst = x86_regs[buf->dst.reg];
        X86_64* n = next(compiler);
        if (n->type == POP && n->dst.type == REG) {
            skip(compiler);
            if (next(compiler)->type == RET)
                cache_emit(MSF|LDR_POST | (8<<12) | dst | (TSP<<5));
            else {
                uint8_t dst1 = x86_regs[n->dst.reg];
                cache_emit(ASF|LDP_POST | (16<<12) | dst | (TSP<<5) | (dst1<<10));
            }
        }else if (n->type != RET) cache_emit(MSF|LDR_POST | (8<<12) | dst | (TSP<<5));
    }
}
void emit_entry(struct compiler_t* compiler) {
    if (compiler->flags&NEED_STACK)
        cache_emit(ASF|ADD_I | TSP | (31<<5));
    if (compiler->flags&NEED_ENTRY) {
        X86_64* n = next(compiler);
        if (n->type == PUSH) {
            skip(compiler);
            uint8_t dst1 = emit_pre_push(compiler, n, 1);
            cache_emit(ASF|STP_PRE | ((-16&0x3FE)<<12) | dst1 | (TSP<<5) | (30<<10));
        }else cache_emit(MSF|STR_PRE | ((-8&0x1FF)<<12) | (TSP<<5) | 30);
    }
}
void emit_ret(struct compiler_t* compiler) {
    X86_64* n = prev(compiler);
    if (n->type == POP) {
        uint8_t dst1 = x86_regs[n->dst.reg];
        cache_emit(ASF|LDP_POST | (16<<12) | dst1 | (TSP<<5) | (30<<10));
    } else if (n->type == LEAVE) {
        cache_emit(ASF|ADD_I | TSP | (29<<5));
        cache_emit(ASF|LDP_POST | (16<<12) | 29 | (TSP<<5) | (30<<10));
    } else cache_emit(MSF|LDR_POST | (8<<12) | 30 | (TSP<<5));
    cache_emit(ASF|ADD_I | 31 | (TSP<<5));
    cache_emit(RET_R | (30 << 5));
}
void emit_jump(uint32_t* dst, uint32_t* target) {
    int64_t delta = target - dst;
    uint32_t type = *dst;
    logger_log("patch %s", instr_types[type]);
    if (type == JMP) {
        *dst = B | (delta & 0x3FFFFFF);
    } else {
        *dst = jcc_group[type - JO] | ((delta & 0x7FFFF) << 5);
    }
}
void emit_call(uint32_t* dst, uint32_t* target) {
    int32_t offset = target - dst;
    *dst = BL | (offset & 0x3FFFFFF);
    __builtin___clear_cache(dst, dst+1);
}
void encode(compiler_t* compiler, X86_64* buf) {
    switch (buf->type) {
        case MOVZX8: buf->size = 8; emit_mov(compiler, buf, 0); break;
        case MOVZX16: buf->size = 16; emit_mov(compiler, buf, 0); break;
        case MOVSX8: buf->size = 8; emit_mov(compiler, buf, 1); break;
        case MOVSX16: buf->size = 16; emit_mov(compiler, buf, 1); break;
        case MOV: emit_mov(compiler, buf, 0); break;
        case PUSH: emit_push(compiler, buf); break;
        case POP: emit_pop(compiler, buf); break;
        case LEA: emit_address(compiler, x86_regs[buf->dst.reg], &buf->src, buf); break;
        case XADD: emit_math(compiler, buf, ADDS_R); break;
        case ADD:
            if (buf->dst.type == REG && buf->src.type == IMM) {
                uint8_t dst = x86_regs[buf->dst.reg];
                emit_add_signed(dst, dst, buf->src.imm);
            } else {
                emit_math(compiler, buf, ADDS_R);
            } break;
        case SUB:
            if (buf->dst.type == REG && buf->src.type == IMM) {
                uint8_t dst = x86_regs[buf->dst.reg];
                emit_add_signed(dst, dst, -buf->src.imm);
            } else {
                emit_math(compiler, buf, SUBS_R);
            } break;
        case OR: case ADC: case SBB:
        case AND: case XOR: case CMP: {
            uint32_t opcode = math_group[buf->type - ADD];
            emit_math(compiler, buf, opcode);
        } break;
        case PXOR ... COMISD: {
            uint32_t opcode = neon_group[buf->type - PXOR];
            emit_neon(compiler, buf, opcode);
        } break;
        case SETB ... SETGE: {
            uint32_t opcode = cset_group[buf->type - SETB];
            cache_emit(opcode | x86_regs[buf->dst.reg]);
        } break;
        case CMOVB ... CMOVGE: {
            uint32_t opcode = csel_group[buf->type - CMOVB];
            uint8_t dst = x86_regs[buf->dst.reg];
            uint8_t src = x86_regs[buf->src.reg];
            uint32_t sf = ASF * (buf->size == 64);
            cache_emit(sf | opcode | dst | (src << 5) | (dst << 16));
        } break;
        case CVTSS2SI: 
        case CVTSD2SI: {
            uint32_t prefix = NSF * (buf->prefix == REPN);
            cache_emit(prefix | FCVTZS | (x86_regs[buf->dst.reg]) | (buf->src.reg << 5));
        } break;
        case CVTSI2SD:
        case CVTSI2SS:{
            uint32_t prefix = (ASF|NSF) * (buf->prefix == REPN);
            if (buf->src.type&MEM) {
                emit_mem(compiler, LD_GRP|SC1R, &buf->src, buf, 0);
                cache_emit(prefix|SCVTF_N | (buf->dst.reg) | (SC1R << 5));
            } else {
                cache_emit(prefix|SCVTF_N | (buf->dst.reg) | (x86_regs[buf->src.reg]<<5));
            }
        } break;
        case MOVSX:
            cache_emit(0x93407c00 | (x86_regs[buf->src.reg]<<5) | (x86_regs[buf->dst.reg]));
            break;
        case MOVQ:
            if (buf->src.type&XMM) cache_emit(FMOV_N | (x86_regs[buf->dst.reg]) | (buf->src.reg << 5));
            else cache_emit(FMOVR_N | (buf->dst.reg) | (x86_regs[buf->src.reg] << 5));
            break;
        case MOVSD: 
            buf->size = 64;
            [[fallthrough]];
        case MOVSS:{
            if (buf->dst.type & MEM)
                emit_mem(compiler, STN_GRP|buf->src.reg, &buf->dst, buf, 0);
            else emit_mem(compiler, LDN_GRP|buf->dst.reg, &buf->src, buf, 0);
        } break;
        case MOVAPD: {
            buf->size = 64;
            uint8_t src = buf->src.reg;
            if (buf->dst.type & MEM) {
                emit_mem(compiler, STN_GRP|src, &buf->dst, buf, 0);
            }else {
                cache_emit(MOV_N | (buf->dst.reg) | (src << 5) | (src << 16));
            }
        } break;
        case IDIV: {
            uint8_t src = SC2R;
            if (buf->src.type&MEM) {
                emit_mem(compiler, LD_GRP|SC2R, &buf->src, buf, 0);
            } else src = x86_regs[buf->src.reg];
            uint8_t dst = x86_regs[buf->dst.reg];
            uint32_t sf = ASF * (buf->size == 64);
            cache_emit(sf|ADD_I | SC1R | (dst<<5));
            cache_emit(sf|SDIV_R | (src<<16) | (SC1R<<5) | (dst));
            cache_emit(sf|MSUB_R | (src<<16) | (SC1R<<10) | (dst << 5) | 2);
        } break;
        case IMUL: {
            uint8_t src = SC2R;
            uint8_t dst = x86_regs[buf->dst.reg];
            if (buf->src.type&MEM) {
                emit_mem(compiler, LD_GRP|SC2R, &buf->src, buf, 0);
            } else src = x86_regs[buf->src.reg];
            if (buf->dst.type == REG) {
                cache_emit(SMUL_R | (src<<16) | (dst<<5) | (dst));
                cache_emit(0x9360FC00 | 2 | (8 << 5));
            } else {
                emit_imm(buf->dst.imm, SC1R);
                cache_emit(SMUL_R | (SC1R<<16) | (src<<5) | dst);
            }
        } break;
        case TEST: emit_math(compiler, buf, ANDS_R); break;
        case NEG: emit_math(compiler, buf, SUBS_R); break;
        case ROR: emit_shift(compiler, buf, ROR_R); break;
        case SHL:
        case SHR: emit_shift(compiler, buf, LSR_R); break;
        case SAL: emit_shift(compiler, buf, LSL_R); break;
        case SAR: emit_shift(compiler, buf, ASR_R); break;
        case EBR: case NOP: case HLT: case LEAVE: break;
        case JMP: emit_branch(compiler, buf, BR); break;
        case CALL: emit_branch(compiler, buf, BLR); break;
        case RET: emit_ret(compiler); break;
        case JO ... JG: emit_patch(compiler, buf); break;
        case CLTQ: case CLTD:
            cache_emit(SXTW | (x86_regs[RAX] << 5) | x86_regs[RAX]);
            if (buf->type == CLTD) cache_emit(0x937ffc00 | (x86_regs[RAX] << 5) | x86_regs[RDX]); // asr x2, x8, #63
            break;
        default:
            logger_err("Unknown encode: %i", buf->type);
            exit(0);
    }
}