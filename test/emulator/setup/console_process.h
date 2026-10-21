#ifndef TEST_EMULATOR_SETUP_CONSOLE_PROCESS_H
#define TEST_EMULATOR_SETUP_CONSOLE_PROCESS_H
#include "lib/types/types_interface.h"


lib_i32 emulator_console_process_run(const char *executable,
    const char *session_directory,
    const char *const *commands, const char *const *markers,
    lib_size command_count);

#endif
