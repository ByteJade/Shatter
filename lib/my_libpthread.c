#include "wrapper.h"

WRAP_FUNC(pthread_create)
WRAP_FUNC(pthread_equal)
WRAP_FUNC(pthread_self)
WRAP_FUNC(pthread_join)
WRAP_FUNC(pthread_once)
WRAP_FUNC(pthread_cancel)
WRAP_FUNC(pthread_sigmask)
WRAP_FUNC(pthread_setspecific)
WRAP_FUNC(__pthread_register_cancel)
WRAP_FUNC(__pthread_unregister_cancel)
WRAP_FUNC(__pthread_unwind_next)

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