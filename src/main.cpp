#include "../include/Logger.hpp"
#include "../include/Stack.hpp"
#include "../include/Elf.hpp"
#include "../include/Debugger.hpp"
#include "../include/Launcher.hpp"
#include "../include/Handler.hpp"
#include <cstdlib>
#include <cstring>

[[noreturn]] void usage() {
    logger.force() << "Usage:" << std::endl
     << "shatter <args> \"programm\" <args>" << std::endl
     << "\t-level <level> : set log level" << std::endl
     << "\t-debug : enable debugger" << std::endl;
    exit(EXIT_SUCCESS);
}

int parse_argv(int argc, char** argv) {
    for (int argp = 1; argp < argc; argp++) {
        const char* arg = argv[argp];
        if (arg[0] != '-') return argp;
        if (strcmp(arg, "-level") == 0) {
            logger.set_level(argv[++argp]);
        } else if (strcmp(arg, "-debug") == 0) {
            debugger.enable();
        } else break;
    }
    usage();
}

void setup_handlers() {
    struct sigaction sa_trap = {0, {0}, SA_SIGINFO, 0};
    sa_trap.sa_sigaction = brk_handler;
    struct sigaction sa_segv {0, {0}, SA_SIGINFO, 0};
    sa_segv.sa_sigaction = segv_handler;
    struct sigaction sa_segi {0, {0}, SA_SIGINFO, 0};
    sa_segi.sa_sigaction = segi_handler;
    sigaction(SIGTRAP, &sa_trap, NULL);
    sigaction(SIGSEGV, &sa_segv, nullptr);
    sigaction(SIGILL, &sa_segv, nullptr);
    sigaction(SIGBUS, &sa_segv, nullptr);
    sigaction(SIGINT, &sa_segi, nullptr);
}

int main(int argc, char** argv, char** envp) {
    int user_argc = parse_argv(argc, argv);
    Stack stack;
    stack.setup(argc - user_argc, argv + user_argc, envp);
    setup_handlers();
    Elf file;
    if (!file.open(argv[user_argc])) {
        logger.err() << "Cannot open file" << std::endl;
        return EXIT_FAILURE;
    }
    file.read_dynamic();
    file.start_init();

    execute_with_stack(file.entry(), stack.get());

    // file.close();
    return EXIT_SUCCESS;
}