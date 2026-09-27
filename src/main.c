#include "../inc/logger.h"
#include "../inc/debugger.h"
#include "../inc/stack.h"
#include "../inc/elf_loader.h"
#include "../inc/elf_manager.h"
#include "../inc/launcher.h"
#include "../inc/handler.h"
#include <stdlib.h>

[[noreturn]] void usage(void) {
    printf("usage: shatter <argv> \"filename\" <argv\n>");
    printf("\t-d : enable debug mode\n");
    printf("\t-l <level> : set logger_log level\n");
    printf("\t-h : print this message\n");
    exit(EXIT_SUCCESS);
}

int parce_argv(int argc, char** argv) {
    for (int argp = 1; argp < argc; argp++) {
        char* arg = argv[argp];
        if (*arg != '-') return argp;
        switch (*++arg) {
            case 'd':
                debugger_init();
                break;
            case 'l':
                logger_set_level(argv[++argp]);
                break;
            default: usage();
        }
    }
    return 0;
}

int main(int argc, char** argv, char** envp) {
    int user_argc = parce_argv(argc, argv);
    if (!user_argc) {
        logger_err("Wait: filename");
        usage();
    }
    elf_manager_init();
    stack_t* stack = stack_init();
    stack_setup(stack, argc - user_argc, argv + user_argc, envp);
    elf_t* elf = elf_init(argv[user_argc]);
    if (!elf) {
        logger_err("Cannot open file");
        return EXIT_FAILURE;
    }
    handler_init();
    elf_read_dynamic(elf);
    elf_start(elf);
    launch((void*)(elf->base + elf->head.e_entry), stack);

    elf_fini(elf);
    stack_fini(stack);
    elf_manager_fini();
    debugger_fini();
    return EXIT_SUCCESS;
}
