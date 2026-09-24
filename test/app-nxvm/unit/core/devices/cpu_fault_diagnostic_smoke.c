#include "lib/types/types_interface.h"
#include <stdio.h>
#include "app-nxvm/devices/device_support.h"

#include "app-nxvm/devices/cpu.h"
#include "app-nxvm/devices/debug_interface.h"
#include "app-nxvm/devices/machine_interface.h"
#include "app-nxvm/devices/memory.h"
#include "support/core_machine_cpu_fixture.h"

lib_i32 main(void)
{
    const core_machine_config config = {
        .memory_bytes = CORE_MACHINE_DEFAULT_MEMORY_BYTES
    };
    lib_u8 program[CORE_MACHINE_CPU_DIAGNOSTIC_WINDOW_CAPACITY + 2u];
    core_machine *machine = LIB_NULL;
    core_machine_run_budget budget = {
        CORE_MACHINE_CPU_DIAGNOSTIC_WINDOW_CAPACITY + 1u, 0u
    };
    core_machine_run_result result;
    core_machine_cpu_diagnostic diagnostic;
    lib_size index;

    if (core_machine_create(&config, &machine) != LIB_STATUS_OK ||
        core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK ||
        core_machine_reset(machine) != LIB_STATUS_OK) goto fail;
    if (!test_core_machine_fixture_prepare_real_mode_execution(machine, 0u)) goto fail;
    for (index = 0u; index < CORE_MACHINE_CPU_DIAGNOSTIC_WINDOW_CAPACITY; ++index) {
        program[index] = 0x90u;
    }
    program[CORE_MACHINE_CPU_DIAGNOSTIC_WINDOW_CAPACITY] = 0xd6u;
    program[CORE_MACHINE_CPU_DIAGNOSTIC_WINDOW_CAPACITY + 1u] = 0x90u;
    if (core_machine_memory_write(machine, 0u, program, sizeof(program)) !=
        LIB_STATUS_OK) goto fail;
    if (!test_core_machine_fixture_preflight_real_ud_terminal(machine)) goto fail;
    if (
        core_machine_run(machine, budget, &result) != LIB_STATUS_INTERNAL_ERROR ||
        core_machine_get_cpu_diagnostic(machine, &diagnostic) != LIB_STATUS_OK) goto fail;
    /* Development instruction history is compiled out of the product; fault
     * snapshots remain the stable diagnostic contract in every build. */
    if (diagnostic.recent_count != 0u ||
        result.reason != CORE_MACHINE_STOP_FAULT ||
        result.detail != VCPUINS_EXCEPT_UD ||
        !diagnostic.first_fault.valid ||
        !CORE_MACHINE_BIT_IS_SET(diagnostic.first_fault.exception_mask,
            VCPUINS_EXCEPT_UD) ||
        diagnostic.first_fault.point.linear_pc !=
            CORE_MACHINE_CPU_DIAGNOSTIC_WINDOW_CAPACITY ||
        diagnostic.first_fault.point.bytes[0] != 0xd6u ||
        diagnostic.first_fault.point.bytes[1] != 0x90u) goto fail;
    core_machine_destroy(machine);
    printf("M5:T152:S1:CPU-FAULT-DIAGNOSTIC:OK\n");
    return 0;

fail:
    core_machine_destroy(machine);
    return 1;
}
