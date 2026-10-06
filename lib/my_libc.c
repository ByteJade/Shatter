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
WRAP_FUNC_VOID(_exit)
// IO
WRAP_FUNC(puts)
WRAP_FUNC(putchar)
WRAP_BIG_FUNC(printf)
WRAP_BIG_FUNC(sprintf)
WRAP_BIG_FUNC(snprintf)
WRAP_BIG_FUNC(scanf)
WRAP_BIG_FUNC(sscanf)
WRAP_BIG_FUNC(perror)
WRAP_BIG_FUNC(__printf_chk)
WRAP_BIG_FUNC(__fprintf_chk)
WRAP_BIG_FUNC(__asprintf_chk)
WRAP_BIG_FUNC(__snprintf_chk)
WRAP_BIG_FUNC(__vsnprintf_chk)

// file
WRAP_FUNC(fflush)
WRAP_FUNC(fflush_unlocked)
WRAP_FUNC(fputs)
WRAP_FUNC(fputs_unlocked)
WRAP_BIG_FUNC(fprintf)
WRAP_FUNC(fread)
WRAP_FUNC(fwrite)
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
WRAP_FUNC(mkdir)
WRAP_FUNC(fileno)

// string
WRAP_FUNC(__strcpy_chk)
WRAP_FUNC(__stpcpy_chk)
WRAP_FUNC(__strcat_chk)
WRAP_FUNC(__realpath_chk)
WRAP_FUNC(realpath)
WRAP_FUNC(strcasecmp)
WRAP_FUNC(strcpy)
WRAP_FUNC(strtok)
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
WRAP_FUNC(getopt_long)

// memory
WRAP_FUNC(__memset_chk)
WRAP_FUNC(__memcpy_chk)
WRAP_FUNC(__memmove_chk)
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
WRAP_FUNC(ctime)
WRAP_FUNC(clock_getres)
WRAP_FUNC(clock_gettime)
WRAP_FUNC(gettimeofday)
WRAP_FUNC(nanosleep)
WRAP_FUNC(sleep)
WRAP_FUNC(usleep)

// env
WRAP_FUNC(getenv)
WRAP_FUNC(setenv)
WRAP_FUNC(getauxval)

// CXA
WRAP_FUNC_VOID(__cxa_finalize)
WRAP_FUNC_VOID(__stack_chk_fail)

WRAP_FUNC(wait)
WRAP_FUNC(fork)
WRAP_FUNC(rand)
WRAP_FUNC(srand)
WRAP_FUNC(isatty)

void my_vprintf() {
    printf("TODO: my_vprintf\n");
}
void my_backtrace() {
    printf("TODO: my_backtrace\n");
}
void my_backtrace_symbols() {
    printf("TODO: my_backtrace_symbols\n");
}
int syscall_override[] = {
    [0] = 63,
    [1] = 64,
    [9] = 222,
    [11] = 215,
};
long my_syscall(long sysno, long arg1, long arg2, long arg3, long arg4, long arg5, long arg6) {
    long ret = syscall(syscall_override[sysno], arg1, arg2, arg3, arg4, arg5, arg6);
    #ifdef __aarch64__
    asm volatile (
        "mov x8, %0\n"
        : : "r"(ret)
        : "memory"
    );
    #endif
    return ret;
}