#pragma once

#include <elf.h>

class Elf {
    Elf64_Ehdr head;
    Elf64_Phdr* pheads = nullptr;
    Elf64_Sym* symtab;
    char* strtab;

    Elf64_Addr base;
    Elf64_Addr init;
    Elf64_Addr* init_array;
    int init_arraysz;

    void mmap_base();
    void reloc_relr(Elf64_Relr* relr, int relrsz);
    void reloc_rela(Elf64_Rela* rela, int relasz);
    // void reloc_rel(Elf64_Rel* rel, int relsz);
public:
    ~Elf();
    
    bool open(const char* filename);
    void read_dynamic();
    void start_init();
    void* entry();
};