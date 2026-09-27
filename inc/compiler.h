#ifndef COMPILER_H
#define COMPILER_H

#include "decoder.h"

typedef struct {
    uint8_t* start;
    uint8_t* end;
    uint32_t buffer;
    uint32_t size;
} block_t;
typedef struct {
    uint8_t* point;
    uint32_t* host;
} point_t;
typedef struct {
    uint32_t* host;
    uint8_t* guest;
} patch_t;

enum CompileFlags {
    NEED_STACK = 1,
    NEED_ENTRY = 2,
};

typedef struct compiler_t {
    uint8_t* guest;

    uint8_t* sizes;
    X86_64* buffer;
    block_t* blocks;
    point_t* points;
    patch_t* patches;

    uint32_t reader;
    int flags;
} compiler_t;

compiler_t* compiler_init(void);
void compiler_fini(compiler_t* compiler);
uint32_t* compiler_step(compiler_t* compiler, uint8_t* guest);

X86_64* next(compiler_t* compiler);
void skip(compiler_t* compiler);

#endif