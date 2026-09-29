#include "support/cpu_board_limit_fixture.h"
#include "app-nxvm/devices/debug_interface.h"
#include <stdio.h>

static lib_i32 bit_test_access_failure(void)
{
    static const lib_u8 read_code[] = {0x0fu, 0xa3u, 0x0eu, 0x10u, 0u};
    static const lib_u8 write_code[] = {0x0fu, 0xabu, 0x0eu, 0x10u, 0u};
    const lib_u32 flags = VCPU_EFLAGS_ZF;
    lib_u8 pass;

    for (pass = 0u; pass < 2u; ++pass) {
        core_machine *machine = LIB_NULL;
        core_machine_run_result result = {0};
        core_machine_cpu_diagnostic diagnostic = {0};
        core_machine_debug_cpu_snapshot after = {0};
        core_machine_debug_register_patch patch = {0};
        lib_u16 before = 2u, observed = 0u;
        lib_i32 failed = !test_cpu_board_limit_prepare(&machine,
            pass ? write_code : read_code, sizeof(read_code),
            pass == 0u, pass == 0u);

        if (!failed) {
            patch.mask = CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_ECX) |
                CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EFLAGS);
            patch.values[CORE_MACHINE_DEBUG_ECX] = 1u;
            patch.values[CORE_MACHINE_DEBUG_EFLAGS] = flags;
            failed |= core_machine_debug_patch_registers(machine, &patch) !=
                    LIB_STATUS_OK ||
                core_machine_memory_write(machine, 0x3010u, &before,
                    sizeof(before)) != LIB_STATUS_OK ||
                core_machine_run(machine, (core_machine_run_budget){1u, 0u},
                    &result) != LIB_STATUS_INTERNAL_ERROR ||
                result.reason != CORE_MACHINE_STOP_FAULT ||
                core_machine_get_cpu_diagnostic(machine, &diagnostic) !=
                    LIB_STATUS_OK ||
                core_machine_debug_capture_cpu_snapshot(machine,
                    CORE_MACHINE_CPU_SNAPSHOT_CURRENT, &after) != LIB_STATUS_OK ||
                !diagnostic.first_fault.valid ||
                !CORE_MACHINE_BIT_IS_SET(diagnostic.first_fault.exception_mask,
                    VCPUINS_EXCEPT_DF) ||
                core_machine_debug_read_real(machine, 0x0301u, 0u, &observed,
                    sizeof(observed)) != LIB_STATUS_OK || observed != before ||
                after.ecx != 1u || after.eflags != flags || after.eip != 0u;
        }
        core_machine_destroy(machine);
        if (failed) return 0;
    }
    return 1;
}

lib_i32 main(void)
{
    if (!bit_test_access_failure()) return 1;
    printf("M5:T310:S5:BIT:OK\n");
    printf("M5:T401:S61:BIT-TEST-PROFILES:OK\n");
    return 0;
}
