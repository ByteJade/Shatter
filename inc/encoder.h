#ifndef ENCODER_H
#define ENCODER_H

#include "decoder.h"

extern const uint8_t x86_regs[];

#define UNK 0xFFFFFFFF

#define GET_FS 0xD53BD040
#define ADRP 0x90000000
#define UBFM 0xD3400000

#define ASF 0x80000000

#define MOVN_I 0x12800000
#define MOVZ_I 0x52800000
#define MOVK_I 0x72800000

#define ADD_R 0x0B000000
#define ADDS_R 0x2B000000
#define SUB_R 0x4B000000
#define SUBS_R 0x6B000000

#define ADD_I 0x11000000
#define ADDS_I 0x31000000
#define SUB_I 0x51000000
#define SUBS_I 0x71000000

#define AND_R 0x0A000000
#define ADC_R 0x1A000000
#define ORR_R 0x2A000000
#define ADCS_R 0x3A000000
#define EOR_R 0x4A000000
#define SBC_R 0x5A000000
#define ANDS_R 0x6A000000
#define SBCS_R 0x7A000000
#define SMUL_R 0x9b207c00

#define LSL_R 0x1AC02000
#define LSR_R 0x1AC02400
#define ASR_R 0x1AC02800
#define ROR_R 0x1AC02C00

#define SXTW_R 0x93407C00

#define MSF 0x40000000

#define STR 0xB9000000
#define STRH 0x79000000
#define STRB 0x39000000
#define STUR 0xB8000000
#define STURH 0x78000000
#define STURB 0x38000000
#define STR_PRE 0xB8000C00
#define STR_POST 0xB8000400
#define STP_PRE 0x29800000
#define STP_POST 0x28800000

#define LDR 0xB9400000
#define LDRH 0x79400000
#define LDRB 0x39400000
#define LDUR 0xB8400000
#define LDURH 0x78400000
#define LDURB 0x38400000
#define LDR_PRE 0xB8400C00
#define LDR_POST 0xB8400400
#define LDP_PRE 0x29C00000
#define LDP_POST 0x28C00000
#define LDRSH 0x79C00000
#define LDRSB 0x39C00000

#define ST_GRP 0x08000000
#define LD_GRP 0x08400000
#define STN_GRP 0x0C000000
#define LDN_GRP 0x0C400000
#define D_GRP 0xF0000000
#define W_GRP 0xB0000000
#define SX_GRP 0x00800000
#define H_GRP 0x70000000
#define B_GRP 0x30000000
#define U_GRP 0x01000000

#define BR 0xD61F0000
#define BLR 0xD63F0000
#define BRK 0xD4200000
#define RET_R 0xD65F0000

#define BEQ 0x54000000
#define BNE 0x54000001
#define BCS 0x54000002
#define BLO 0x54000003
#define BMI 0x54000004
#define BHI 0x54000008
#define BLS 0x54000009
#define BGE 0x5400000A
#define BLT 0x5400000B
#define BGT 0x5400000C
#define BLE 0x5400000D
#define B 0x14000000
#define BL 0x94000000

#define CSETB 0x1a9f27e0
#define CSETAE 0x1a9f37e0
#define CSETE 0x1a9f17e0
#define CSETNE 0x1a9f07e0 
#define CSETBE 0x1a9f97e0
#define CSETA 0x1a9f87e0
#define CSETS 0x1a9f47e0
#define CSETNS 0x1a9f57e0
#define CSETL 0xa9fa7e0
#define CSETGE 0x1a9fb7e0

#define CSELB 0x1A803000
#define CSELAE 0x1A802000
#define CSELE 0x1A800000
#define CSELNE 0x1A801000
#define CSELBE 0x1A809000
#define CSELA 0x1A808000
#define CSELS 0x1A804000
#define CSELNS 0x1A805000
#define CSELL 0x1A80B000
#define CSELGE 0x1A80A000

#define NSF 0x00400000

#define EOR_N 0x6E201C00
#define ADD_N 0x1E202800
#define MUL_N 0x1E200800
#define SUB_N 0x1E203800
#define DIV_N 0x1E201800
#define CMPE_N 0x1E202010
#define CMP_N 0x1E202000
#define FCVTU_N 0x1E22C000
#define FCVT_N 0x1e624000
#define LDR_N 0xBD400000
#define STR_N 0xBD000000

#define MOV_N 0x4EA01C00
#define FMOV_N 0x9E660000
#define FMOVR_N 0x9E670000
#define FCVTNS 0x1E200000
#define FCVTZS 0x1E780000
#define SCVTF_N 0x1E220000

struct compiler_t;

void emit_entry(struct compiler_t* compiler);
void emit_jump(uint32_t* dst, uint32_t* target);
void emit_call(uint32_t* dst, uint32_t* target);
void encode(struct compiler_t* compiler, X86_64* buf);

#endif
