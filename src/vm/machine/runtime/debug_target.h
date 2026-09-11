#ifndef VM_MACHINE_DEBUG_TARGET_H
#define VM_MACHINE_DEBUG_TARGET_H

#include "type.h"

#include "core/debug/debug_target.h"

#include "vm/machine/runtime/machine_interface.h"

const core_debug_target *vm_machine_debug_target(
    vm_machine *machine);
C_VOID vm_machine_debug_target_finalize(vm_machine *machine);

#endif
