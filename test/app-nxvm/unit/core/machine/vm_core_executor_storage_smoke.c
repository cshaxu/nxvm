#include "app-nxvm/profiles/machine_factory_interface.h"
#include "lib/types/types_interface.h"
#include <stdio.h>
#include "x86/product/machine/machine_private.h"




#include "x86/core/debug_interface.h"

#include "x86/product/machine/machine_interface.h"
#include "support/rom/session_assets.h"

lib_i32 main(void)
{
    vm_machine *machine = LIB_NULL;
    core_machine_cpu_state cpu;
    lib_u32 eax;

    if (vm_test_default_pc_at_session_create(LIB_NULL, &machine) != LIB_STATUS_OK ||
        machine == LIB_NULL || machine->core_machine == LIB_NULL ||
        core_machine_get_cpu_state(machine->core_machine, &cpu) != LIB_STATUS_OK ||
        cpu.cs != 0xf000u || cpu.eip != 0xfff0u ||
        core_machine_debug_write_register(machine->core_machine,
            CORE_MACHINE_DEBUG_EAX, 0x12345678u) != LIB_STATUS_OK ||
        core_machine_debug_read_register(machine->core_machine,
            CORE_MACHINE_DEBUG_EAX, &eax) != LIB_STATUS_OK || eax != 0x12345678u) {
        vm_machine_destroy(machine);
        return 1;
    }
    vm_machine_destroy(machine);
    puts("M5:T83:S2:CORE-EXECUTOR-STORAGE:OK");
    return 0;
}
