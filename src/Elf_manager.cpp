#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#include <cstdint>
#include <dlfcn.h>
#endif
#include "../include/Elf_manager.hpp"
#include "../include/Logger.hpp"
#include <vector>
#include <cstring>
#include <link.h>
#include <sys/mman.h>

#define LIB_DIR "./lib/"

struct Library {
    const char* name;
    int refs;
    bool native;
    void* data;
};
struct Search_data {
    const char* symname;
    Elf64_Addr patch;
};

static const char* error = nullptr;
static std::vector<Library*> libraries;

void* get_ptr(uint64_t base, uint64_t link) {
    if (link < base) return (void*)(base + link);
    return (void*)link;
}
static int patch_library(struct dl_phdr_info* info, size_t size, void* data) {
    ((void)size);
    Search_data* search = (Search_data*)data;
    Elf64_Addr base = info->dlpi_addr;
    Elf64_Dyn* dyn = nullptr;
    for (int i = 0; i < info->dlpi_phnum; i++) {
        const Elf64_Phdr* phdr = &info->dlpi_phdr[i];
        if (phdr->p_type == PT_DYNAMIC) {
            dyn = (Elf64_Dyn*)(info->dlpi_addr + phdr->p_vaddr);
            break;
        }
    }
    if (!dyn) return 0;

    const char* strtab = nullptr;
    Elf64_Sym* symtab = nullptr;
    Elf64_Rela *rela = nullptr;
    size_t rela_size = 0;
    for (; dyn->d_tag != DT_NULL; ++dyn) {
        switch (dyn->d_tag) {
            case DT_STRTAB:
                strtab = (const char*)get_ptr(base, dyn->d_un.d_ptr);
                break;
            case DT_SYMTAB:
                symtab = (Elf64_Sym*)get_ptr(base, dyn->d_un.d_ptr);
                break;
            case DT_RELA:
                rela = (Elf64_Rela*)get_ptr(base, dyn->d_un.d_ptr);
                break;
            case DT_RELASZ:
                rela_size = dyn->d_un.d_val;
                break;
        }
    }
    if (!(strtab && symtab && rela)) return 0;
    size_t count = rela_size / sizeof(Elf64_Rela);
    for (size_t i = 0; i < count; i++) {
        uint32_t idx = ELF64_R_SYM(rela[i].r_info);
        uint32_t type = ELF64_R_TYPE(rela[i].r_info);
        Elf64_Addr* patch = (Elf64_Addr*)(base + rela[i].r_offset);
        if (type == R_X86_64_JUMP_SLOT || type == R_X86_64_GLOB_DAT ||
            type == R_AARCH64_JUMP_SLOT || type == R_AARCH64_GLOB_DAT) {
            const char* name = strtab + symtab[idx].st_name;
            if (strcmp(name, search->symname) == 0) {
                logger.deb() << "Success patch " << name << std::endl;
                void* page_start = (void*)(((uint64_t)patch) & ~(0x1000 - 1));
                mprotect(page_start, 0x1000, PROT_READ | PROT_WRITE);
                *patch = (Elf64_Addr)search->patch;
            }
        }
    }
    return 0;
}
void override_got(const char* symname, void* patch) {
    Search_data data = {symname, (Elf64_Addr)patch};
    dl_iterate_phdr(patch_library, &data);
}
// TODO: dlopen_guest()
void* dlopen_wrapper(const char* filename) {
    char fullpath[512];
    Library* lib = new Library;
    snprintf(
        fullpath, sizeof(fullpath),
        LIB_DIR"my_%s", filename
    );
    lib->data = dlopen(fullpath, RTLD_NOW|RTLD_GLOBAL);
    if (!lib->data) {
        logger.err() << "Cannot open wrapper: " << fullpath << std::endl
         << ": " << dlerror() << std::endl;
        return nullptr;
    }
    lib->name = strdup(filename);
    lib->refs = 1;
    lib->native = 0;
    logger.deb() << "Success wrap " << fullpath << std::endl;
    libraries.push_back(lib);
    return libraries.back();
}
void* dlopen_native(const char* filename, int flags) {
    Library* lib = new Library;
    lib->data = dlopen(filename, flags);
    lib->name = strdup(filename);
    lib->refs = 1;
    lib->native = 1;
    logger.warn() << "Using native library: " << filename << std::endl;
    libraries.push_back(lib);
    return libraries.back();
}
void* my_dlopen(const char* filename, int flags) {
    for (Library* lib : libraries) {
        if (strcmp(lib->name, filename) == 0) {
            lib->refs++;
            return lib;
        }
    }
    void* lib = dlopen_wrapper(filename);
    if (!lib) lib = dlopen_native(filename, flags);
    if (!lib) error = "No such file or directory";
    return lib;
}

void* dlsym_override(const char* symname) {
    if (strcmp(symname, "dlopen") == 0)
        return (void*)my_dlopen;
    else if (strcmp(symname, "dlsym") == 0)
        return (void*)my_dlsym;
    else if (strcmp(symname, "dlerror") == 0)
        return (void*)my_dlerror;
    return nullptr;
}
void* dlsym_wrapped(const char* symname) {
    char my_symbol[512];
    snprintf(
        my_symbol, sizeof(my_symbol),
        "my_%s", symname
    );
    for (Library* lib : libraries) {
        void* sym = dlsym(lib->data, my_symbol);
        if (sym) return sym;
    }
    return nullptr;
}
void* my_dlsym(void* handle, const char* symname) {
    // TODO: RTLD_*
    (void)handle;
    void* sym = dlsym_wrapped(symname);
    if (!sym) sym = dlsym_override(symname);
    if (!sym) {
        sym = dlsym(RTLD_DEFAULT, symname);
        if (sym) logger.warn() << "Using native symbol: " << symname << std::endl;
    }
    error = nullptr;
    if (!sym) error = "Undefined symbol";
    return sym;
}
const char* my_dlerror() {
    if (error) {
        const char* ret = error;
        error = nullptr;
        return ret;
    }
    return dlerror();
}
void my_dlclose(void* handler) {
    for (uint32_t i = 0; i < libraries.size(); i++) {
        Library* lib = libraries[i];
        if (lib != handler) continue;
        if (--lib->refs > 0) break;
        dlclose(lib->data);
        free(lib);
        if (libraries.size() != 0) {
            libraries[i] = libraries.back();
            libraries.pop_back();
        }
    }
}