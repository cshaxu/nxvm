#ifndef VM_APP_CONFIG_H
#define VM_APP_CONFIG_H

#include "type.h"

#include "core/machine/machine_interface.h"
#include "app/request_interface.h"

/* Combine an INI runtime request with the one generated build Profile binding.
 * The App validates user policy; Profiles alone interpret board construction. */
type_status vm_app_configure_machine(const vm_session_request *request,
    vm_machine_config *out_config);

#endif
