#ifndef CACHE_H
#define CACHE_H

#include <stdint.h>

typedef struct  {
    uint8_t* guest;
    uint32_t* host;
} code_t;

void cache_init(void);
void cache_fini(void);

void* cache_mmap_guest(uint8_t* base, uint64_t size);
void cache_start_block(uint8_t* guest);
void cache_end_block();
void cache_emit(uint32_t data);
uint32_t cache_set_patch(uint8_t* guest);
uint8_t* cache_get_patch(uint32_t id);

void cache_print();
void cache_usage();
uint32_t* cache_get_host();
uint32_t* cache_search(uint8_t* guest);

void cache_lock();
void cache_unlock();

#endif