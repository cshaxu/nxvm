#include <stdio.h>

#include "core/machine/keyboard_interface.h"
#include "vm/composition_machine.h"

int main(void)
{
    vm_composition_live_machine session = {0};
    machineInit(&session);
    core_machine_keyboard_apply_host_state(NXVM_KEYBOARD_ASYNC_ALT, 0u);
    if (!core_machine_keyboard_get_modifier(CORE_MACHINE_KEYBOARD_MODIFIER_ALT)) {
        machineFinal(&session);
        return 1;
    }
    machineFinal(&session);
    puts("M5:T41:S1:KEYBOARD-FIRMWARE:OK");
    return 0;
}
