#ifndef VM_APP_STARTUP_INTERFACE_H
#define VM_APP_STARTUP_INTERFACE_H
#include "lib/types/types_interface.h"
#include "ibmpc/product/request_interface.h"


/* Compose the App-selected configuration name beside its executable. */
lib_status vm_app_ini_executable_path(const char *name, lib_u8 *path,
    lib_size capacity);

/* NXVM-family loader passed into the shared Product entry. */
lib_status vm_app_ini_load_request(const char *name,
    vm_session_request *out_request);

#endif
