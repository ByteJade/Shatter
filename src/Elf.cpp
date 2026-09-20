#include "../include/Elf.hpp"
#include "../include/Elf_manager.hpp"
#include "../include/Logger.hpp"
#include "../include/Launcher.hpp"
#include "../include/Cache.hpp"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <elf.h>

#define PAGE_SIZE 0x1000

void Elf::mmap_base() {
    size_t max = 0;
    size_t min = SIZE_MAX;
    for (int i = 0; i < head.e_phnum; i++) {
        Elf64_Phdr* phdr = pheads + i;
        if (phdr->p_type == PT_LOAD) {
            size_t start = phdr->p_vaddr;
            size_t end = start + phdr->p_memsz;
            if (start < min) min = start;
            if (end > max) max = end;
        }
    }
    min &= ~(PAGE_SIZE - 1);
    max = (max + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);
    base = (Elf64_Addr)cache.mmap_guest(max - min);
    base -= min;
}
void Elf::reloc_relr(Elf64_Relr* relr, int relrsz) {
    size_t count = relrsz / sizeof(Elf64_Addr);
    Elf64_Addr *where = nullptr;
    for (size_t i = 0; i < count; i++) {
        Elf64_Addr entry = relr[i];
        if (!(entry & 1)) {
            where = (Elf64_Addr*)(base + entry);
            *where++ += base;
        } else {
            for (long i = 0; (entry >>= 1) != 0; i++) {
                if (entry&1) where[i] += base;
            } where += 63;
        }
    }
}
void Elf::reloc_rela(Elf64_Rela* rela, int relasz) {
    bool allright = true;
    for (size_t i = 0; i < relasz / sizeof(Elf64_Rela); i++) {
        Elf64_Rela* rel = rela + i;
        int type = ELF64_R_TYPE(rel->r_info);
        int sym_idx = ELF64_R_SYM(rel->r_info);
        Elf64_Addr* patch = (Elf64_Addr*)(base + rel->r_offset);
        Elf64_Sym* sym = symtab + sym_idx;
        const char* symname = strtab + sym->st_name;
        switch(type) {
            case R_X86_64_NONE: break;
            case R_X86_64_64:
                *patch = (Elf64_Addr)(sym->st_value + rel->r_addend);
                break;
            case R_X86_64_RELATIVE:
                *patch = (Elf64_Addr)(base + rel->r_addend);
                break;
            case R_X86_64_JUMP_SLOT:
            case R_X86_64_GLOB_DAT: {
                void *sym_addr = my_dlsym(RTLD_DEFAULT, symname);
                if (sym_addr) {
                    *patch = (Elf64_Addr)sym_addr;
                } else if (ELF64_ST_BIND(sym->st_info) == STB_WEAK) {
                    *patch = 0;
                } else {
                    logger.err() << "Undefined GLOB symbol: " << symname << std::endl;
                    allright = false;
                }
            } break;
            case R_X86_64_COPY: {
                void *sym_addr = my_dlsym(RTLD_DEFAULT, symname);
                size_t size = sym->st_size;
                if (sym_addr && size) {
                    memmove(patch, sym_addr, size);
                    override_got(symname, patch);
                } else {
                    logger.err() << "Undefined COPY symbol: " << symname << std::endl;
                    allright = false;
                }
            } break;
            default:
                logger.warn() << "Unknown RELA " << type << std::endl;
                allright = false;
        }
    }
    if (!allright) exit(EXIT_FAILURE);
}
// void Elf::reloc_rel(Elf64_Rel* rel, int relsz) {}

Elf::~Elf() {
    if (pheads) delete[] pheads;
}

