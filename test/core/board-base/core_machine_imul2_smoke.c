#include "lib/types/file.h"
#include "cpu_board_limit_fixture.h"
static lib_i32 imul_read_failure(void)
{
    static const lib_u8 code[] = { 0x0fu,0xafu,0x0eu,0x10u,0u };
    const lib_u32 flags = CORE_MACHINE_DEBUG_EFLAGS_CF | CORE_MACHINE_DEBUG_EFLAGS_OF;
    core_machine *machine = LIB_NULL;
    core_machine_run_result result = {0};
    core_machine_debug_cpu_snapshot after = {0};
    core_machine_debug_register_patch patch = {0};
    lib_i32 failed = !test_cpu_board_limit_prepare(&machine, code,
        sizeof(code), LIB_TRUE, LIB_TRUE);

    if (!failed) {
        patch.mask = CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_ECX) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EFLAGS);
        patch.values[CORE_MACHINE_DEBUG_ECX] = 0xaabbccddu;
        patch.values[CORE_MACHINE_DEBUG_EFLAGS] = flags;
        failed |= core_machine_debug_patch_registers(machine, &patch) !=
                LIB_STATUS_OK ||
            !test_core_machine_fixture_shutdown_wait(core_machine_run(machine,
                (core_machine_run_budget){ 1u, 0u }, &result), &result) ||
            core_machine_debug_capture_cpu_snapshot(machine,
                CORE_MACHINE_CPU_SNAPSHOT_CURRENT, &after) != LIB_STATUS_OK ||
            after.ecx != 0xaabbccddu || after.eflags != flags ||
            after.eip != 0u;
    }
    core_machine_destroy(machine);
    return !failed;
}

lib_i32 main(void)
{
    if (!imul_read_failure()) return 1;
    lib_c_printf("IMUL2:OK\n");
    lib_c_printf("IMUL2-PROFILES:OK\n");
    return 0;
}
