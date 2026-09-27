#include "../inc/compiler.h"
#include "../inc/dynarray.h"
#include "../inc/logger.h"
#include "../inc/cache.h"
#include "../inc/debugger.h"
#include "../inc/encoder.h"
#include "../inc/printer_X86_64.h"
#include <stdint.h>

compiler_t* compiler_init(void) {
    compiler_t* compiler = (compiler_t*)malloc(sizeof(compiler_t));

    compiler->sizes = dynarray_init(sizeof(uint8_t));
    compiler->buffer = dynarray_init(sizeof(X86_64));
    compiler->blocks = dynarray_init(sizeof(block_t));
    compiler->points = dynarray_init(sizeof(point_t));
    compiler->patches = dynarray_init(sizeof(patch_t));
    compiler->reader = 0;
    compiler->need_entry = 0;

    return compiler;
}
void compiler_fini(compiler_t* compiler) {
    if (compiler) {
        dynarray_fini(compiler->sizes);
        dynarray_fini(compiler->buffer);
        dynarray_fini(compiler->blocks);
        dynarray_fini(compiler->points);
        dynarray_fini(compiler->patches);
        free(compiler);
    }
}
void push_jump(compiler_t* compiler, int delta) {
    size_t i = 0;
    uint8_t* dst = compiler->guest + delta;
    point_t* points = compiler->points;
    size_t end = dynarray_size(points);
    for (; i < end; i++) {
        uint8_t* p = points[i].point;
        if (p == dst) return;
        if (p > dst) break;
    }
    end = dynarray_push((void**)&compiler->points);
    points = compiler->points;
    for (size_t y = end; y > i; y--) {
        points[y].point = points[y-1].point;
    }
    points[i].point = dst;
}
point_t* search_point(compiler_t* compiler, uint8_t* guest) {
    point_t* points = compiler->points;
    size_t size = dynarray_size(points);
    if (!size) return NULL;
    if (guest > points[size-1].point) return NULL;
    size_t left = 0;
    size_t right = size;
    while (left < right) {
        size_t mid = left + (right - left) / 2;
        point_t* p = points + mid;
        if (p->point == guest) return p;
        if (p->point < guest) left = mid + 1;
        else right = mid;
    }
    return NULL;
}
int has_block(compiler_t* compiler, uint8_t* p) {
    for (size_t i = 0; i < dynarray_size(compiler->blocks); i++) {
        block_t* block = compiler->blocks + i;
        if (p >= block->start && p < block->end) {
            return 1;
        }
    }
    return 0;
}
void block_start(compiler_t* compiler) {
    size_t block_p = dynarray_push((void**)&compiler->blocks);
    block_t* block = compiler->blocks + block_p;
    block->start = compiler->guest;
    block->end = 0;
    block->buffer = dynarray_size(compiler->buffer);
    block->size = 0;
}
int jump(compiler_t* compiler) {
    size_t prev_p = dynarray_size(compiler->blocks)-1;
    block_t* prev = compiler->blocks + prev_p;
    size_t buffer_size = dynarray_size(compiler->buffer);
    prev->size = buffer_size - prev->buffer;
    prev->end = compiler->guest;
    point_t* points = compiler->points;
    for (; compiler->reader < dynarray_size(points); compiler->reader++) {
        uint8_t* p = points[compiler->reader].point;
        if (!has_block(compiler, p)) {
            compiler->guest = p;
            block_start(compiler);
            return 1;
        }
    }
    return 0;
}
int emulate(compiler_t* compiler, X86_64* buf) {
    int do_jump = 0;
    switch (buf->type) {
        case JO ... JG:
            push_jump(compiler, buf->dst.imm);
            break;
        case JMP:
            if (buf->dst.type == IMM) {
                push_jump(compiler, buf->dst.imm);
            }
            [[fallthrough]];
        case HLT:
            do_jump = 1;
            break;
        case RET:
            compiler->need_entry = 1;
            do_jump = 1;
            break;
    }
    if (do_jump || search_point(compiler, compiler->guest)) {
        if (!jump(compiler)) return 0;
    }
    return 1;
}
void decode_step(compiler_t* compiler) {
    X86_64* buf;
    // TODO: flags
    block_start(compiler);
    do {
        size_t buf_p = dynarray_push((void**)&compiler->buffer);
        buf = compiler->buffer + buf_p;
        uint8_t* prev_guest = compiler->guest;
        decode(compiler, buf);
        uint8_t* cur_guest = compiler->guest;
        size_t size_p = dynarray_push((void**)&compiler->sizes);
        compiler->sizes[size_p] = cur_guest - prev_guest;
    } while (emulate(compiler, buf));
    logger_deb("End decode, blocks: %i", dynarray_size(compiler->blocks));
}
void encode_step(compiler_t* compiler, block_t* block) {
    compiler->guest = block->start;
    uint32_t end = block->buffer + block->size;
    for (compiler->reader = block->buffer; compiler->reader < end; compiler->reader++) {
        point_t* p = search_point(compiler, compiler->guest);
        if (p) p->host = cache_get_host();
        compiler->guest += compiler->sizes[compiler->reader];
        X86_64* buf = compiler->buffer + compiler->reader;
        if (debugger_enabled()) print_x86_64(buf);
        encode(compiler, buf);
    }
    logger_deb("End compile");
}
void patch_step(compiler_t* compiler) {
    size_t patches_size = dynarray_size(compiler->patches);
    logger_deb("Patch %i jumps", patches_size);
    for (size_t i = 0; i < patches_size; i++) {
        patch_t* patch = compiler->patches + i;
        point_t* point = search_point(compiler, patch->guest);
        if (point) {
            emit_jump(patch->host, point->host);
        } else logger_err("Compiler: Cannot patch jump");
    }
}
uint32_t* compiler_step(compiler_t* compiler, uint8_t* guest) {
    logger_deb("Start compile: %p", guest);
    compiler->guest = guest;
    decode_step(compiler);
    cache_start_block(guest);
    uint32_t* ret = cache_get_host();
    emit_entry(compiler);
    for (size_t i = 0; i < dynarray_size(compiler->blocks); i++) {
        logger_log("start block %i", i);
        block_t* block = compiler->blocks + i;
        encode_step(compiler, block);
    }
    patch_step(compiler);
    cache_end_block();
    if (debugger_enabled()) debugger_brk(ret);
    return ret;
}
