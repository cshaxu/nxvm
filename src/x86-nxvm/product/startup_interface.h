#ifndef X86_NXVM_PRODUCT_STARTUP_INTERFACE_H
#define X86_NXVM_PRODUCT_STARTUP_INTERFACE_H
#include "lib/types/types_interface.h"
#include "x86-nxvm/product/factory_interface.h"


/* Compose the caller-selected configuration name beside its executable. */
lib_status x86_nxvm_product_ini_executable_path(const char *name, lib_u8 *path,
    lib_size capacity);

/* NXVM-family loader passed into the shared Product entry. */
lib_status x86_nxvm_product_ini_load_request(const char *name,
    x86_nxvm_product_request *out_request);

/* NXVM-family startup owns its common INI grammar, opening identity and fixed
 * machine adapter; shared Product receives completed machine, presentation and
 * banner values. */
lib_i32 x86_nxvm_product_run(const char *name, const char *ini_name,
    const x86_nxvm_product_machine_binding *binding);

#endif
