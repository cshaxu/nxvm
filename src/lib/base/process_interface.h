#ifndef LIB_BASE_PROCESS_INTERFACE_H
#define LIB_BASE_PROCESS_INTERFACE_H

#include "lib/types/types_interface.h"

/* Copies the current executable's containing directory, without a trailing
 * separator except for a filesystem root. On failure, output is unchanged. */
lib_status base_process_executable_directory(char *output, lib_size capacity);

#endif
