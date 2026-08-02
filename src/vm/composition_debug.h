#ifndef NTVDM64_VM_COMPOSITION_DEBUG_H
#define NTVDM64_VM_COMPOSITION_DEBUG_H

#include "core/product/debug/debug_target.h"
#include "vm/composition_live_machine.h"

const core_product_debug_target *vm_composition_debug_target(
    vm_composition_live_machine *machine);

#endif
