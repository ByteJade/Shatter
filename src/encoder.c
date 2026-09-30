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

void emit_imm(int64_t imm, uint8_t dst) {
    if (imm >= 0 && imm <= INT16_MAX) {
        cache_emit(ASF|MOVZ_I | (imm << 5) | dst);
    } else if (imm < 0 && ~imm <= INT16_MAX) {
        cache_emit(ASF|MOVN_I | ((~imm << 5)&0xFFFF) | dst);
    } else {
        uint16_t a = imm & 0xFFFF;
        uint16_t b = (imm>>16) & 0xFFFF;
        cache_emit(ASF|MOVZ_I | (a << 5) | dst);
        if (b) cache_emit(ASF|MOVK_I | (1 << 21) | (b << 5) | dst);
        if (imm >= INT32_MIN && imm <= INT32_MAX) {
            //if (imm < 0) cache.emit(SXTW_REG | (dst << 5) | dst);
            return;
        }
        uint16_t c = (imm>>32) & 0xFFFF;
        uint16_t d = imm>>48;
        if (c) cache_emit(ASF|MOVK_I | (2 << 21) | (c << 5) | dst);
        if (d) cache_emit(ASF|MOVK_I | (3 << 21) | (d << 5) | dst);
    }
}
void emit_add_signed(uint8_t dst, uint8_t src, int64_t imm) {
    if (imm > 0)
        cache_emit(ASF|ADD_I | (dst) | (src<<5) | (imm<<10));
    else cache_emit(ASF|SUB_I | (dst) | (src<<5) | (-imm<<10));
}
void emit_address(compiler_t* compiler, uint8_t dst, operand_t* op, X86_64* buf) {
    uint8_t t = op->type;
    if (buf->prefix == FS) {
        cache_emit(GET_FS | dst);
        emit_add_signed(dst, dst, op->imm);
    } else if (op->type == (MEM|IMM)) {
        uint64_t full = (uint64_t)(compiler->guest + op->imm);
        int64_t target = full & ~0xFFF;
        int64_t current = (uint64_t)(cache_get_host()) & ~0xFFF;
        int64_t delta = (target - current) >> 12;
        if (delta < -4294967296LL || delta > 4294967296LL) {
            logger_err("Too large rip distance");
        }
        cache_emit(ADRP | ((delta & 0x3) << 29) | (((delta >> 2) & 0x7FFFF) << 5) | dst);
        if (full & 0xFFF) cache_emit((ASF|ADD_I | ((full & 0xFFF) << 10) | (dst << 5) | dst));
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
void emit_load(compiler_t* compiler, uint8_t dst, operand_t* op, X86_64* buf, int fast, int sx) {
    uint32_t instruction = LDUR;
    switch (buf->size) {
        case 64: instruction |= MSF; break;
        case 16: instruction = LDURH; break;
        case 8: instruction = LDURB; break;
    }
    if (sx) instruction |= 9 << 20;
    if (op->type == (MEM|REG|IMM) &&
        op->imm > -256 &&
        op->imm < 255) {
        cache_emit(instruction | ((op->imm&0x1FF)<<12)|(x86_regs[op->reg]<<5)|dst);
        return;
    }
    instruction |= 1 << 24; // LDUR -> LDR
    if (op->type == (MEM|REG)) {
        cache_emit(instruction | (x86_regs[op->reg]<<5) | dst);
    } else {
        if (!fast) emit_address(compiler, SC1R, op, buf);
        cache_emit(instruction | (SC1R<<5) | dst);
    }
}
void emit_store(compiler_t* compiler, uint8_t src, operand_t* op, X86_64* buf, int fast) {
    uint32_t instruction = STUR;
    switch (buf->size) {
        case 64: instruction |= MSF; break;
        case 16: instruction = STURH; break;
        case 8: instruction = STURB; break;
    }
    if (op->type == (MEM|REG|IMM) &&
        op->imm > -256 &&
        op->imm < 255) {
        cache_emit(instruction | ((op->imm&0x1FF)<<12)|(x86_regs[op->reg]<<5)|(src));
        return;
    }
    instruction |= 1 << 24; // STUR -> STR
    if (op->type == (MEM|REG)) {
        cache_emit(instruction | (x86_regs[op->reg]<<5) | src);
    } else {
        if (!fast) emit_address(compiler, SC1R, op, buf);
        cache_emit(instruction | (SC1R<<5) | src);
    }
}
void emit_math(compiler_t* compiler, X86_64* buf, uint32_t opcode, int unsave) {
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
        emit_load(compiler, SC2R, &buf->src, buf, 0, 0);
        src = SC2R;
    }
    if (buf->dst.type&MEM) {
        emit_load(compiler, SC2R, &buf->dst, buf, 0, 0);
        dst = SC2R;
    } else dst = x86_regs[buf->dst.reg];
    
    uint32_t sf = (buf->size == 64) * ASF;
    if (buf->size < 32) logger_err("TODO: 8, 16 bit instructions");
    if (unsave) {
        cache_emit(sf | opcode | XZR | (dst<<5) | (src<<16));
    } else {
        cache_emit(sf | opcode | dst | (dst<<5) | (src<<16));
        if (buf->dst.type&MEM) emit_store(compiler, dst, &buf->dst, buf, 1);
    }
}
void emit_neon(compiler_t* compiler, X86_64* buf, uint32_t opcode, int small) {
    uint8_t r0 = buf->dst.reg;
    uint8_t r1 = buf->src.reg;
    uint32_t msf = (buf->prefix == REPN) * MSF;
    if (buf->dst.type & MEM) {
        emit_address(compiler, SC1R, &buf->dst, buf);
        cache_emit(msf|LDR_N | (SC1R<<5) | 16);
        r0 = 16;
    } else if (buf->src.type & MEM) {
        emit_address(compiler, SC1R, &buf->src,  buf);
        cache_emit(msf|LDR_N | (SC1R<<5) | 16);
        r1 = 16;
    }
    if (small)
        cache_emit(opcode|(r0)|(r1<<5));
    else cache_emit(opcode|(r0)|(r0<<5)|(r1<<16));
    if (buf->dst.type & MEM) cache_emit(msf|STR_N | (SC1R<<5) | 16);
}
void emit_branch(compiler_t* compiler, X86_64* buf, uint32_t opcode) {
    if (buf->dst.type == IMM) {
        if (buf->type == CALL) {
            if (prev(compiler)->type != CALL) cache_emit(ASF|ADD_I | 31 | (TSP<<5));
            cache_emit(BRK | (cache_set_patch(compiler->guest + buf->dst.imm)<<5));
            if (next(compiler)->type != CALL) cache_emit(ASF|ADD_I | TSP | (31<<5));
        } else emit_patch(compiler, buf);
    } else {
        uint8_t dst;
        if (buf->dst.type&MEM) {
            emit_load(compiler, SC1R, &buf->dst, buf, 0, 0);
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
            emit_load(compiler, dst, &buf->src, buf, 0, sx);
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
        emit_store(compiler, src, &buf->dst, buf, 0);
    }
}
void emit_push(compiler_t* compiler, X86_64* buf) {
    uint8_t dst;
    if (buf->dst.type == IMM) {
        emit_imm(buf->dst.imm, SC3R);
        dst = SC3R;
    } else if (buf->dst.type&MEM) {
        emit_load(compiler, SC1R, &buf->dst, buf, 0, 0);
        dst = SC1R;
    } else {
        if (buf->dst.reg == RBP) {
            if (next(compiler)->type == MOV)
                skip(compiler);
            return;
        } if (buf->dst.reg == RSP) {
            cache_emit(ASF|ADD_I | SC1R | (31<<5));
            dst = SC1R;
        } else dst = x86_regs[buf->dst.reg];
    }
    cache_emit(MSF|STR_PRE | ((-8&0x1FF)<<12) | dst | (TSP<<5));
}
void emit_pop(compiler_t* compiler, X86_64* buf) {
    (void)compiler; // for future use
    if (buf->dst.type&MEM) {
        logger_err("pop []; not supported");
    } else {
        if (buf->dst.reg == RBP) return;
        uint8_t dst = x86_regs[buf->dst.reg];
        cache_emit(MSF|LDR_POST | (8<<12) | dst | (TSP<<5));
    }
}
void emit_patch(compiler_t* compiler, X86_64* buf) {
    size_t patch_p = dynarray_push((void**)&compiler->patches);
    patch_t* patch = compiler->patches + patch_p;
    patch->host = cache_get_host();
    patch->guest = compiler->guest + buf->dst.imm;
    cache_emit(buf->type);
}
void emit_entry(struct compiler_t* compiler) {
    if (compiler->flags&NEED_STACK) {
        if (compiler->flags&NEED_ENTRY) {
            cache_emit(ASF|STP_PRE | ((-16&0x3FF)<<12) | 29 | (31<<5) | (30<<10));
        }
        if (compiler->flags&NEED_LEAVE) {
            cache_emit(ASF|ADD_I | 29 | (31<<5));
        }
        cache_emit(ASF|ADD_I | TSP | (31<<5));
    }
}
void emit_ret(struct compiler_t* compiler) {
    if (compiler->flags&NEED_STACK) {
        if (compiler->flags&NEED_LEAVE) {
            cache_emit(ASF|ADD_I | 31 | (29<<5));
        } else cache_emit(ASF|ADD_I | 31 | (TSP<<5));
        cache_emit(ASF|LDP_POST | (16<<12) | 29 | (31<<5) | (30<<10));
    }
    cache_emit(RET_R | (30 << 5));
}
void emit_jump(uint32_t* dst, uint32_t* target) {
    int64_t delta = target - dst;
    uint32_t type = *dst;
    logger_log("patch %s", instr_types[type]);
    switch (type) {
    case JE:
        *dst = BEQ | ((delta & 0x7FFFF) << 5);
        break;
    case JNE:
        *dst = BNE | ((delta & 0x7FFFF) << 5);
        break;
    case JAE:
        *dst = BCS | ((delta & 0x7FFFF) << 5);
        break;
    case JBE:
        *dst = BLS | ((delta & 0x7FFFF) << 5);
        break;
    case JGE:
        *dst = BGE | ((delta & 0x7FFFF) << 5);
        break;
    case JL:
        *dst = BLT | ((delta & 0x7FFFF) << 5);
        break;
    case JG:
        *dst = BGT | ((delta & 0x7FFFF) << 5);
        break;
    case JLE:
        *dst = BLE | ((delta & 0x7FFFF) << 5);
        break;
    case JA:
        *dst = BHI | ((delta & 0x7FFFF) << 5);
        break;
    case JS:
        *dst = BMI | ((delta & 0x7FFFF) << 5);
        break;
    case JMP:
        *dst = B | (delta & 0x3FFFFFF);
        break;
    default:
        logger_err("Unknown jump type %i", type);
    }
}
void emit_call(uint32_t* dst, uint32_t* target) {
    int32_t offset = target - dst;
    *dst = BL | (offset & 0x3FFFFFF);
    __builtin___clear_cache(dst, dst+1);
}
uint32_t math_group[] = {
    ADDS_R, ORR_R, ADCS_R, SBCS_R,
    ANDS_R, SUBS_R, EOR_R, SUBS_R,
};
uint32_t neon_group[] = {
    EOR_N,
    CMP_N, NSF|CMP_N,
    ADD_N, NSF|ADD_N,
    MUL_N, NSF|MUL_N,
    SUB_N, NSF|SUB_N,
    DIV_N, NSF|DIV_N,
    FCVTU_N, FCVT_N
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
        case ADD ... CMP: {
            uint32_t opcode = math_group[buf->type - ADD];
            emit_math(compiler, buf, opcode, buf->type == CMP);
        } break;
        case PXOR ... CVTSD2SS: {
            uint32_t opcode = neon_group[buf->type - PXOR];
            emit_neon(compiler, buf, opcode, buf->type >= CVTSS2SD);
        } break;
        case SETB ... SETGE: {
            uint32_t opcode = cset_group[buf->type - SETB];
            cache_emit(opcode | x86_regs[buf->dst.reg]);
        } break;
        case CMOVB ... CMOVGE: {
            uint32_t opcode = csel_group[buf->type - CMOVB];
            uint8_t dst = x86_regs[buf->dst.reg];
            cache_emit(opcode | dst | (x86_regs[buf->src.reg] << 5) | (dst << 16));
        } break;
        case CVTSS2SI: 
        case CVTSD2SI: {
            uint32_t prefix = NSF * (buf->prefix == REPN);
            cache_emit(prefix | FCVTZS | (x86_regs[buf->dst.reg]) | (buf->src.reg << 5));
        } break;
        case CVTSI2SS: 
        case CVTSI2SD: {
            uint32_t prefix = (ASF|NSF) * (buf->prefix == REPN);
            if (buf->src.type&MEM) {
                emit_load(compiler, SC1R, &buf->src, buf, 0, 0);
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
        case MOVSS: 
        case MOVSD:{
            uint32_t sf = (buf->prefix == REPN) * MSF;
            if (buf->dst.type & MEM) {
                emit_address(compiler, SC1R, &buf->dst, buf);
                cache_emit(sf|STR_N | (SC1R<<5) | buf->src.reg);
            }else {
                emit_address(compiler, SC1R, &buf->src, buf);
                cache_emit(sf|LDR_N | (SC1R<<5) | buf->dst.reg);
            }
        } break;
        case MOVAPD: {
            uint8_t src = buf->src.reg;
            cache_emit(MOV_N | (buf->dst.reg) | (src << 5) | (src << 16));
        } break;
        case IDIV: {
            uint8_t dst = x86_regs[buf->dst.reg];
            uint8_t src = x86_regs[buf->src.reg];
            cache_emit(ADD_I | SC1R | (dst<<5));
            cache_emit(0x9ac00000 | (src<<16) | (SC1R<<5) | (dst)); // sdiv
            cache_emit(0x9b000000 | (src<<16) | (SC1R<<10) | (dst << 5) | 2); // msub
        } break;
        case IMUL: {
            uint8_t src = SC2R;
            uint8_t dst = x86_regs[buf->dst.reg];
            if (buf->src.type&MEM) {
                emit_load(compiler, SC2R, &buf->src, buf, 0, 0);
            } else src = x86_regs[buf->src.reg];
            if (buf->dst.type == REG) {
                cache_emit(SMUL_R | (src<<16) | (dst<<5) | (dst));
            } else {
                emit_imm(buf->dst.imm, SC1R);
                cache_emit(SMUL_R | (SC1R<<16) | (src<<5) | dst);
            }
        } break;
        case TEST: emit_math(compiler, buf, ANDS_R, 1); break;
        case ROR: emit_math(compiler, buf, ROR_R, 0); break;
        case SHL:
        case SAL: emit_math(compiler, buf, LSL_R, 0); break;
        case SHR: emit_math(compiler, buf, LSR_R, 0); break;
        case SAR: emit_math(compiler, buf, ASR_R, 0); break;
        case EBR: case NOP: case LEAVE: case HLT: break;
        case JMP: emit_branch(compiler, buf, BR); break;
        case CALL: emit_branch(compiler, buf, BLR); break;
        case RET: emit_ret(compiler); break;
        case JO ... JG: emit_patch(compiler, buf); break;
        case CLTQ: case CLTD:
            cache_emit(SXTW_R | (x86_regs[RAX] << 5) | x86_regs[RAX]);
            if (buf->type == CLTD) cache_emit(0x937ffc00 | (x86_regs[RAX] << 5) | x86_regs[RDX]); // asr x2, x8, #63
            break;
        default:
            logger_err("Unknown encode: %i", buf->type);
            exit(0);
    }
}
