#include "type.h"

#include "core/machine/cpu.h"
#include "core/machine/debug_interface.h"
#include "core/machine/machine_interface.h"
#include "core/machine/memory.h"
#include "../support/core_machine_cpu_fixture.h"

C_INT main(C_VOID)
{
    const core_machine_config config = {
        .memory_bytes = CORE_MACHINE_DEFAULT_MEMORY_BYTES
    };
    C_UCHAR program[CORE_MACHINE_CPU_DIAGNOSTIC_WINDOW_CAPACITY + 2u];
    core_machine *machine = STD_NULL;
    core_machine_run_budget budget = {
        CORE_MACHINE_CPU_DIAGNOSTIC_WINDOW_CAPACITY + 1u, 0u
    };
    core_machine_run_result result;
    core_machine_cpu_diagnostic diagnostic;
    const core_machine_cpu_execution_point *last;
    STD_SIZE_T index;

    if (core_machine_create(&config, &machine) != TYPE_STATUS_OK ||
        core_machine_freeze_execution_providers(machine) != TYPE_STATUS_OK ||
        core_machine_reset(machine) != TYPE_STATUS_OK) goto fail;
    if (!test_core_machine_fixture_prepare_real_mode_execution(machine, 0u)) goto fail;
    for (index = 0u; index < CORE_MACHINE_CPU_DIAGNOSTIC_WINDOW_CAPACITY; ++index) {
        program[index] = 0x90u;
    }
    program[CORE_MACHINE_CPU_DIAGNOSTIC_WINDOW_CAPACITY] = 0xd6u;
    program[CORE_MACHINE_CPU_DIAGNOSTIC_WINDOW_CAPACITY + 1u] = 0x90u;
    if (core_machine_memory_write(machine, 0u, program, sizeof(program)) !=
        TYPE_STATUS_OK) goto fail;
    if (!test_core_machine_fixture_preflight_real_ud_terminal(machine)) goto fail;
    if (
        core_machine_run(machine, budget, &result) != TYPE_STATUS_FAULT ||
        core_machine_get_cpu_diagnostic(machine, &diagnostic) != TYPE_STATUS_OK) goto fail;
    last = &diagnostic.recent[diagnostic.recent_count - 1u];
    if (diagnostic.recent_count != CORE_MACHINE_CPU_DIAGNOSTIC_WINDOW_CAPACITY ||
        result.reason != CORE_MACHINE_STOP_FAULT ||
        result.detail != VCPUINS_EXCEPT_UD ||
        !diagnostic.first_fault.valid ||
        !TYPE_GET_BIT(diagnostic.first_fault.exception_mask,
            VCPUINS_EXCEPT_UD) ||
        diagnostic.first_fault.point.linear_pc !=
            CORE_MACHINE_CPU_DIAGNOSTIC_WINDOW_CAPACITY ||
        diagnostic.first_fault.point.bytes[0] != 0xd6u ||
        diagnostic.first_fault.point.bytes[1] != 0x90u ||
        last->linear_pc != CORE_MACHINE_CPU_DIAGNOSTIC_WINDOW_CAPACITY ||
        last->bytes[0] != 0xd6u || last->bytes[1] != 0x90u) goto fail;
    core_machine_destroy(machine);
    STD_PRINTF("M5:T152:S1:CPU-FAULT-DIAGNOSTIC:OK\n");
    return 0;

fail:
    core_machine_destroy(machine);
    return 1;
}
