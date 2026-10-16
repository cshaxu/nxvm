#include "lib/types/file.h"
#include "lib/types/types_interface.h"

#include "x86-core/machine/debug_interface.h"
#include "x86-nxvm/board-base/machine_board_interface.h"

static lib_bool diagnostic_run_case(lib_bool shutdown)
{
    const x86_nxvm_board_config config = {
        .memory_bytes = (16u * 1024u * 1024u)
    };
    lib_u8 program[X86_CORE_CPU_DIAGNOSTIC_WINDOW_CAPACITY + 2u];
    x86_core_machine *machine = LIB_NULL;
    x86_core_machine_run_budget budget = {
        X86_CORE_CPU_DIAGNOSTIC_WINDOW_CAPACITY + 1u, 0u
    };
    x86_core_machine_run_result result;
    x86_core_cpu_diagnostic diagnostic;
    const lib_u8 load_idt[] = { 0x0fu, 0x01u, 0x1eu, 0x00u, 0x03u };
    const lib_u8 idtr[] = {
        shutdown ? 0x17u : 0xffu, shutdown ? 0u : 3u, 0u, 0u, 0u, 0u
    };
    const lib_u16 ud_vector[] = {0x0400u, 0u};
    const lib_u8 handler = 0xf4u;
    x86_core_cpu_debug_register_patch entry = {
        .mask = X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_CS) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_DS) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_ES) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_SS) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_EIP),
        .values = { [X86_CORE_CPU_DEBUG_EIP] = 0x0200u }
    };
    lib_size index;

    if (x86_nxvm_board_create(&config, &machine, LIB_NULL) != LIB_STATUS_OK ||
        x86_core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK ||
        x86_core_machine_reset(machine) != LIB_STATUS_OK) goto fail;
    /* REAL_UD_TERMINAL_GUEST_LIDT: compare a real UD handler with
     * unavailable UD/DF entries, preserving the same source PC and budget. */
    if (x86_core_machine_debug_patch_registers(machine, &entry) != LIB_STATUS_OK ||
        x86_core_machine_memory_write(machine, 0x0200u, load_idt, sizeof(load_idt)) !=
            LIB_STATUS_OK ||
        x86_core_machine_memory_write(machine, 0x0300u, idtr, sizeof(idtr)) !=
            LIB_STATUS_OK ||
        x86_core_machine_run(machine, (x86_core_machine_run_budget){1u, 0u}, &result) !=
            LIB_STATUS_OK || result.reason != X86_CORE_MACHINE_STOP_BUDGET ||
        result.executed != 1u) goto fail;
    entry.values[X86_CORE_CPU_DEBUG_EIP] = 0u;
    if (x86_core_machine_debug_patch_registers(machine, &entry) != LIB_STATUS_OK) goto fail;
    for (index = 0u; index < X86_CORE_CPU_DIAGNOSTIC_WINDOW_CAPACITY; ++index) {
        program[index] = 0x90u;
    }
    program[X86_CORE_CPU_DIAGNOSTIC_WINDOW_CAPACITY] = 0xd6u;
    program[X86_CORE_CPU_DIAGNOSTIC_WINDOW_CAPACITY + 1u] = 0x90u;
    if (x86_core_machine_memory_write(machine, 6u * 4u, ud_vector,
            sizeof(ud_vector)) != LIB_STATUS_OK ||
        x86_core_machine_memory_write(machine, 0x0400u, &handler,
            sizeof(handler)) != LIB_STATUS_OK ||
        x86_core_machine_memory_write(machine, 0u, program, sizeof(program)) !=
        LIB_STATUS_OK) goto fail;
    if (
        x86_core_machine_run(machine, budget, &result) != LIB_STATUS_OK ||
        x86_core_machine_get_cpu_diagnostic(machine, &diagnostic) != LIB_STATUS_OK) goto fail;
    /* Development instruction history is compiled out of the product; fault
     * snapshots remain the stable diagnostic contract in every build. */
    if (diagnostic.recent_count != 0u ||
        result.reason != (shutdown ? X86_CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT :
            X86_CORE_MACHINE_STOP_BUDGET) ||
        result.detail != (shutdown ? VCPUINS_EXCEPT_SHUTDOWN : 0u) ||
        result.executed != X86_CORE_CPU_DIAGNOSTIC_WINDOW_CAPACITY ||
        diagnostic.first_fault.valid || !diagnostic.last_delivered_exception.valid ||
        diagnostic.last_delivered_exception.exception_mask !=
            (shutdown ? VCPUINS_EXCEPT_SHUTDOWN : VCPUINS_EXCEPT_UD) ||
        diagnostic.last_delivered_exception.point.linear_pc !=
            X86_CORE_CPU_DIAGNOSTIC_WINDOW_CAPACITY ||
        diagnostic.last_delivered_exception.point.bytes[0] != 0xd6u ||
        diagnostic.last_delivered_exception.point.bytes[1] != 0x90u) goto fail;
    x86_core_machine_destroy(machine);
    return LIB_TRUE;

fail:
    x86_core_machine_destroy(machine);
    return LIB_FALSE;
}

lib_i32 main(void)
{
    if (!diagnostic_run_case(LIB_FALSE) || !diagnostic_run_case(LIB_TRUE)) return 1;
    lib_c_printf("CPU-FAULT-DIAGNOSTIC:OK\n");
    return 0;
}
