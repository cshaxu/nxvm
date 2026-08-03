#include <stdio.h>
#include <stdlib.h>

#include "core/machine/keyboard_interface.h"
#include "vm/composition_machine.h"

int main(void)
{
    vm_composition_live_machine *first =
        (vm_composition_live_machine *)STD_CALLOC(1u, sizeof(*first));
    vm_composition_live_machine *second =
        (vm_composition_live_machine *)STD_CALLOC(1u, sizeof(*second));

    if (first == NULL || second == NULL) {
        STD_FREE(second);
        STD_FREE(first);
        return 1;
    }

    vm_composition_initialize(first);
    vm_composition_initialize(second);
    core_machine_keyboard_apply_host_state_to(first->keyboard_provider,
        NXVM_KEYBOARD_ASYNC_ALT, 0u);
    core_machine_keyboard_apply_host_state_to(second->keyboard_provider,
        NXVM_KEYBOARD_ASYNC_CONTROL, 0u);

    if (!core_machine_keyboard_get_modifier_from(first->keyboard_provider,
            CORE_MACHINE_KEYBOARD_MODIFIER_ALT) ||
        core_machine_keyboard_get_modifier_from(first->keyboard_provider,
            CORE_MACHINE_KEYBOARD_MODIFIER_CONTROL) ||
        core_machine_keyboard_get_modifier_from(second->keyboard_provider,
            CORE_MACHINE_KEYBOARD_MODIFIER_ALT) ||
        !core_machine_keyboard_get_modifier_from(second->keyboard_provider,
            CORE_MACHINE_KEYBOARD_MODIFIER_CONTROL)) {
        vm_composition_finalize(second);
        vm_composition_finalize(first);
        STD_FREE(second);
        STD_FREE(first);
        return 1;
    }

    vm_composition_finalize(second);
    vm_composition_finalize(first);
    STD_FREE(second);
    STD_FREE(first);
    puts("M5:T76:S1:KEYBOARD-PROVIDER-ISOLATION:OK");
    return 0;
}
