#include <stdio.h>
#include <stdlib.h>

#include "core/machine/keyboard_interface.h"
#include "vm/composition_machine.h"

int main(void)
{
    vm_composition_live_machine *session =
        (vm_composition_live_machine *)STD_CALLOC(1u, sizeof(*session));
    if (session == NULL) return 1;
    vm_composition_initialize(session);
    core_machine_keyboard_apply_host_state_to(session->keyboard_provider,
        NXVM_KEYBOARD_ASYNC_ALT, 0u);
    if (!core_machine_keyboard_get_modifier_from(session->keyboard_provider,
        CORE_MACHINE_KEYBOARD_MODIFIER_ALT)) {
        vm_composition_finalize(session);
        STD_FREE(session);
        return 1;
    }
    vm_composition_finalize(session);
    STD_FREE(session);
    puts("M5:T41:S1:KEYBOARD-FIRMWARE:OK");
    return 0;
}
