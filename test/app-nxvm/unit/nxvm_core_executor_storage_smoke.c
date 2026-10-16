#include "x86-nxvm/machine/machine_interface.h"
#include "lib/types/types_interface.h"
#include <stdio.h>
#include "x86-nxvm/machine/machine_private.h"




#include "x86-core/machine/debug_interface.h"

#include "x86-nxvm/machine/machine_interface.h"
#include "../support/rom/session_assets.h"

lib_i32 main(void)
{
    x86_nxvm_machine *machine = LIB_NULL;
    x86_core_cpu_state cpu;
    lib_u32 eax;

    if (vm_test_default_pc_at_session_create(LIB_NULL, &machine) != LIB_STATUS_OK ||
        machine == LIB_NULL || machine->x86_core_machine == LIB_NULL ||
        x86_core_machine_get_cpu_state(machine->x86_core_machine, &cpu) != LIB_STATUS_OK ||
        cpu.cs != 0xf000u || cpu.eip != 0xfff0u ||
        x86_core_machine_debug_write_register(machine->x86_core_machine,
            X86_CORE_CPU_DEBUG_EAX, 0x12345678u) != LIB_STATUS_OK ||
        x86_core_machine_debug_read_register(machine->x86_core_machine,
            X86_CORE_CPU_DEBUG_EAX, &eax) != LIB_STATUS_OK || eax != 0x12345678u) {
        x86_nxvm_machine_destroy(machine);
        return 1;
    }
    x86_nxvm_machine_destroy(machine);
    puts("CORE-EXECUTOR-STORAGE:OK");
    return 0;
}
