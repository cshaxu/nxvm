#ifndef LIB_BASE_PROCESS_H
#define LIB_BASE_PROCESS_H

#include "lib/types/types_interface.h"

/* Implemented once by the selected host platform source. */
lib_status base_process_platform_executable_path(char *output, lib_size capacity);

#endif
