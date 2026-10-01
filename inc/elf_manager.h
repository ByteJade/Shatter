#ifndef ELF_MANAGER_H
#define ELF_MANAGER_H

#include <dlfcn.h>
#include <elf.h>

void elf_manager_init(void);
void elf_manager_fini(void);

void* elf_get_ptr(Elf64_Addr base, Elf64_Addr link);

void override_got(const char* symname, void* patch);
void* my_dlopen(const char* filename, int flags);
void* my_dlsym(void* handle, const char* symname);
const char* my_dlerror(void);
void my_dlclose(void* handler);

#endif