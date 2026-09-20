#include "../include/Cache.hpp"
#include "../include/Logger.hpp"
#include <bits/floatn.h>
#include <sys/mman.h>
#include <cstdint>
#include <cstdlib>

Cache cache;

void* Cache::mmap_guest(size_t size) {
    void* map = mmap(
        nullptr, size+size,
        PROT_READ | PROT_WRITE | PROT_EXEC,
        MAP_ANON | MAP_PRIVATE,
        -1, 0
    );
    #ifdef __aarch64__
    mprotect(map, size, PROT_READ | PROT_WRITE);
    #endif
    host = (uint32_t*)((uint8_t*)map + size);
    return map;
}

void Cache::start_block(uint8_t* guest) {
    mtx.lock();
    prev_host_p = host_p;
    blocks.push_back({guest, host + host_p});
}
void Cache::emit(uint32_t data) {
    host[host_p++] = data;
}
void Cache::end_block() {
    __builtin___clear_cache(host+prev_host_p, host+host_p);
    mtx.unlock();
}
int Cache::set_patch(uint8_t* guest) {
    int id;
    if (reuse.empty()) {
        id = patches.size();
        patches.push_back(guest);
    } else {
        id = reuse.back();
        patches[reuse.back()] = guest;
        reuse.pop_back();
    }
    return id;
}
uint8_t* Cache::get_patch(int id) {
    reuse.push_back(id);
    return patches[id];
}
void Cache::print() {
    logger.force() << std::hex;
    for (uint32_t i = prev_host_p; i < host_p; i++) {
        logger.force() << host[i] << std::endl;
    }
    logger.force() << std::dec;
}
uint32_t* Cache::get_host() {
    return host + host_p;
}
uint32_t* Cache::search(uint8_t* guest) {
    for (CodeBlock& block : blocks) {
        if (block.guest == guest) {
            return block.host;
        }
    }
    return nullptr;
}