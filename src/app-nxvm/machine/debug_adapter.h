#ifndef VM_MACHINE_DEBUG_TARGET_H
#define VM_MACHINE_DEBUG_TARGET_H
#include "lib/types/types_interface.h"


#include "common/machine/machine_interface.h"
#include "x86/debug/protocol_interface.h"

#include "app-nxvm/machine/machine_interface.h"

lib_status vm_machine_debug_execute(void *context,
    const void *request, lib_size request_size,
    void *response, lib_size response_capacity, lib_size *response_size);

#endif
