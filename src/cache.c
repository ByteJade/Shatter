#include "../inc/cache.h"
#include "../inc/logger.h"
#include "../inc/dynarray.h"
#include "../inc/printer_Aarch64.h"
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>

code_t* blocks;
uint8_t** patches;
int* reuse;

pthread_mutex_t mtx;
uint32_t* host;
size_t prev_host_p;
size_t host_p;

void cache_init(void) {
    blocks = dynarray_init(sizeof(code_t));
    patches = dynarray_init(sizeof(uint8_t*));
    reuse = dynarray_init(sizeof(int));
}
void cache_fini(void) {
    dynarray_fini(blocks);
    dynarray_fini(patches);
    dynarray_fini(reuse);
}

void* cache_mmap_guest(uint64_t size) {
    void* map = mmap(
        NULL, size+size,
        PROT_READ | PROT_WRITE | PROT_EXEC,
        MAP_ANON | MAP_PRIVATE,
        -1, 0
    );
    if (map == MAP_FAILED) {
        logger_err("Cannot map guest & host (Out of memory?)");
        exit(EXIT_FAILURE);
    }
    #ifdef __aarch64__
    mprotect(map, size, PROT_READ | PROT_WRITE);
    #endif
    host = (uint32_t*)((uint8_t*)map + size);
    return map;
}
void cache_start_block(uint8_t* guest) {
    pthread_mutex_lock(&mtx);
    prev_host_p = host_p;
    size_t block_p = dynarray_push((void**)&blocks);
    code_t* block = blocks + block_p;
    block->guest = guest;
    block->host = host + host_p;
}
void cache_end_block() {
    __builtin___clear_cache(host+prev_host_p, host+host_p);
    pthread_mutex_unlock(&mtx);
}
void cache_emit(uint32_t data) {
    host[host_p++] = data;
}
int cache_set_patch(uint8_t* guest) {
    int id;
    if (dynarray_size(reuse)) {
        id = dynarray_pop((void*)reuse);
    } else {
        id = dynarray_push((void**)&patches);
    }
    patches[id] = guest;
    return ++id;
}
uint8_t* cache_get_patch(int id) {
    id--;
    size_t reuse_p = dynarray_push((void**)&reuse);
    reuse[reuse_p] = id;
    return patches[id];
}

void cache_print() {
    for (uint32_t i = prev_host_p; i < host_p; i++) {
        uint32_t buf = host[i];
        printf("%X: ", buf);
        print_aarch64(buf);
    }
}
void cache_usage() {
    printf("program: %li bytes\n", host_p*4);
    printf("cache: %li bytes\n",
        dynarray_size(blocks) * sizeof(code_t) +
        dynarray_size(patches) * sizeof(uint8_t*) +
        dynarray_size(reuse) * sizeof(int)
    );
}
uint32_t* cache_get_host() {
    return host + host_p;
}
uint32_t* cache_search(uint8_t* guest) {
    for (size_t i = 0; i < dynarray_size(blocks); i++) {
        code_t* block = blocks + i;
        if (block->guest == guest) {
            return block->host;
        }
    }
    return NULL;
}