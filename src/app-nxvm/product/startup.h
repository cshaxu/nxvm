#ifndef VM_APP_STARTUP_H
#define VM_APP_STARTUP_H
#include "lib/types/types_interface.h"


/* Compose NXVM.ini beside the current executable. */
lib_status vm_app_ini_executable_path(lib_u8 *path, lib_size capacity);

#endif
