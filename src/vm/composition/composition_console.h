#ifndef NTVDM64_VM_COMPOSITION_CONSOLE_H
#define NTVDM64_VM_COMPOSITION_CONSOLE_H

#include "type.h"

#include "vm/product/console_target.h"

#include "vm/composition/composition_live_machine.h"

C_VOID vm_composition_console_target_initialize(
    nxvm_product_console_target *target,
    vm_composition_live_machine *machine);

#endif
