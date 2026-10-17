#ifndef LIB_BASE_PROCESS_INTERFACE_H
#define LIB_BASE_PROCESS_INTERFACE_H

#include "lib/types/types_interface.h"

/* Copies the current executable's containing directory as NUL-terminated UTF-8,
 * without a trailing separator except for a filesystem root. Capacity is bytes
 * including NUL and must accommodate the full executable path. Insufficient
 * capacity returns LIMIT_EXCEEDED; an unrepresentable native name is UNSUPPORTED.
 * POSIX non-UTF-8 filenames are not supported. On failure, output is unchanged. */
lib_status base_process_executable_directory(char *output, lib_size capacity);

#endif
