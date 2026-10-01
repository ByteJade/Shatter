#include "wrapper.h"

WRAP_FUNC(pthread_create)
WRAP_FUNC(pthread_equal)
WRAP_FUNC(pthread_self)
WRAP_FUNC(pthread_join)
WRAP_FUNC(pthread_cancel)
WRAP_FUNC(__pthread_register_cancel)
WRAP_FUNC(__pthread_unregister_cancel)
WRAP_FUNC(__pthread_unwind_next)

WRAP_FUNC(pthread_cond_init)
WRAP_FUNC(pthread_cond_destroy)
WRAP_FUNC(pthread_cond_wait)
WRAP_FUNC(pthread_cond_broadcast)

WRAP_FUNC(pthread_attr_init)
WRAP_FUNC(pthread_attr_destroy)

WRAP_FUNC(pthread_mutex_init)
WRAP_FUNC(pthread_mutex_destroy)
WRAP_FUNC(pthread_mutex_lock)
WRAP_FUNC(pthread_mutex_unlock)