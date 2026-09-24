#include "lib/types/types_interface.h"
#include <stdio.h>

#include "test/app-nxvm/integration/support/nxvm_console_process.h"

lib_i32 main(lib_i32 argc, char **argv)
{
    static const char *const commands[] = {
        "speed turbo\r", "help\r", "info\r", "exit\r"
    };
    static const char *const markers[] = {
        "Speed: turbo", "VM Console Commands", "Machine:           ibm-5170-model-339", ""
    };

    if (argc != 3 || !nxvm_console_process_run(argv[2], argv[1],
            "NXVM.ini", commands, markers,
            sizeof(commands) / sizeof(commands[0]))) return 1;
    printf("M5:T533:S4:CONSOLE-INI-LIFECYCLE:OK\n");
    return 0;
}
