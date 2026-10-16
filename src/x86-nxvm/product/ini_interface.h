#ifndef X86_NXVM_PRODUCT_INI_INTERFACE_H
#define X86_NXVM_PRODUCT_INI_INTERFACE_H
#include "lib/types/types_interface.h"

#include "x86-nxvm/product/request_interface.h"

/* Parse one code-owned document or load one App-selected INI. Paths are resolved
 * only against the INI directory; firmware is intentionally absent. */
lib_status x86_nxvm_product_ini_parse(const lib_u8 *directory, const lib_u8 *name,
    lib_u8 *document, x86_nxvm_product_request *out_request);
lib_status x86_nxvm_product_ini_load(const lib_u8 *path, x86_nxvm_product_request *out_request);

#endif
