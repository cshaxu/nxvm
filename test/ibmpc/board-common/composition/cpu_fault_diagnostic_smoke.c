#include "lib/types/file.h"
#include "lib/types/types_interface.h"
#include "x86/core/device_support_interface.h"

#include "x86/core/debug_interface.h"
#include "ibmpc/board-common/machine_board_interface.h"

static lib_bool diagnostic_run_case(lib_bool shutdown)
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
    const lib_u8 idtr[] = {
        shutdown ? 0x17u : 0xffu, shutdown ? 0u : 3u, 0u, 0u, 0u, 0u
    };
    const lib_u16 ud_vector[] = {0x0400u, 0u};
    const lib_u8 handler = 0xf4u;
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
    /* REAL_UD_TERMINAL_GUEST_LIDT: compare a real UD handler with
     * unavailable UD/DF entries, preserving the same source PC and budget. */
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
    if (core_machine_memory_write(machine, 6u * 4u, ud_vector,
            sizeof(ud_vector)) != LIB_STATUS_OK ||
        core_machine_memory_write(machine, 0x0400u, &handler,
            sizeof(handler)) != LIB_STATUS_OK ||
        core_machine_memory_write(machine, 0u, program, sizeof(program)) !=
        LIB_STATUS_OK) goto fail;
    if (
        core_machine_run(machine, budget, &result) != LIB_STATUS_OK ||
        core_machine_get_cpu_diagnostic(machine, &diagnostic) != LIB_STATUS_OK) goto fail;
    /* Development instruction history is compiled out of the product; fault
     * snapshots remain the stable diagnostic contract in every build. */
    if (diagnostic.recent_count != 0u ||
        result.reason != (shutdown ? CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT :
            CORE_MACHINE_STOP_BUDGET) ||
        result.detail != (shutdown ? VCPUINS_EXCEPT_SHUTDOWN : 0u) ||
        result.executed != CORE_MACHINE_CPU_DIAGNOSTIC_WINDOW_CAPACITY ||
        diagnostic.first_fault.valid || !diagnostic.last_delivered_exception.valid ||
        diagnostic.last_delivered_exception.exception_mask !=
            (shutdown ? VCPUINS_EXCEPT_SHUTDOWN : VCPUINS_EXCEPT_UD) ||
        diagnostic.last_delivered_exception.point.linear_pc !=
            CORE_MACHINE_CPU_DIAGNOSTIC_WINDOW_CAPACITY ||
        diagnostic.last_delivered_exception.point.bytes[0] != 0xd6u ||
        diagnostic.last_delivered_exception.point.bytes[1] != 0x90u) goto fail;
    core_machine_destroy(machine);
    return LIB_TRUE;

fail:
    core_machine_destroy(machine);
    return LIB_FALSE;
}

lib_i32 main(void)
{
    if (!diagnostic_run_case(LIB_FALSE) || !diagnostic_run_case(LIB_TRUE)) return 1;
    lib_c_printf("CPU-FAULT-DIAGNOSTIC:OK\n");
    return 0;
}
