#include "support/cpu_board_limit_fixture.h"
#include <stdio.h>

static lib_i32 scan_test_read_failure(void)
{
    static const lib_u8 codes[][5] = {
        {0x0fu,0xbcu,0x0eu,0x10u,0u},
        {0x0fu,0xbdu,0x0eu,0x10u,0u}
    };
    const lib_u32 flags = VCPU_EFLAGS_CF | VCPU_EFLAGS_OF;
    lib_u8 index;

    for (index = 0u; index < 2u; ++index) {
        core_machine *machine = LIB_NULL;
        core_machine_run_result result = {0};
        core_machine_cpu_diagnostic diagnostic = {0};
        core_machine_debug_cpu_snapshot after = {0};
        core_machine_debug_register_patch patch = {0};
        lib_i32 failed = !test_cpu_board_limit_prepare(&machine,
            codes[index], sizeof(codes[index]), LIB_TRUE, LIB_TRUE);

        if (!failed) {
            patch.mask = CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_ECX) |
                CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EFLAGS);
            patch.values[CORE_MACHINE_DEBUG_ECX] = 0xaabbccddu;
            patch.values[CORE_MACHINE_DEBUG_EFLAGS] = flags;
            failed |= core_machine_debug_patch_registers(machine, &patch) !=
                    LIB_STATUS_OK ||
                core_machine_run(machine,
                    (core_machine_run_budget){ 1u, 0u }, &result) !=
                    LIB_STATUS_INTERNAL_ERROR ||
                result.reason != CORE_MACHINE_STOP_FAULT ||
                core_machine_get_cpu_diagnostic(machine, &diagnostic) !=
                    LIB_STATUS_OK ||
                core_machine_debug_capture_cpu_snapshot(machine,
                    CORE_MACHINE_CPU_SNAPSHOT_CURRENT, &after) != LIB_STATUS_OK ||
                !diagnostic.first_fault.valid ||
                !CORE_MACHINE_BIT_IS_SET(diagnostic.first_fault.exception_mask,
                    VCPUINS_EXCEPT_DF) ||
                after.ecx != 0xaabbccddu || after.eflags != flags ||
                after.eip != 0u;
        }
        core_machine_destroy(machine);
        if (failed) return 0;
    }
    return 1;
}

lib_i32 main(void)
{
    if (!scan_test_read_failure()) return 1;
    printf("M5:T310:S7:BIT-SCAN:OK\n");
    printf("M5:T401:S63:BIT-SCAN-PROFILES:OK\n");
    return 0;
}
