#include "lib/types/file.h"
#include "cpu_board_limit_fixture.h"
#include "x86/core/debug_interface.h"
static lib_i32 rotate_access_failure(lib_bool shift)
{
    const lib_u32 flags = shift ?
        CORE_MACHINE_DEBUG_EFLAGS_CF | CORE_MACHINE_DEBUG_EFLAGS_OF | CORE_MACHINE_DEBUG_EFLAGS_AF :
        CORE_MACHINE_DEBUG_EFLAGS_CF | CORE_MACHINE_DEBUG_EFLAGS_ZF;
    lib_u8 group, pass;

    for (group = 0u; group < (shift ? 3u : 1u); ++group)
    for (pass = 0u; pass < 2u; ++pass) {
        const lib_u8 code[] = {
            0xc1u,
            shift ? (lib_u8)(((group == 2u ? 7u : group + 4u) << 3u) |
                0x06u) : 0x06u,
            0x10u, 0u, 1u
        };
        core_machine *machine = LIB_NULL;
        core_machine_run_result result = {0};
        core_machine_debug_cpu_snapshot after = {0};
        core_machine_debug_register_patch patch = {0};
        lib_u16 before = 0x8123u, observed = 0u;
        lib_i32 failed = !test_cpu_board_limit_prepare(&machine, code,
            sizeof(code), pass == 0u, pass == 0u);

        if (!failed) {
            patch.mask = CORE_MACHINE_DEBUG_REGISTER_MASK(
                CORE_MACHINE_DEBUG_EFLAGS);
            patch.values[CORE_MACHINE_DEBUG_EFLAGS] = flags;
            failed = core_machine_debug_patch_registers(machine, &patch) !=
                    LIB_STATUS_OK ||
                core_machine_memory_write(machine, 0x3010u, &before,
                    sizeof(before)) != LIB_STATUS_OK ||
                !test_core_machine_fixture_shutdown_wait(core_machine_run(machine,
                    (core_machine_run_budget){1u, 0u}, &result), &result) ||
                core_machine_debug_capture_cpu_snapshot(machine,
                    CORE_MACHINE_CPU_SNAPSHOT_CURRENT, &after) !=
                    LIB_STATUS_OK ||
                core_machine_debug_read_real(machine, 0x0301u, 0u, &observed,
                    sizeof(observed)) != LIB_STATUS_OK || observed != before ||
                after.eflags != flags || after.eip != 0u;
        }
        core_machine_destroy(machine);
        if (failed) return 0;
    }
    return 1;
}

lib_i32 main(void)
{
    if (!rotate_access_failure(LIB_FALSE) ||
        !rotate_access_failure(LIB_TRUE)) return 1;
    lib_c_printf("ROTATE-BOARD:OK\n");
    lib_c_printf("SHIFT-BOARD:OK\n");
    return 0;
}
