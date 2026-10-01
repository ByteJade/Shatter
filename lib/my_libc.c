#include "wrapper.h"
#include <stdio.h>
#include <unistd.h>

void my___libc_start_main(
    int (*main) (int, char**, char**),
    int argc, char** argv,
    void (*init) (void), void (*fini) (void),
    void (*rtld_fini) (void), void* stack_end)
{
    // if (init) init();
    int stat = main(argc, argv, __environ);
    // if (fini) fini();
    //if (rtld_fini) rtld_fini();
    fflush(stdout);
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
WRAP_BIG_FUNC(vprintf)
WRAP_BIG_FUNC(snprintf)
WRAP_BIG_FUNC(scanf)
WRAP_BIG_FUNC(sscanf)
WRAP_BIG_FUNC(perror)
WRAP_BIG_FUNC(__printf_chk)
WRAP_BIG_FUNC(__snprintf_chk)

// file
WRAP_FUNC(fflush)
WRAP_FUNC(fflush_unlocked)
WRAP_FUNC(fputs)
WRAP_FUNC(fputs_unlocked)
WRAP_BIG_FUNC(fprintf)
WRAP_FUNC(fread)
WRAP_FUNC(opendir)
WRAP_FUNC(readdir)
WRAP_FUNC(closedir)
WRAP_FUNC(fseek)
WRAP_FUNC(fdopen)
WRAP_FUNC(dirfd)
WRAP_FUNC(fopen)
WRAP_FUNC(fclose)
WRAP_FUNC(open)
WRAP_FUNC(close)

// string
WRAP_FUNC(__strcpy_chk)
WRAP_FUNC(__stpcpy_chk)
WRAP_FUNC(strsep)
WRAP_FUNC(strlen)
WRAP_FUNC(strcmp)
WRAP_FUNC(strtod)
WRAP_FUNC(strstr)
WRAP_FUNC(strchr)
WRAP_FUNC(strncmp)
WRAP_FUNC(strrchr)
WRAP_FUNC(strcoll)
WRAP_FUNC(strnlen)
WRAP_FUNC(strspn)
WRAP_FUNC(stpcpy)
WRAP_FUNC(strncpy)
WRAP_FUNC(strdup)
WRAP_FUNC(strtol)
WRAP_FUNC(strerror)
WRAP_FUNC(atoi)

// memory
WRAP_FUNC(__memset_chk)
WRAP_FUNC(mempcpy)
WRAP_FUNC(memcpy)
WRAP_FUNC(memmove)
WRAP_FUNC(memset)
WRAP_FUNC(memcmp)
WRAP_FUNC(malloc)
WRAP_FUNC(calloc)
WRAP_FUNC(realloc)
WRAP_FUNC_VOID(free)

// time
WRAP_FUNC(time)
WRAP_FUNC(clock_gettime)
WRAP_FUNC(gettimeofday)
WRAP_FUNC(nanosleep)
WRAP_FUNC(sleep)
WRAP_FUNC(usleep)

// env
WRAP_FUNC(getenv)
WRAP_FUNC(setenv)

// CXA
WRAP_FUNC_VOID(__cxa_finalize)
WRAP_FUNC_VOID(__stack_chk_fail)

WRAP_FUNC(wait)
WRAP_FUNC(fork)
WRAP_FUNC(backtrace)
WRAP_FUNC(backtrace_symbols)