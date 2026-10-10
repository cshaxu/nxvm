#ifndef TEST_INTEGRATION_SUPPORT_NXVM_CONSOLE_PROCESS_H
#define TEST_INTEGRATION_SUPPORT_NXVM_CONSOLE_PROCESS_H
#include "lib/types/types_interface.h"


lib_i32 nxvm_console_process_run(const char *executable,
    const char *session_directory, const char *profile_file,
    const char *const *commands, const char *const *markers,
    lib_size command_count);

#endif
