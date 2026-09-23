#include "type.h"

#include "test/app-nxvm/integration/support/nxvm_console_process.h"

C_INT main(C_INT argc, C_CHAR **argv)
{
    static const C_CHAR *const commands[] = {
        "speed turbo\r", "help\r", "info\r", "exit\r"
    };
    static const C_CHAR *const markers[] = {
        "Speed: turbo", "VM Console Commands", "Machine:           ibm-5170-model-339", ""
    };

    if (argc != 3 || !nxvm_console_process_run(argv[2], argv[1],
            "NXVM.ini", commands, markers,
            sizeof(commands) / sizeof(commands[0]))) return 1;
    STD_PRINTF("M5:T533:S4:CONSOLE-INI-LIFECYCLE:OK\n");
    return 0;
}
