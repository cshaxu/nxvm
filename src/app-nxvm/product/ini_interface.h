#ifndef VM_APP_INI_INTERFACE_H
#define VM_APP_INI_INTERFACE_H
#include "lib/types/types_interface.h"

#include "app-nxvm/product/request_interface.h"

/* Parse one code-owned document or load one NXVM.ini file.  Paths are resolved
 * only against the INI directory; firmware is intentionally absent. */
lib_status vm_app_ini_parse(const lib_u8 *directory, const lib_u8 *name,
    lib_u8 *document, vm_session_request *out_request);
lib_status vm_app_ini_load(const lib_u8 *path, vm_session_request *out_request);

#endif
