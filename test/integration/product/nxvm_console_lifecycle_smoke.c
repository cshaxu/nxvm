#include "type.h"

#include "test/integration/support/nxvm_console_process.h"

C_INT main(C_INT argc, C_CHAR **argv)
{
    static const C_CHAR *const commands[] = {
        "speed turbo\r", "help\r", "info\r", "exit\r"
    };
    static const C_CHAR *const markers[] = {
        "Speed: turbo", "VM Console Commands", "Device Info", ""
    };

    if (argc != 3 || !nxvm_console_process_run(argv[2], argv[1],
            "ibm-5170-model-339-1200k.yaml", commands, markers,
            sizeof(commands) / sizeof(commands[0]))) return 1;
    STD_PRINTF("M5:T515:S3:CONSOLE-YAML-LIFECYCLE:OK\n");
    return 0;
}
