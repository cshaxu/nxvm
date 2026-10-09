#include "lib/types/file.h"
#include "cpu_board_limit_fixture.h"
#include "core/x86/debug_interface.h"
static lib_i32 setcc_test_limit_nonpublication(void)
{
    static const lib_u8 limit_code[] = {0x67u, 0x0fu, 0x94u, 0x05u,
        0x10u, 0x00u, 0x00u, 0x00u};
    const lib_u32 flags = CORE_MACHINE_DEBUG_EFLAGS_ZF | CORE_MACHINE_DEBUG_EFLAGS_OF;
    const lib_u8 initial = 0xa5u;
    lib_u8 value = 0u;
    core_machine *machine = LIB_NULL;
    core_machine_run_result result = {0};
    core_machine_debug_cpu_snapshot after = {0};
    core_machine_debug_register_patch patch = {0};
    lib_i32 failed = !test_cpu_board_limit_prepare(&machine, limit_code,
        sizeof(limit_code), LIB_TRUE, LIB_TRUE);

    if (!failed) {
        patch.mask = CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EAX) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EFLAGS);
        patch.values[CORE_MACHINE_DEBUG_EAX] = 0x99aabbccu;
        patch.values[CORE_MACHINE_DEBUG_EFLAGS] = flags;
        failed |= core_machine_debug_patch_registers(machine, &patch) !=
                LIB_STATUS_OK ||
            core_machine_memory_write(machine, 0x3010u, &initial,
                sizeof(initial)) != LIB_STATUS_OK ||
            !test_core_machine_fixture_shutdown_wait(core_machine_run(machine,
                (core_machine_run_budget){1u, 0u}, &result), &result) ||
            core_machine_debug_capture_cpu_snapshot(machine,
                CORE_MACHINE_CPU_SNAPSHOT_CURRENT, &after) != LIB_STATUS_OK ||
            core_machine_debug_read_real(machine, 0x0301u, 0u, &value,
                sizeof(value)) != LIB_STATUS_OK || value != initial ||
            after.eax != 0x99aabbccu || after.eflags != flags ||
            after.eip != 0u;
    }
    core_machine_destroy(machine);
    return !failed;
}

lib_i32 main(void)
{
    if (!setcc_test_limit_nonpublication()) return 1;
    lib_c_printf("SETCC:OK\n");
    lib_c_printf("SETCC-PROFILES:OK\n");
    return 0;
}
