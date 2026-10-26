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
    int stat = main(argc, argv, NULL);
    // if (fini) fini();
    //if (rtld_fini) rtld_fini();
    fflush(stdout);
    _exit(stat);
}
void my_exit(int stat) {
    _exit(stat);
}
WRAP_VOID_FUNC(_exit)
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
WRAP_FUNC(fgetc)
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
WRAP_VOID_FUNC(rewind)

// string
WRAP_FUNC(__strcpy_chk)
WRAP_FUNC(__stpcpy_chk)
WRAP_FUNC(__strcat_chk)
WRAP_FUNC(__realpath_chk)
WRAP_FUNC(__isoc23_strtol)
WRAP_FUNC(__isoc23_sscanf)
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
WRAP_FUNC(optarg)

// memory
WRAP_FUNC(__memset_chk)
WRAP_FUNC(__memcpy_chk)
WRAP_FUNC(__memmove_chk)
WRAP_FUNC(mempcpy)
WRAP_FUNC(memcpy)
WRAP_FUNC(memmove)
WRAP_FUNC(memset)
WRAP_FUNC(memcmp)
WRAP_FUNC(memchr)
WRAP_FUNC(malloc)
WRAP_FUNC(calloc)
WRAP_FUNC(realloc)
WRAP_VOID_FUNC(free)

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
WRAP_FUNC(getrusage)
WRAP_FUNC(sysconf)

// CXA
WRAP_VOID_FUNC(__cxa_finalize)
WRAP_VOID_FUNC(__stack_chk_fail)

WRAP_FUNC(wait)
WRAP_FUNC(fork)
WRAP_FUNC(rand)
WRAP_FUNC(srand)
WRAP_FUNC(isatty)

typedef struct {
    unsigned int gp_offset;
    unsigned int fp_offset;
    char *overflow_arg_area;
    char *reg_save_area;
} my_va_list;

size_t my_va_arg_r(void* vlist) {
    my_va_list *ap = (my_va_list *)vlist;
    size_t value;
    if (ap->gp_offset < 48) {
        char *reg_ptr = ap->reg_save_area + ap->gp_offset;
        value = *(size_t *)reg_ptr;
        ap->gp_offset += 8;
    } else {
        value = *(size_t *)ap->overflow_arg_area;
        ap->overflow_arg_area += 8;
    }
    return value;
}
double my_va_arg_x(void* vlist) {
    double value;
    my_va_list *ap = (my_va_list *)vlist;
    if (ap->fp_offset < 176) {
        char *reg_ptr = ap->reg_save_area + ap->fp_offset;
        value = *(double *)reg_ptr;
        ap->fp_offset += 16;
    } else {
        value = *(double *)ap->overflow_arg_area;
        ap->overflow_arg_area += 8;
    }
    return value;
}
int construct_format(const char* src, char* dst) {
    int pos = 0;
    dst[pos++] = '%';
    char c = *src++;
    dst[pos++] = c;
    if (c == 'l') {
        dst[pos++] = *src++;
    }
    dst[pos] = '\0';
    return pos-1;
}
size_t my_vsnprintf(char *buffer, size_t bufsz, const char *format, void* vlist) {
    char c;
    size_t cur = 0;
    size_t avail = bufsz;
    char* top = buffer;
    do {
        c = *format++;
        if (c == '%') {
            char nformat[10];
            int pos = construct_format(format, nformat);
            if (nformat[pos] == 'f' || nformat[pos] == 'g') {
                cur += snprintf(top, avail, nformat, my_va_arg_x(vlist));
            }else {
                cur += snprintf(top, avail, nformat, my_va_arg_r(vlist));
            }
            format += pos;
        } else {
            if (buffer && cur < bufsz) *top = c;
            cur++;
        }
        if (buffer && cur < bufsz) {
            top = buffer + cur;
            avail = bufsz - cur;
        } else {
            top = NULL;
            avail = 0;
        }
    } while (c != '\0');
    WRAPPED_RET(cur);
    return cur;
}
void my_vprintf(const char *format, void* vlist) {
    char buffer[256];
    size_t cur = my_vsnprintf(buffer, sizeof(buffer), format, vlist);
    printf("%s", buffer);
    WRAPPED_RET(cur);
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
    WRAPPED_RET(ret);
    return ret;
}