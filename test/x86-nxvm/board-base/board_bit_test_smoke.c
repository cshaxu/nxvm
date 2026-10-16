#include "lib/types/file.h"
#include "cpu_board_limit_fixture.h"
#include "x86-core/machine/debug_interface.h"
static lib_i32 bit_test_access_failure(void)
{
    static const lib_u8 read_code[] = {0x0fu, 0xa3u, 0x0eu, 0x10u, 0u};
    static const lib_u8 write_code[] = {0x0fu, 0xabu, 0x0eu, 0x10u, 0u};
    const lib_u32 flags = X86_CORE_CPU_DEBUG_EFLAGS_ZF;
    lib_u8 pass;

    for (pass = 0u; pass < 2u; ++pass) {
        x86_core_machine *machine = LIB_NULL;
        x86_core_machine_run_result result = {0};
        x86_core_cpu_debug_cpu_snapshot after = {0};
        x86_core_cpu_debug_register_patch patch = {0};
        lib_u16 before = 2u, observed = 0u;
        lib_i32 failed = !test_cpu_board_limit_prepare(&machine,
            pass ? write_code : read_code, sizeof(read_code),
            pass == 0u, pass == 0u);

        if (!failed) {
            patch.mask = X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_ECX) |
                X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_EFLAGS);
            patch.values[X86_CORE_CPU_DEBUG_ECX] = 1u;
            patch.values[X86_CORE_CPU_DEBUG_EFLAGS] = flags;
            failed |= x86_core_machine_debug_patch_registers(machine, &patch) !=
                    LIB_STATUS_OK ||
                x86_core_machine_memory_write(machine, 0x3010u, &before,
                    sizeof(before)) != LIB_STATUS_OK ||
                !test_x86_nxvm_machine_fixture_shutdown_wait(x86_core_machine_run(machine,
                    (x86_core_machine_run_budget){1u, 0u}, &result), &result) ||
                x86_core_machine_debug_capture_cpu_snapshot(machine,
                    X86_CORE_CPU_SNAPSHOT_CURRENT, &after) != LIB_STATUS_OK ||
                x86_core_machine_debug_read_real(machine, 0x0301u, 0u, &observed,
                    sizeof(observed)) != LIB_STATUS_OK || observed != before ||
                after.ecx != 1u || after.eflags != flags || after.eip != 0u;
        }
        x86_core_machine_destroy(machine);
        if (failed) return 0;
    }
    return 1;
}

lib_i32 main(void)
{
    if (!bit_test_access_failure()) return 1;
    lib_c_printf("BIT:OK\n");
    lib_c_printf("BIT-TEST-PROFILES:OK\n");
    return 0;
}
