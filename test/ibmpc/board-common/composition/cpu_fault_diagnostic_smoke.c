#include "lib/types/types_interface.h"
#include <stdio.h>
#include "x86/core/device_support_interface.h"

#include "x86/core/debug_interface.h"
#include "ibmpc/board-common/machine_board_interface.h"

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
    const lib_u8 load_idt[] = { 0x0fu, 0x01u, 0x1eu, 0x00u, 0x03u };
    const lib_u8 idtr[] = { 0x17u, 0u, 0u, 0u, 0u, 0u };
    core_machine_debug_register_patch entry = {
        .mask = CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_CS) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_DS) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_ES) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_SS) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EIP),
        .values = { [CORE_MACHINE_DEBUG_EIP] = 0x0200u }
    };
    lib_size index;

    if (core_machine_create(&config, &machine, LIB_NULL) != LIB_STATUS_OK ||
        core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK ||
        core_machine_reset(machine) != LIB_STATUS_OK) goto fail;
    /* T337_REAL_UD_TERMINAL_GUEST_LIDT: exclude vector 6 using guest LIDT
       before the measured NOP/fault sequence, preserving its PC and budget. */
    if (core_machine_debug_patch_registers(machine, &entry) != LIB_STATUS_OK ||
        core_machine_memory_write(machine, 0x0200u, load_idt, sizeof(load_idt)) !=
            LIB_STATUS_OK ||
        core_machine_memory_write(machine, 0x0300u, idtr, sizeof(idtr)) !=
            LIB_STATUS_OK ||
        core_machine_run(machine, (core_machine_run_budget){1u, 0u}, &result) !=
            LIB_STATUS_OK || result.reason != CORE_MACHINE_STOP_BUDGET ||
        result.executed != 1u) goto fail;
    entry.values[CORE_MACHINE_DEBUG_EIP] = 0u;
    if (core_machine_debug_patch_registers(machine, &entry) != LIB_STATUS_OK) goto fail;
    for (index = 0u; index < CORE_MACHINE_CPU_DIAGNOSTIC_WINDOW_CAPACITY; ++index) {
        program[index] = 0x90u;
    }
    program[CORE_MACHINE_CPU_DIAGNOSTIC_WINDOW_CAPACITY] = 0xd6u;
    program[CORE_MACHINE_CPU_DIAGNOSTIC_WINDOW_CAPACITY + 1u] = 0x90u;
    if (core_machine_memory_write(machine, 0u, program, sizeof(program)) !=
        LIB_STATUS_OK) goto fail;
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