bool Elf::open(const char* filename) {
    FILE* fp = fopen(filename, "rb");
    if (fp == nullptr) return false;
    fread(&head, sizeof(Elf64_Ehdr), 1, fp);
    pheads = new Elf64_Phdr[head.e_phnum];
    fseek(fp, head.e_phoff, SEEK_SET);
    fread(pheads, sizeof(Elf64_Phdr), head.e_phnum, fp);
    mmap_base();
    for (int i = 0; i < head.e_phnum; i++) {
        Elf64_Phdr* phdr = pheads + i;
        if (phdr->p_type == PT_LOAD) {
            char* dst = (char*)base + phdr->p_vaddr;
            fseek(fp, phdr->p_offset, SEEK_SET);
            fread(dst, 1, phdr->p_filesz, fp);
            if (phdr->p_filesz != phdr->p_memsz) {
                memset(dst + phdr->p_filesz, 0,
                    phdr->p_memsz - phdr->p_filesz
                );
            }
        }
    }
    fclose(fp);
    return true;
}
void Elf::read_dynamic() {
    Elf64_Phdr* dyn_phdr = nullptr;
    for (int i = 0; i < head.e_phnum; i++) {
        Elf64_Phdr* phdr = pheads + i;
        if (phdr->p_type == PT_DYNAMIC) {
            dyn_phdr = phdr;
            break;
        }
    }
    if (dyn_phdr == nullptr) {
        logger.err() << "No dynamic section" << std::endl;
        exit(EXIT_FAILURE);
    }
    Elf64_Dyn* dyn = (Elf64_Dyn*)(base + dyn_phdr->p_vaddr);
    size_t relrsz = 0;
    Elf64_Relr* relr = nullptr;
    size_t relasz = 0;
    Elf64_Rela* rela = nullptr;
    size_t jmprelsz = 0;
    Elf64_Rela* jmprel = nullptr;
    for (; dyn->d_tag != DT_NULL; dyn++) {
        switch (dyn->d_tag) {
            case DT_PLTRELSZ:
                jmprelsz = dyn->d_un.d_val;
                break;
            case DT_STRTAB:
                strtab = (char*)(base + dyn->d_un.d_ptr);
                break;
            case DT_SYMTAB:
                symtab = (Elf64_Sym*)(base + dyn->d_un.d_ptr);
                break;
            case DT_RELA:
                rela = (Elf64_Rela*)(base + dyn->d_un.d_ptr);
                break;
            case DT_RELASZ:
                relasz = dyn->d_un.d_val;
                break;
            case DT_INIT:
                init = base + dyn->d_un.d_ptr;
                break;
            case DT_JMPREL:
                jmprel = (Elf64_Rela*)(base + dyn->d_un.d_ptr);
                break;
            case DT_INIT_ARRAY:
                init_array = (Elf64_Addr*)(base + dyn->d_un.d_ptr);
                break;
            case DT_INIT_ARRAYSZ:
                init_arraysz = dyn->d_un.d_val;
                break;
            case DT_RELRSZ:
                relrsz = dyn->d_un.d_val;
                break;
            case DT_RELR:
                relr = (Elf64_Relr*)(base + dyn->d_un.d_ptr);
                break;
        }
    }
    dyn = (Elf64_Dyn*)(base + dyn_phdr->p_vaddr);
    for (; dyn->d_tag != DT_NULL; dyn++) {
        if (dyn->d_tag == DT_NEEDED) {
            my_dlopen(strtab + dyn->d_un.d_val, RTLD_GLOBAL|RTLD_NOW);
        }
    }
    if (relr) reloc_relr(relr, relrsz);
    if (rela) reloc_rela(rela, relasz);
    if (jmprel) reloc_rela(jmprel, jmprelsz);
}
void Elf::start_init() {
    if (init) {
        logger.deb() << "Jump to init" << std::endl;
        execute_with_save((void*)init);
    }
    if (init_array) {
        size_t count = init_arraysz / sizeof(Elf64_Addr);
        for (size_t i = 0; i < count; i++) {
            logger.deb() << "Jump to init_array[" << i << "]" << std::endl;
            execute_with_save((void*)init_array[i]);
        }
    }
}
void* Elf::entry() {
    return (void*)(base + head.e_entry);
}