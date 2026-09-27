#ifndef ELF_LOADER_H
#define ELF_LOADER_H

#include <elf.h>

typedef struct {
    Elf64_Ehdr head;
    Elf64_Phdr* pheads;
    Elf64_Sym* symtab;
    char* strtab;

    Elf64_Addr base;
    Elf64_Addr init;
    Elf64_Addr* init_array;
    int init_arraysz;
} elf_t;

void mmap_base(elf_t* elf);
void reloc_relr(elf_t* elf, Elf64_Relr* relr, int relrsz);
void reloc_rela(elf_t* elf, Elf64_Rela* rela, int relasz);

elf_t* elf_init(const char* filename);
void elf_read_dynamic(elf_t* elf);
void elf_start(elf_t* elf);
void elf_fini(elf_t* elf);

#endif