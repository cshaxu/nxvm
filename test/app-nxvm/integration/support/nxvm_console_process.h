#ifndef TEST_INTEGRATION_SUPPORT_NXVM_CONSOLE_PROCESS_H
#define TEST_INTEGRATION_SUPPORT_NXVM_CONSOLE_PROCESS_H

#include "type.h"

C_INT nxvm_console_process_run(const C_CHAR *executable,
    const C_CHAR *session_directory, const C_CHAR *profile_file,
    const C_CHAR *const *commands, const C_CHAR *const *markers,
    STD_SIZE_T command_count);

#endif
