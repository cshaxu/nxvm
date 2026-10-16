#include "lib/types/file.h"
#include "cpu_board_limit_fixture.h"
#include "x86-core/machine/debug_interface.h"
static lib_i32 setcc_test_limit_nonpublication(void)
{
    static const lib_u8 limit_code[] = {0x67u, 0x0fu, 0x94u, 0x05u,
        0x10u, 0x00u, 0x00u, 0x00u};
    const lib_u32 flags = X86_CORE_CPU_DEBUG_EFLAGS_ZF | X86_CORE_CPU_DEBUG_EFLAGS_OF;
    const lib_u8 initial = 0xa5u;
    lib_u8 value = 0u;
    x86_core_machine *machine = LIB_NULL;
    x86_core_machine_run_result result = {0};
    x86_core_cpu_debug_cpu_snapshot after = {0};
    x86_core_cpu_debug_register_patch patch = {0};
    lib_i32 failed = !test_cpu_board_limit_prepare(&machine, limit_code,
        sizeof(limit_code), LIB_TRUE, LIB_TRUE);

    if (!failed) {
        patch.mask = X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_EAX) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_EFLAGS);
        patch.values[X86_CORE_CPU_DEBUG_EAX] = 0x99aabbccu;
        patch.values[X86_CORE_CPU_DEBUG_EFLAGS] = flags;
        failed |= x86_core_machine_debug_patch_registers(machine, &patch) !=
                LIB_STATUS_OK ||
            x86_core_machine_memory_write(machine, 0x3010u, &initial,
                sizeof(initial)) != LIB_STATUS_OK ||
            !test_x86_nxvm_machine_fixture_shutdown_wait(x86_core_machine_run(machine,
                (x86_core_machine_run_budget){1u, 0u}, &result), &result) ||
            x86_core_machine_debug_capture_cpu_snapshot(machine,
                X86_CORE_CPU_SNAPSHOT_CURRENT, &after) != LIB_STATUS_OK ||
            x86_core_machine_debug_read_real(machine, 0x0301u, 0u, &value,
                sizeof(value)) != LIB_STATUS_OK || value != initial ||
            after.eax != 0x99aabbccu || after.eflags != flags ||
            after.eip != 0u;
    }
    x86_core_machine_destroy(machine);
    return !failed;
}

lib_i32 main(void)
{
    if (!setcc_test_limit_nonpublication()) return 1;
    lib_c_printf("SETCC:OK\n");
    lib_c_printf("SETCC-PROFILES:OK\n");
    return 0;
}
