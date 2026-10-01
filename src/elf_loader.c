#include "../inc/elf_loader.h"
#include "../inc/elf_manager.h"
#include "../inc/cache.h"
#include "../inc/logger.h"
#include "../inc/launcher.h"
#include <elf.h>
#include <stdlib.h>
#include <string.h>

#define PAGE_SIZE 0x1000

void mmap_base(elf_t* elf) {
    size_t max = 0;
    size_t min = SIZE_MAX;
    for (int i = 0; i < elf->head.e_phnum; i++) {
        Elf64_Phdr* phdr = elf->pheads + i;
        if (phdr->p_type == PT_LOAD) {
            size_t start = phdr->p_vaddr;
            size_t end = start + phdr->p_memsz;
            if (start < min) min = start;
            if (end > max) max = end;
        }
    }
    min &= ~(PAGE_SIZE - 1);
    max = (max + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);
    elf->base = (Elf64_Addr)cache_mmap_guest(max - min);
    elf->base -= min;
}
void reloc_relr(elf_t* elf, Elf64_Relr* relr, int relrsz) {
    size_t count = relrsz / sizeof(Elf64_Addr);
    Elf64_Addr *where = NULL;
    Elf64_Addr base = elf->base;
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
void reloc_rela(elf_t* elf, Elf64_Rela* rela, int relasz) {
    int allright = 1;
    for (size_t i = 0; i < relasz / sizeof(Elf64_Rela); i++) {
        Elf64_Rela* rel = rela + i;
        int type = ELF64_R_TYPE(rel->r_info);
        int sym_idx = ELF64_R_SYM(rel->r_info);
        Elf64_Addr* patch = (Elf64_Addr*)(elf->base + rel->r_offset);
        Elf64_Sym* sym = elf->symtab + sym_idx;
        const char* symname = elf->strtab + sym->st_name;
        switch(type) {
            case R_X86_64_NONE: break;
            case R_X86_64_64:
                *patch = (Elf64_Addr)(sym->st_value + rel->r_addend);
                break;
            case R_X86_64_RELATIVE:
                *patch = (Elf64_Addr)(elf->base + rel->r_addend);
                break;
            case R_X86_64_JUMP_SLOT:
            case R_X86_64_GLOB_DAT: {
                void *sym_addr = my_dlsym(RTLD_DEFAULT, symname);
                if (sym_addr) {
                    *patch = (Elf64_Addr)sym_addr;
                } else if (ELF64_ST_BIND(sym->st_info) == STB_WEAK) {
                    *patch = 0;
                } else {
                    logger_err("Undefined GLOB symbol: %s", symname);
                    allright = 0;
                }
            } break;
            case R_X86_64_COPY: {
                void *sym_addr = my_dlsym(RTLD_DEFAULT, symname);
                size_t size = sym->st_size;
                if (sym_addr && size) {
                    memmove(patch, sym_addr, size);
                    override_got(symname, patch);
                } else {
                    logger_err("Undefined COPY symbol: %s", symname);
                    allright = 0;
                }
            } break;
            default:
                logger_err("Unknown RELA %i", type);
                allright = 0;
        }
    }
    if (!allright) exit(EXIT_FAILURE);
}

elf_t* elf_init(const char* filename) {
    FILE* fp = fopen(filename, "rb");
    if (fp == NULL) return NULL;
    elf_t* elf = (elf_t*)malloc(sizeof(elf_t));
    int allright = 1;
    allright &= fread(&elf->head, sizeof(Elf64_Ehdr), 1, fp) != 0;
    elf->pheads = (Elf64_Phdr*)malloc(sizeof(Elf64_Phdr) * elf->head.e_phnum);
    fseek(fp, elf->head.e_phoff, SEEK_SET);
    allright &= fread(elf->pheads, sizeof(Elf64_Phdr), elf->head.e_phnum, fp) != 0;
    mmap_base(elf);
    for (int i = 0; i < elf->head.e_phnum; i++) {
        Elf64_Phdr* phdr = elf->pheads + i;
        if (phdr->p_type == PT_LOAD) {
            char* dst = (char*)elf->base + phdr->p_vaddr;
            fseek(fp, phdr->p_offset, SEEK_SET);
            allright &= fread(dst, 1, phdr->p_filesz, fp) != 0;
            if (phdr->p_filesz != phdr->p_memsz) {
                memset(dst + phdr->p_filesz, 0,
                    phdr->p_memsz - phdr->p_filesz
                );
            }
        }
    }
    fclose(fp);
    if (!allright) {
        elf_fini(elf);
        return NULL;
    }
    return elf;
}
void elf_read_dynamic(elf_t* elf) {
    Elf64_Phdr* dyn_phdr = NULL;
    for (int i = 0; i < elf->head.e_phnum; i++) {
        Elf64_Phdr* phdr = elf->pheads + i;
        if (phdr->p_type == PT_DYNAMIC) {
            dyn_phdr = phdr;
            break;
        }
    }
    if (dyn_phdr == NULL) {
        logger_err("No dynamic section");
        exit(EXIT_FAILURE);
    }
    Elf64_Dyn* dyn = (Elf64_Dyn*)(elf->base + dyn_phdr->p_vaddr);
    size_t relrsz = 0;
    Elf64_Relr* relr = NULL;
    size_t relasz = 0;
    Elf64_Rela* rela = NULL;
    size_t jmprelsz = 0;
    Elf64_Rela* jmprel = NULL;
    for (; dyn->d_tag != DT_NULL; dyn++) {
        switch (dyn->d_tag) {
            case DT_PLTRELSZ:
                jmprelsz = dyn->d_un.d_val;
                break;
            case DT_STRTAB:
                elf->strtab = (char*)(elf->base + dyn->d_un.d_ptr);
                break;
            case DT_SYMTAB:
                elf->symtab = (Elf64_Sym*)(elf->base + dyn->d_un.d_ptr);
                break;
            case DT_RELA:
                rela = (Elf64_Rela*)(elf->base + dyn->d_un.d_ptr);
                break;
            case DT_RELASZ:
                relasz = dyn->d_un.d_val;
                break;
            case DT_INIT:
                elf->init = dyn->d_un.d_ptr;
                break;
            case DT_JMPREL:
                jmprel = (Elf64_Rela*)(elf->base + dyn->d_un.d_ptr);
                break;
            case DT_INIT_ARRAY:
                elf->init_array = (Elf64_Addr*)(elf->base + dyn->d_un.d_ptr);
                break;
            case DT_INIT_ARRAYSZ:
                elf->init_arraysz = dyn->d_un.d_val;
                break;
            case DT_RELRSZ:
                relrsz = dyn->d_un.d_val;
                break;
            case DT_RELR:
                relr = (Elf64_Relr*)(elf->base + dyn->d_un.d_ptr);
                break;
        }
    }
    dyn = (Elf64_Dyn*)(elf->base + dyn_phdr->p_vaddr);
    for (; dyn->d_tag != DT_NULL; dyn++) {
        if (dyn->d_tag == DT_NEEDED) {
            my_dlopen(elf->strtab + dyn->d_un.d_val, RTLD_GLOBAL|RTLD_NOW);
        }
    }
    if (relr) reloc_relr(elf, relr, relrsz);
    if (rela) reloc_rela(elf, rela, relasz);
    if (jmprel) reloc_rela(elf, jmprel, jmprelsz);
}
void elf_start(elf_t* elf) {
    if (elf->init) {
        logger_deb("Jump to init");
        execute(elf_get_ptr(elf->base, elf->init));
    }
    if (elf->init_array) {
        size_t count = elf->init_arraysz / sizeof(Elf64_Addr);
        for (size_t i = 0; i < count; i++) {
            logger_deb("Jump to init_array[%i]", i);
            execute(elf_get_ptr(elf->base, elf->init_array[i]));
        }
    }
}
void elf_fini(elf_t* elf) {
    if (elf) {
        free(elf->pheads);
        free(elf);
    }
}
