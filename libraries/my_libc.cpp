#include "wrapper.hpp"
#include <unistd.h>
extern "C" {
    
    void my___libc_start_main(
        int (*main) (int, char**, char**),
        int argc, char** argv,
        void (*init) (void), void (*fini) (void),
        void (*rtld_fini) (void), void* stack_end)
    {
        if (init) init();
        int stat = main(argc, argv, environ);
        if (fini) fini();
        //if (rtld_fini) rtld_fini();
        _exit(stat);
    }
    void my_exit(int stat) {
        _exit(stat);
    }
    // IO
    WRAP_FUNC(puts)
    WRAP_FUNC(putchar)
    WRAP_BIG_FUNC(printf)
    WRAP_BIG_FUNC(sprintf)
    WRAP_BIG_FUNC(snprintf)
    WRAP_BIG_FUNC(scanf)
    WRAP_BIG_FUNC(sscanf)

    WRAP_FUNC(fflush)

    // string
    WRAP_FUNC(strlen)
    WRAP_FUNC(strcmp)
    WRAP_FUNC(strtod)
    WRAP_FUNC(strstr)

    // time
    WRAP_FUNC(gettimeofday)
    WRAP_FUNC(nanosleep)

    // env
    WRAP_FUNC(getenv)
    WRAP_FUNC(setenv)
    
    // CXA
    WRAP_FUNC_VOID(__cxa_finalize)
    WRAP_FUNC_VOID(__stack_chk_fail)
}