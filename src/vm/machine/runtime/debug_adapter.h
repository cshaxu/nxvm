#ifndef VM_MACHINE_DEBUG_TARGET_H
#define VM_MACHINE_DEBUG_TARGET_H

#include "type.h"

#include "common/machine/machine_interface.h"

#include "vm/machine/runtime/machine_interface.h"

lib_status vm_machine_common_debug_execute(void *context,
    const common_machine_debug_request *request,
    common_machine_debug_result *out_result);

#endif
