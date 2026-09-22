#include "../include/Compiler.hpp"
#include "../include/Cache.hpp"
#include "../include/Logger.hpp"
#include "../include/Encoder.hpp"
#include "../include/Printer_X86_64.hpp"
#include <cstdint>
#include <stdint.h>

#define SC1R 9
#define SC2R 10
#define SC3R 11
#define XZR 31

const uint8_t x86_regs[] = {
    8, 3, 2, 19,
    28, 29, 1, 0,
    4, 5, 6, 7,
    20, 21, 22, 23,
};
void emit_add_signed(uint8_t dst, uint8_t src, int64_t imm) {
    if (imm > 0)
        cache.emit(ASF|ADD_I | (dst) | (src<<5) | (imm<<10));
    else cache.emit(ASF|SUB_I | (dst) | (src<<5) | (-imm<<10));
}
void Compiler::emit_address(uint8_t dst, Operand& op, X86_64& buf) {
    uint8_t t = op.type;
    if (buf.prefix == FS) {
        cache.emit(GET_FS | dst);
        emit_add_signed(dst, dst, op.imm);
    } else if (op.type == (MEM|IMM)) {
        uint64_t full = (uint64_t)(guest + op.imm);
        int64_t target = full & ~0xFFF;
        int64_t current = (uint64_t)(cache.get_host()) & ~0xFFF;
        int64_t delta = (target - current) >> 12;
        if (delta < -4294967296LL || delta > 4294967296LL) {
            logger.err() << "too large rip distance" << std::endl;
        }
        cache.emit(ADRP | ((delta & 0x3) << 29) | (((delta >> 2) & 0x7FFFF) << 5) | dst);
        cache.emit((ASF|ADD_I | ((full & 0xFFF) << 10) | (dst << 5) | dst));
    } else if (op.type&IDX) {
        if (op.scale != 0) {
            cache.emit(UBFM | ((-(op.scale) & 0x3F) << 16) |
            (((63 - op.scale) & 0x3F) << 10) | (x86_regs[op.idx] << 5) | dst);
        } else cache.emit(ASF|ADD_I | (dst) | (x86_regs[op.idx]<<5));
        if (t&REG) {
            cache.emit(ASF|ADD_R | (dst) | (dst<<5) | (x86_regs[op.reg]<<16));
        }
        if ((t&IMM) && op.imm != 0) {
            emit_add_signed(dst, dst, op.imm);
        }
    }else {
        if (t&IMM) {
            emit_add_signed(dst, x86_regs[op.reg], op.imm);
        } else {
            cache.emit(ASF|ADD_I | (dst) | (x86_regs[op.reg]<<5));
        }
    }
}
void Compiler::emit_imm(int64_t imm, uint8_t dst) {
    if (imm >= 0 && imm <= INT16_MAX) {
        cache.emit(ASF|MOVZ_I | (imm << 5) | dst);
    } else if (imm < 0 && ~imm <= INT16_MAX) {
        cache.emit(ASF|MOVN_I | (~imm << 5) | dst);
    } else {
        uint16_t a = imm & 0xFFFF;
        uint16_t b = (imm>>16) & 0xFFFF;
        if (a) cache.emit(ASF|MOVZ_I | (a << 5) | dst);
        if (b) cache.emit(ASF|MOVK_I | (1 << 21) | (b << 5) | dst);
        if (imm >= INT32_MIN && imm <= INT32_MAX) {
            //if (imm < 0) cache.emit(SXTW_REG | (dst << 5) | dst);
            return;
        }
        uint16_t c = (imm>>32) & 0xFFFF;
        uint16_t d = imm>>48;
        if (c) cache.emit(ASF|MOVK_I | (2 << 21) | (c << 5) | dst);
        if (d) cache.emit(ASF|MOVK_I | (3 << 21) | (d << 5) | dst);
    }
}
void Compiler::emit_load(uint8_t dst, Operand& op, X86_64& buf, bool fast) {
    uint32_t sf = MSF*(buf.size==64);
    if (op.type == (MEM|REG|IMM) &&
        op.imm > -256 &&
        op.imm < 255) {
        cache.emit(sf|LDUR|((op.imm&0x1FF)<<12)|(x86_regs[op.reg]<<5)|dst);
    } else if (op.type == (MEM|REG)) {
        cache.emit(sf|LDR | (x86_regs[op.reg]<<5) | dst);
    } else {
        if (!fast) emit_address(SC1R, op, buf);
        cache.emit(sf|LDR | (SC1R<<5) | dst);
    }
}
void Compiler::emit_store(uint8_t src, Operand& op, X86_64& buf, bool fast) {
    uint32_t sf = MSF*(buf.size==64);
    if (op.type == (MEM|REG|IMM) &&
        op.imm > -256 &&
        op.imm < 255) {
        cache.emit(sf|STUR|((op.imm&0x1FF)<<12)|(x86_regs[op.reg]<<5)|(src));
    } else if (op.type == (MEM|REG)) {
        cache.emit(sf|STR | (x86_regs[op.reg]<<5) | src);
    } else {
        if (!fast) emit_address(SC1R, op, buf);
        cache.emit(sf|STR | (SC1R<<5) | src);
    }
}
void Compiler::emit_math(X86_64& buf, uint32_t opcode, bool unsafe) {
    // ADD, SUB, OR, XOR, AND
    uint8_t src;
    uint8_t dst;
    if (buf.src.type == IMM) {
        if (buf.src.imm) {
            emit_imm(buf.src.imm, SC3R);
            src = SC3R;
        } else src = XZR;
    } else if (buf.src.type == REG) {
        src = x86_regs[buf.src.reg];
    } else {
        emit_load(SC2R, buf.src, buf, false);
        src = SC2R;
    }
    if (buf.dst.type&MEM) {
        emit_load(SC2R, buf.dst, buf, false);
        dst = SC2R;
    } else dst = x86_regs[buf.dst.reg];
    
    uint32_t sf = (buf.size == 64) * ASF;
    if (unsafe) {
        cache.emit(sf | opcode | XZR | (dst<<5) | (src<<16));
    } else {
        cache.emit(sf | opcode | dst | (dst<<5) | (src<<16));
        if (buf.dst.type&MEM) emit_store(dst, buf.dst, buf, true);
    }
}
void Compiler::emit_branch(X86_64& buf, uint32_t opcode) {
    if (buf.dst.type == IMM) {
        if (buf.type == CALL) {
            cache.emit(ASF|ADD_I | 31 | (28<<5));
            cache.emit(BRK | (cache.set_patch(guest+buf.dst.imm)<<5));
        } else emit_patch(buf);
    } else {
        cache.emit(ASF|ADD_I | 31 | (28<<5));
        uint8_t dst;
        if (buf.dst.type&MEM) {
            emit_load(SC1R, buf.dst, buf, 0);
            dst = SC1R;
        } else dst = x86_regs[buf.dst.reg];
        cache.emit(opcode | (dst << 5));
    }
}
void Compiler::emit_mov(X86_64& buf) {
    if (buf.dst.type == REG) {
        uint8_t dst = x86_regs[buf.dst.reg];
        if (buf.src.type == IMM) {
            emit_imm(buf.src.imm, dst);
        } else if (buf.src.type&MEM) {
            emit_load(dst, buf.src, buf, false);
        } else {
            cache.emit(ASF|ADD_I | dst | (x86_regs[buf.src.reg]<<5));
        }
    } else {
        uint8_t src;
        if (buf.src.type == IMM) {
            if (buf.src.imm) {
                emit_imm(buf.src.imm, SC3R);
                src = SC3R;
            } else src = XZR;
        } else src = x86_regs[buf.src.reg];
        emit_store(src, buf.dst, buf, false);
    }
}
void Compiler::emit_push(X86_64& buf) {
    uint8_t dst;
    if (buf.dst.type == IMM) {
        emit_imm(buf.dst.imm, SC3R);
        dst = SC3R;
    } else if (buf.dst.type&MEM) {
        emit_load(SC1R, buf.dst, buf, false);
        dst = SC1R;
    } else {
        if (buf.dst.reg == RBP) return;
        if (buf.dst.reg == RSP) {
            cache.emit(ASF|ADD_I | SC1R | (31<<5));
            dst = SC1R;
        } else dst = x86_regs[buf.dst.reg];
    }
    // STR Xt, [SP, #-16]!
    cache.emit(MSF|STR_PRE | ((-8&0x1FF)<<12) | dst | (28<<5));
}
void Compiler::emit_pop(X86_64& buf) {
    uint8_t dst;
    if (buf.dst.type&MEM) {
        logger.err() << "pop []; not supported" << std::endl;
    } else {
        if (buf.dst.reg == RBP) return;
        dst = x86_regs[buf.dst.reg];
        cache.emit(MSF|LDR_POST | (8<<12) | dst | (28<<5));
    }
    // LDR Xt, [SP], #16
}
void Compiler::emit_patch(X86_64& buf) {
    patches.push_back({cache.get_host(), guest + buf.dst.imm});
    cache.emit(buf.type);
}
void Compiler::emit_entry() {
    cache.emit(ASF|ADD_I | 28 | (31<<5));
    if (need_entry) cache.emit(ASF|STP_PRE | ((-16&0x3FF)<<12) | 30 | (28<<5) | (29<<10));
}
void Compiler::emit_ret() {
    if (need_entry) cache.emit(ASF|LDP_POST | (16<<12) | 30 | (28<<5) | (29<<10));
    cache.emit(ASF|ADD_I | 31 | (29<<5));
    cache.emit(0xD65F03C0);
}

