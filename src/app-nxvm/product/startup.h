#ifndef VM_APP_STARTUP_H
#define VM_APP_STARTUP_H
#include "lib/types/types_interface.h"

#include "type.h"

/* Compose NXVM.ini beside the current executable. */
type_status vm_app_ini_executable_path(C_CHAR *path, lib_size capacity);

#endif
