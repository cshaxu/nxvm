#ifndef VM_APP_STARTUP_H
#define VM_APP_STARTUP_H

#include "type.h"

/* Compose NXVM.ini beside the current executable. */
type_status vm_app_ini_executable_path(C_CHAR *path, STD_SIZE_T capacity);

#endif
