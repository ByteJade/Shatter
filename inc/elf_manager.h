#ifndef ELF_MANAGER_H
#define ELF_MANAGER_H

#include <dlfcn.h>
#include "elf_loader.h"

#define LIB_DIR "./lib/"

void elf_manager_init(void);
void elf_manager_fini(void);
elf_t* elf_manager_set_executable(const char* name);

void override_got(const char* symname, void* patch);
void* my_dlopen(const char* filename, int flags);
void* my_dlsym(void* handle, const char* symname);
const char* my_dlerror(void);
void my_dlclose(void* handler);

#endif