#ifndef VM_APP_INI_INTERFACE_H
#define VM_APP_INI_INTERFACE_H
#include "lib/types/types_interface.h"

#include "type.h"
#include "app-nxvm/product/request_interface.h"

/* Parse one code-owned document or load one NXVM.ini file.  Paths are resolved
 * only against the INI directory; firmware is intentionally absent. */
type_status vm_app_ini_parse(const C_CHAR *directory, const C_CHAR *name,
    C_CHAR *document, vm_session_request *out_request);
type_status vm_app_ini_load(const C_CHAR *path, vm_session_request *out_request);

#endif
