#pragma once

#include <cstdint>
#include <mutex>
#include <vector>

struct CodeBlock {
    uint8_t* guest;
    uint32_t* host;
};

class Cache {
    std::vector<CodeBlock> blocks;
    std::vector<uint8_t*> patches;
    std::vector<int> reuse;
    std::mutex mtx;
    uint32_t* host;
    size_t host_p;
public:
    void* mmap_guest(size_t size);
    void start_block(uint8_t* guest);
    void end_block();
    void emit(uint32_t data);
    int set_patch(uint8_t*);
    uint8_t* get_patch(int id);

    void print();
    uint32_t* get_host();
    uint32_t* search(uint8_t* guest);
};

extern Cache cache;