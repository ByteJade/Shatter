#include "wrapper.h"
#include "my_libc.c"
#include <pthread.h>

WRAP_FUNC(pthread_create)
WRAP_FUNC(pthread_equal)
WRAP_FUNC(pthread_self)
WRAP_FUNC(pthread_join)
WRAP_FUNC(pthread_once)
WRAP_FUNC(pthread_cancel)
WRAP_FUNC(pthread_sigmask)
WRAP_FUNC(pthread_setspecific)

void my___pthread_register_cancel(__pthread_unwind_buf_t* buf) {
    __pthread_unwind_buf_t arm;
    armfx86((__jmp_buf_tag_x86*)buf->__cancel_jmp_buf, (__jmp_buf_tag_arm*)arm.__cancel_jmp_buf);
    __pthread_register_cancel(&arm);
}
void my___pthread_unregister_cancel(__pthread_unwind_buf_t* buf) {
    __pthread_unwind_buf_t arm;
    armfx86((__jmp_buf_tag_x86*)buf->__cancel_jmp_buf, (__jmp_buf_tag_arm*)arm.__cancel_jmp_buf);
    __pthread_unregister_cancel(&arm);
}
void my___pthread_unwind_next(__pthread_unwind_buf_t* buf) {
    __pthread_unwind_buf_t arm;
    armfx86((__jmp_buf_tag_x86*)buf->__cancel_jmp_buf, (__jmp_buf_tag_arm*)arm.__cancel_jmp_buf);
    __pthread_unwind_next(&arm);
}

WRAP_FUNC(pthread_cond_init)
WRAP_FUNC(pthread_cond_destroy)
WRAP_FUNC(pthread_cond_wait)
WRAP_FUNC(pthread_cond_broadcast)

WRAP_FUNC(pthread_attr_init)
WRAP_FUNC(pthread_attr_destroy)
WRAP_FUNC(pthread_attr_setscope)
WRAP_FUNC(pthread_attr_setguardsize)
WRAP_FUNC(pthread_attr_setstack)

WRAP_FUNC(pthread_mutex_init)
WRAP_FUNC(pthread_mutex_destroy)
WRAP_FUNC(pthread_mutex_lock)
WRAP_FUNC(pthread_mutex_unlock)

WRAP_FUNC(pthread_mutexattr_init)
WRAP_FUNC(pthread_mutexattr_destroy)
WRAP_FUNC(pthread_mutexattr_settype)