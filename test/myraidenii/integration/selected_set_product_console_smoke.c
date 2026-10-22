#include "lib/types/file.h"
#include "lib/types/types_interface.h"

#include "test/setup/console_process.h"

int main(lib_i32 argc, char **argv)
{
    static const char *const commands[] = {
        "start\r", "pause\r", "resume\r", "stop\r", "exit\r"};
    static const char *const markers[] = {
        "Machine started.", "Machine paused.", "Machine resumed.",
        "Machine stopped.", ""};

    if (argc != 3 || !emulator_console_process_run(argv[1], argv[2], commands, markers,
            sizeof(commands) / sizeof(commands[0]))) return 1;
    lib_c_printf("MYRAIDENII:SELECTED-SET-PRODUCT-CONSOLE:OK\n");
    return 0;
}