void Compiler::encode(X86_64& buf) {
    switch (buf.type) {
        case MOV: emit_mov(buf); break;
        case PUSH: emit_push(buf); break;
        case POP: emit_pop(buf); break;
        case LEA: emit_address(x86_regs[buf.dst.reg], buf.src, buf); break;
        case ADD: emit_math(buf, ADDS_R, false); break;
        case SUB: emit_math(buf, SUBS_R, false); break;
        case CMP: emit_math(buf, SUBS_R, true); break;
        case TEST: emit_math(buf, ANDS_R, true); break;
        case OR:  emit_math(buf, ORR_R, false); break;
        case XOR: emit_math(buf, EOR_R, false); break;
        case AND: emit_math(buf, ANDS_R, false); break;
        case ROR: emit_math(buf, ROR_R, false); break;
        case SHL:
        case SAL: emit_math(buf, ORR_R, false); break;
        case SHR: emit_math(buf, LSL_R, false); break;
        case SAR: emit_math(buf, ASR_R, false); break;
        case EBR: case NOP: case LEAVE: case HLT: break;
        case JMP: emit_branch(buf, BR); break;
        case CALL: emit_branch(buf, BLR); break;
        case RET: emit_ret(); break;
        case JO ... JG: emit_patch(buf); break;
        case CLTQ: cache.emit(SXTW_R | (x86_regs[RAX] << 5) | x86_regs[RAX]); break;
        case CLTD:
            cache.emit(SXTW_R | (x86_regs[RAX] << 5) | x86_regs[RAX]);
            cache.emit(0x937ffc00 | (x86_regs[RAX] << 5) | x86_regs[RDX]); // asr x2, x8, #63
            break;
        default:
            logger.err() << "Unknown encode: " << (int)buf.type << std::endl;
            exit(0);
    }
}
void Compiler::emit_jump(uint8_t type, uint32_t* dst, uint32_t* target) {
    int64_t delta = target - dst;
    logger.log() << "patch " << instr_types[type] << std::endl;
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
    case JMP:
        *dst = B | (delta & 0x3FFFFFF);
        break;
    default:
        logger.err() << "Unknown jump type " << type << std::endl;
    }
}