#include "type.h"

#include "test/integration/support/nxvm_console_process.h"

C_INT main(C_INT argc, C_CHAR **argv)
{
    static const C_CHAR *const commands[] = {"exit\r"};
    static const C_CHAR *const markers[] = {""};

    if (argc != 3 || !nxvm_console_process_run(argv[2], argv[1],
            "compaq-deskpro-386-model-40-1200k.yaml", commands, markers,
            sizeof(commands) / sizeof(commands[0]))) return 1;
    STD_PRINTF("M5:T515:S3:MODEL40-CONSOLE-YAML:OK\n");
    return 0;
}
