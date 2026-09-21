#ifndef VM_APP_CONFIG_H
#define VM_APP_CONFIG_H

#include "type.h"

#include "core/machine/machine_interface.h"
#include "app/request_interface.h"

/* Resolve product configuration into the public VM-machine construction
 * contract.  This owner parses names and validates product policy; the
 * machine owner alone creates the concrete machine. */
type_status vm_app_configure_machine(const vm_session_request *request,
    vm_machine_config *out_config);

#endif
