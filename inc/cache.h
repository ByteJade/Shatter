#ifndef CACHE_H
#define CACHE_H

#include <stdint.h>

typedef struct  {
    uint8_t* guest;
    uint32_t* host;
} code_t;

void cache_init(void);
void cache_fini(void);

void* cache_mmap_guest(uint64_t size);
void cache_start_block(uint8_t* guest);
void cache_end_block();
void cache_emit(uint32_t data);
int cache_set_patch(uint8_t* guest);
uint8_t* cache_get_patch(int id);

void cache_print();
uint32_t* cache_get_host();
uint32_t* cache_search(uint8_t* guest);
void cache_clear(void *address, uint64_t len);

#endif