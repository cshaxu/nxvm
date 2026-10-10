#ifndef VM_APP_STARTUP_INTERFACE_H
#define VM_APP_STARTUP_INTERFACE_H
#include "lib/types/types_interface.h"
#include "core/product/factory_interface.h"


/* Compose the App-selected configuration name beside its executable. */
lib_status vm_app_ini_executable_path(const char *name, lib_u8 *path,
    lib_size capacity);

/* NXVM-family loader passed into the shared Product entry. */
lib_status vm_app_ini_load_request(const char *name,
    vm_session_request *out_request);

/* NXVM-family startup owns its common INI grammar, opening identity and fixed
 * machine adapter; shared Product receives completed machine, presentation and
 * banner values. */
lib_i32 vm_app_run(const char *name, const vm_app_machine_binding *binding);

#endif
