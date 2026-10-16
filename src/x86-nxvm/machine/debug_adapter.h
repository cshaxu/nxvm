#ifndef X86_NXVM_MACHINE_DEBUG_TARGET_H
#define X86_NXVM_MACHINE_DEBUG_TARGET_H
#include "lib/types/types_interface.h"


#include "emulator/machine/machine_interface.h"
#include "x86-debug/product/protocol_interface.h"

#include "x86-nxvm/machine/machine_interface.h"

lib_status x86_nxvm_machine_debug_execute(void *context,
    const void *request, lib_size request_size,
    void *response, lib_size response_capacity, lib_size *response_size);

#endif
