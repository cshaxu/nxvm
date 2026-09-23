#ifndef VM_MACHINE_DEBUG_TARGET_H
#define VM_MACHINE_DEBUG_TARGET_H

#include "type.h"

#include "common/machine/machine_interface.h"
#include "x86/debug/protocol_interface.h"

#include "app-nxvm/machine/machine_interface.h"

lib_status vm_machine_debug_execute(C_VOID *context,
    const C_VOID *request, lib_size request_size,
    C_VOID *response, lib_size response_capacity, lib_size *response_size);

#endif
