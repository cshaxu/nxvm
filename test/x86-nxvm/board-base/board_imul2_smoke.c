#include "lib/types/file.h"
#include "cpu_board_limit_fixture.h"
static lib_i32 imul_read_failure(void)
{
    static const lib_u8 code[] = { 0x0fu,0xafu,0x0eu,0x10u,0u };
    const lib_u32 flags = X86_CORE_CPU_DEBUG_EFLAGS_CF | X86_CORE_CPU_DEBUG_EFLAGS_OF;
    x86_core_machine *machine = LIB_NULL;
    x86_core_machine_run_result result = {0};
    x86_core_cpu_debug_cpu_snapshot after = {0};
    x86_core_cpu_debug_register_patch patch = {0};
    lib_i32 failed = !test_cpu_board_limit_prepare(&machine, code,
        sizeof(code), LIB_TRUE, LIB_TRUE);

    if (!failed) {
        patch.mask = X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_ECX) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_EFLAGS);
        patch.values[X86_CORE_CPU_DEBUG_ECX] = 0xaabbccddu;
        patch.values[X86_CORE_CPU_DEBUG_EFLAGS] = flags;
        failed |= x86_core_machine_debug_patch_registers(machine, &patch) !=
                LIB_STATUS_OK ||
            !test_x86_nxvm_machine_fixture_shutdown_wait(x86_core_machine_run(machine,
                (x86_core_machine_run_budget){ 1u, 0u }, &result), &result) ||
            x86_core_machine_debug_capture_cpu_snapshot(machine,
                X86_CORE_CPU_SNAPSHOT_CURRENT, &after) != LIB_STATUS_OK ||
            after.ecx != 0xaabbccddu || after.eflags != flags ||
            after.eip != 0u;
    }
    x86_core_machine_destroy(machine);
    return !failed;
}

lib_i32 main(void)
{
    if (!imul_read_failure()) return 1;
    lib_c_printf("IMUL2:OK\n");
    lib_c_printf("IMUL2-PROFILES:OK\n");
    return 0;
}
