#include "lib/types/types_interface.h"
#include <stdio.h>

#include "test/app-nxvm/integration/support/nxvm_console_process.h"

lib_i32 main(lib_i32 argc, char **argv)
{
    static const char *const commands[] = {"exit\r"};
    static const char *const markers[] = {""};

    if (argc != 3 || !nxvm_console_process_run(argv[2], argv[1],
            "NXVM.ini", commands, markers,
            sizeof(commands) / sizeof(commands[0]))) return 1;
    printf("M5:T533:S4:MODEL40-CONSOLE-INI:OK\n");
    return 0;
}
