#ifndef VM_MACHINE_REQUEST_FACTORY_H
#define VM_MACHINE_REQUEST_FACTORY_H

#include "type.h"

#include "vm/machine/runtime/machine_interface.h"
#include "vm/machine/request_interface.h"

type_status vm_machine_create_from_request(
    const vm_session_request *request, vm_machine **out_session);

#endif
