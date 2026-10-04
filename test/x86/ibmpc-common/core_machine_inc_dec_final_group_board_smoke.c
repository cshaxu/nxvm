#include "lib/types/types_interface.h"
#include <stdio.h>
#include "cpu_board_fault_fixture.h"
#include "cpu_board_de_fixture.h"

static lib_i32 inc_dec_final_group_protected_faults(void)
{
    static const test_cpu_board_fault_case cases[] = {
        {{0x09u,0x16u,0x10u,0u},4u,LIB_FALSE,LIB_TRUE,0x8000u,
            0u,1u,CORE_MACHINE_DEBUG_EFLAGS_AF | CORE_MACHINE_DEBUG_EFLAGS_CF | CORE_MACHINE_DEBUG_EFLAGS_OF},
        {{0x09u,0x16u,0x10u,0u},4u,LIB_FALSE,LIB_FALSE,0x8000u,
            0u,1u,CORE_MACHINE_DEBUG_EFLAGS_AF | CORE_MACHINE_DEBUG_EFLAGS_CF | CORE_MACHINE_DEBUG_EFLAGS_OF},
        {{0x21u,0x16u,0x10u,0u},4u,LIB_FALSE,LIB_TRUE,0x8081u,
            0u,0xffffu,CORE_MACHINE_DEBUG_EFLAGS_AF | CORE_MACHINE_DEBUG_EFLAGS_CF | CORE_MACHINE_DEBUG_EFLAGS_OF},
        {{0x21u,0x16u,0x10u,0u},4u,LIB_FALSE,LIB_FALSE,0x8081u,
            0u,0xffffu,CORE_MACHINE_DEBUG_EFLAGS_AF | CORE_MACHINE_DEBUG_EFLAGS_CF | CORE_MACHINE_DEBUG_EFLAGS_OF},
        {{0x29u,0x16u,0x10u,0u},4u,LIB_FALSE,LIB_TRUE,0u,
            0u,1u,CORE_MACHINE_DEBUG_EFLAGS_CF},
        {{0x29u,0x16u,0x10u,0u},4u,LIB_FALSE,LIB_FALSE,0u,
            0u,1u,CORE_MACHINE_DEBUG_EFLAGS_CF},
        {{0x31u,0x16u,0x10u,0u},4u,LIB_FALSE,LIB_TRUE,0u,
            0u,0xffffu,CORE_MACHINE_DEBUG_EFLAGS_AF | CORE_MACHINE_DEBUG_EFLAGS_CF | CORE_MACHINE_DEBUG_EFLAGS_OF},
        {{0x31u,0x16u,0x10u,0u},4u,LIB_FALSE,LIB_FALSE,0u,
            0u,0xffffu,CORE_MACHINE_DEBUG_EFLAGS_AF | CORE_MACHINE_DEBUG_EFLAGS_CF | CORE_MACHINE_DEBUG_EFLAGS_OF},
        {{0x39u,0x16u,0x10u,0u},4u,LIB_FALSE,LIB_TRUE,0u,
            0u,1u,CORE_MACHINE_DEBUG_EFLAGS_CF},
        {{0x3bu,0x16u,0x10u,0u},4u,LIB_FALSE,LIB_TRUE,0u,
            0u,1u,CORE_MACHINE_DEBUG_EFLAGS_CF}
    };

    return test_cpu_board_faults(cases,
        sizeof(cases) / sizeof(cases[0]));
}

static lib_i32 inc_dec_final_group_aam_zero(void)
{
    const test_cpu_board_de_case entry = {
        .code = {0xd4u,0u}, .bytes = 2u, .eax = 0x1122332fu,
        .flags = CORE_MACHINE_DEBUG_EFLAGS_CF | CORE_MACHINE_DEBUG_EFLAGS_OF
    };
    return test_cpu_board_de_delivery(&entry);
}

static lib_i32 inc_dec_final_group_xlat_fault(void)
{
    static const lib_u8 code[] = {0xd7u};
    core_machine *machine = LIB_NULL;
    core_machine_debug_register_patch patch = {0};
    core_machine_debug_cpu_snapshot after = {0};
    core_machine_cpu_diagnostic diagnostic = {0};
    core_machine_run_result result = {0};
    lib_i32 failed = !test_cpu_board_limit_prepare(&machine, code,
        sizeof(code), LIB_FALSE, LIB_TRUE);

    if (!failed) {
        patch.mask = CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EAX) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EBX) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EFLAGS);
        patch.values[CORE_MACHINE_DEBUG_EAX] = 0x11223304u;
        patch.values[CORE_MACHINE_DEBUG_EBX] = 0x10u;
        patch.values[CORE_MACHINE_DEBUG_EFLAGS] = CORE_MACHINE_DEBUG_EFLAGS_CF;
        failed = core_machine_debug_patch_registers(machine, &patch) !=
                LIB_STATUS_OK ||
            core_machine_run(machine, (core_machine_run_budget){1u,0u},
                &result) != LIB_STATUS_INTERNAL_ERROR ||
            result.reason != CORE_MACHINE_STOP_FAULT ||
            core_machine_get_cpu_diagnostic(machine, &diagnostic) !=
                LIB_STATUS_OK ||
            core_machine_debug_capture_cpu_snapshot(machine,
                CORE_MACHINE_CPU_SNAPSHOT_CURRENT, &after) != LIB_STATUS_OK ||
            !diagnostic.first_fault.valid ||
            !(diagnostic.first_fault.exception_mask & VCPUINS_EXCEPT_DF) ||
            after.eax != 0x11223304u || after.ebx != 0x10u ||
            after.eflags != CORE_MACHINE_DEBUG_EFLAGS_CF || after.eip != 0u;
    }
    core_machine_destroy(machine);
    return !failed;
}

int main(void)
{
    if (!inc_dec_final_group_protected_faults() ||
        !inc_dec_final_group_aam_zero() ||
        !inc_dec_final_group_xlat_fault()) {
        fputs("M5:T539:S35:BOARD-LOGICAL-DECIMAL-XLAT:FAIL\n", stderr);
        return 1;
    }
    puts("M5:T539:S35:BOARD-LOGICAL-DECIMAL-XLAT:OK");
    return 0;
}
