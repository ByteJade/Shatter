#pragma once

#include <dlfcn.h>

void override_got(const char* symname, void* patch);
void* my_dlopen(const char* filename, int flags);
void* my_dlsym(void* handle, const char* symname);
const char* my_dlerror();
void my_dlclose(void* handler);