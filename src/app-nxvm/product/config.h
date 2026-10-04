#ifndef VM_APP_CONFIG_H
#define VM_APP_CONFIG_H
#include "lib/types/types_interface.h"


#include "app-nxvm/profiles/machine_factory_interface.h"
#include "x86/product/request_interface.h"
#include "x86/product/machine_interface.h"

/* Combine an INI runtime request with the one generated build Profile binding.
 * The App validates user policy; Profiles alone interpret board construction. */
lib_status vm_app_configure_machine(const vm_session_request *request,
    vm_machine_config *out_config);

void vm_app_configure_factory(const vm_machine_assets *firmware,
    vm_app_factory *out_factory);

#endif
