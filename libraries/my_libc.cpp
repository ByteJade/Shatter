#include "wrapper.hpp"
#include <unistd.h>
#include <setjmp.h>

extern "C" {
    static int ret_code;
    static jmp_buf g_exit_jmp;
    
    int my___libc_start_main(
        int (*main) (int, char**, char**),
        int argc, char** argv,
        void (*init) (void), void (*fini) (void),
        void (*rtld_fini) (void), void* stack_end)
    {
        if (init) init();
        if (setjmp(g_exit_jmp) == 0) {
            ret_code = main(argc, argv, environ);
        }
        if (fini) fini();
        //if (rtld_fini) rtld_fini();
        #ifdef __x86_64__
        _exit(0);
        #endif
        return ret_code;
    }
    void my_exit(int stat) {
        ret_code = stat;
        longjmp(g_exit_jmp, 1);
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