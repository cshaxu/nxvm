#ifndef NTVDM64_VM_COMPOSITION_DEBUG_H
#define NTVDM64_VM_COMPOSITION_DEBUG_H

#include "type.h"

#include "core/product/debug/debug_target.h"

#include "vm/composition_live_machine.h"

const core_product_debug_target *vm_composition_debug_target(
    vm_composition_live_machine *machine);
C_VOID vm_composition_debug_target_finalize(vm_composition_live_machine *machine);

#endif
