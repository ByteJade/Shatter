#ifdef __aarch64__
#define WRAP_FUNC_VOID(func) \
    void my_##func() { \
        asm volatile("b " #func); \
    }
#define WRAP_FUNC(func) \
    void my_##func() { \
        asm volatile( \
            "stp x30, x29, [sp, #-16]!\n" \
            "bl " #func "\n" \
            "ldp x30, x29, [sp], #16\n" \
            "mov x8, x0\n" \
        ); \
    }
#define WRAP_MED_FUNC(func) \
    void my_##func() { \
        asm volatile( \
            "stp x30, x29, [sp, #-16]!\n" \
            "ldp x6, x7, [sp]\n" \
            "bl " #func "\n" \
            "ldp x30, x29, [sp], #16\n" \
            "mov x8, x0\n" \
        ); \
    }
#define WRAP_BIG_FUNC(func) \
    void my_##func() { \
        asm volatile( \
            "mov x24, x30\n" \
            "ldp x25, x26, [sp]\n" \
            "ldp x6, x7, [sp], #16\n" \
            "bl " #func "\n" \
            "stp x25, x26, [sp, #-16]!\n" \
            "mov x30, x24\n" \
            "mov x8, x0\n" \
        ); \
    }
#else
#define JUMP(func) \
    __attribute__((naked)) \
    void my_##func() { \
        asm volatile("jmp " #func); \
    }

#define WRAP_FUNC_VOID(func) JUMP(func)
#define WRAP_FUNC(func)      JUMP(func)
#define WRAP_MED_FUNC(func)  JUMP(func)
#define WRAP_BIG_FUNC(func)  JUMP(func)
#endif