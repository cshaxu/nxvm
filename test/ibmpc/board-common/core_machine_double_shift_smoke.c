#include "lib/types/file.h"
#include "cpu_board_limit_fixture.h"
#include "x86/core/debug_interface.h"
static lib_i32 shift_test_access_failure(void)
{
    static const lib_u8 shld[] = {0x0fu, 0xa4u, 0x0eu, 0x10u, 0u, 1u};
    static const lib_u8 shrd[] = {0x0fu, 0xacu, 0x0eu, 0x10u, 0u, 1u};
    const lib_u32 flags = CORE_MACHINE_DEBUG_EFLAGS_CF | CORE_MACHINE_DEBUG_EFLAGS_ZF;
    lib_u8 pass;

    for (pass = 0u; pass < 2u; ++pass) {
        core_machine *machine = LIB_NULL;
        core_machine_run_result result = {0};
        core_machine_cpu_diagnostic diagnostic = {0};
        core_machine_debug_cpu_snapshot after = {0};
        core_machine_debug_register_patch patch = {0};
        lib_u16 before = 0x8123u, observed = 0u;
        lib_i32 failed = !test_cpu_board_limit_prepare(&machine,
            pass ? shrd : shld, sizeof(shld), pass == 0u, pass == 0u);

        if (!failed) {
            lib_status patch_status, write_status, run_status, diag_status;
            lib_status snapshot_status, read_status;
            patch.mask = CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_ECX) |
                CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EFLAGS);
            patch.values[CORE_MACHINE_DEBUG_ECX] = 0x7654u;
            patch.values[CORE_MACHINE_DEBUG_EFLAGS] = flags;
            patch_status = core_machine_debug_patch_registers(machine, &patch);
            write_status = core_machine_memory_write(machine, 0x3010u, &before,
                sizeof(before));
            run_status = core_machine_run(machine,
                (core_machine_run_budget){1u, 0u}, &result);
            diag_status = core_machine_get_cpu_diagnostic(machine, &diagnostic);
            snapshot_status = core_machine_debug_capture_cpu_snapshot(machine,
                CORE_MACHINE_CPU_SNAPSHOT_CURRENT, &after);
            read_status = core_machine_debug_read_real(machine, 0x0301u, 0u,
                &observed, sizeof(observed));
            failed |= patch_status != LIB_STATUS_OK ||
                write_status != LIB_STATUS_OK ||
                run_status != LIB_STATUS_INTERNAL_ERROR ||
                result.reason != CORE_MACHINE_STOP_FAULT ||
                diag_status != LIB_STATUS_OK || snapshot_status != LIB_STATUS_OK ||
                !diagnostic.first_fault.valid ||
                !CORE_MACHINE_BIT_IS_SET(diagnostic.first_fault.exception_mask,
                    VCPUINS_EXCEPT_DF) ||
                read_status != LIB_STATUS_OK || observed != before ||
                after.eflags != flags || after.eip != 0u;
        }
        core_machine_destroy(machine);
        if (failed) {
            lib_c_fprintf(lib_c_stderr, "shift board pass=%u fault=%u mask=%u eip=%u flags=%u memory=%u\n",
                (unsigned)pass, (unsigned)diagnostic.first_fault.valid,
                (unsigned)diagnostic.first_fault.exception_mask,
                (unsigned)after.eip, (unsigned)after.eflags,
                (unsigned)observed);
            return 0;
        }
    }
    return 1;
}

lib_i32 main(void)
{
    if (!shift_test_access_failure()) return 1;
    lib_c_printf("M5:T310:S6:DOUBLE-SHIFT:OK\n");
    lib_c_printf("M5:T401:S62:DOUBLE-SHIFT-PROFILES:OK\n");
    return 0;
}
