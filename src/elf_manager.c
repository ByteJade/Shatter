#define _GNU_SOURCE
#include "../inc/elf_manager.h"
#include "../inc/logger.h"
#include "../inc/dynarray.h"
#include <link.h>
#include <string.h>
#include <sys/mman.h>

#define LIB_DIR "./lib/"

typedef struct {
    const char* name;
    int refs;
    int native;
    void* data;
} library_t;
typedef struct {
    const char* symname;
    Elf64_Addr patch;
} Search_data;

static const char* error = NULL;
static library_t* libraries;

void elf_manager_init(void) {
    libraries = (library_t*)dynarray_init(sizeof(library_t));
}
void elf_manager_fini(void) {
    dynarray_fini(libraries);
}

void* elf_get_ptr(Elf64_Addr base, Elf64_Addr link) {
    if (link < base) return (void*)(base + link);
    return (void*)link;
}
static int patch_library(struct dl_phdr_info* info, size_t size, void* data) {
    (void)size;
    Search_data* search = (Search_data*)data;
    Elf64_Addr base = info->dlpi_addr;
    Elf64_Dyn* dyn = NULL;
    for (int i = 0; i < info->dlpi_phnum; i++) {
        const Elf64_Phdr* phdr = &info->dlpi_phdr[i];
        if (phdr->p_type == PT_DYNAMIC) {
            dyn = (Elf64_Dyn*)(info->dlpi_addr + phdr->p_vaddr);
            break;
        }
    }
    if (!dyn) return 0;

    const char* strtab = NULL;
    Elf64_Sym* symtab = NULL;
    Elf64_Rela *rela = NULL;
    size_t rela_size = 0;
    for (; dyn->d_tag != DT_NULL; ++dyn) {
        switch (dyn->d_tag) {
            case DT_STRTAB:
                strtab = (const char*)elf_get_ptr(base, dyn->d_un.d_ptr);
                break;
            case DT_SYMTAB:
                symtab = (Elf64_Sym*)elf_get_ptr(base, dyn->d_un.d_ptr);
                break;
            case DT_RELA:
                rela = (Elf64_Rela*)elf_get_ptr(base, dyn->d_un.d_ptr);
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
                logger_deb("Success patch %s", name);
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
void* dlopen_wrapper(const char* filename) {
    char fullpath[512];
    snprintf(
        fullpath, sizeof(fullpath),
        LIB_DIR"my_%s", filename
    );
    void* data = dlopen(fullpath, RTLD_NOW|RTLD_GLOBAL);
    if (!data) {
        logger_err("Cannot open wrapper: %s\n: %s", fullpath, dlerror());
        return NULL;
    }
    size_t lib_p = dynarray_push((void**)&libraries);
    library_t* lib = libraries + lib_p;
    lib->name = strdup(filename);
    lib->refs = 1;
    lib->native = 0;
    lib->data = data;
    logger_deb("Success wrap: %s", fullpath);
    return lib;
}
void* dlopen_native(const char* filename, int flags) {
    void* data = dlopen(filename, flags);
    if (!data) {
        logger_err("Cannot open native library: %s\n: %s", filename, dlerror());
        return NULL;
    }
    size_t lib_p = dynarray_push((void**)&libraries);
    library_t* lib = libraries + lib_p;
    lib->data = data;
    lib->name = strdup(filename);
    lib->refs = 1;
    lib->native = 1;
    logger_warn("Using native library: %s", filename);
    return lib;
}
void* my_dlopen(const char* filename, int flags) {
    for (size_t i = 0; i < dynarray_size(libraries); i++) {
        library_t* lib = libraries + i;
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
    else if (strcmp(symname, "dlclose") == 0)
        return (void*)my_dlclose;
    return NULL;
}
void* dlsym_wrapped(const char* symname) {
    char my_symbol[512];
    snprintf(
        my_symbol, sizeof(my_symbol),
        "my_%s", symname
    );
    for (size_t i = 0; i < dynarray_size(libraries); i++) {
        void* sym = dlsym(libraries[i].data, my_symbol);
        if (sym) return sym;
    }
    return NULL;
}
void* my_dlsym(void* handle, const char* symname) {
    // TODO: RTLD_*
    (void)handle;
    void* sym = dlsym_wrapped(symname);
    if (!sym) sym = dlsym_override(symname);
    if (!sym) {
        sym = dlsym(RTLD_DEFAULT, symname);
        if (sym) logger_warn("Using native symbol: %s", symname);
    }
    error = NULL;
    if (!sym) error = "Undefined symbol";
    return sym;
}
const char* my_dlerror(void) {
    if (error) {
        const char* ret = error;
        error = NULL;
        return ret;
    }
    return dlerror();
}
void my_dlclose(void* handler) {
    for (uint32_t i = 0; i < dynarray_size(libraries); i++) {
        library_t* lib = libraries + i;
        if (lib != handler) continue;
        if (--lib->refs <= 0) {
            dlclose(lib->data);
            libraries[i] = libraries[dynarray_pop(libraries)];
        }
        return;
    }
}