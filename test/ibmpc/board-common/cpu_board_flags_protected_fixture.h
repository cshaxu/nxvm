#ifndef TEST_CPU_BOARD_FLAGS_PROTECTED_FIXTURE_H
#define TEST_CPU_BOARD_FLAGS_PROTECTED_FIXTURE_H

#include "lib/types/types_interface.h"
#include "cpu_board_limit_fixture.h"

typedef struct test_cpu_board_flags_protected_result {
    core_machine_debug_cpu_snapshot before;
    core_machine_debug_cpu_snapshot after;
    core_machine_cpu_diagnostic diagnostic;
} test_cpu_board_flags_protected_result;

static lib_i32 test_cpu_board_flags_protected_run(const lib_u8 *code,
    lib_size bytes, lib_u32 eax, lib_u32 flags,
    test_cpu_board_flags_protected_result *out)
{
    core_machine *machine = LIB_NULL;
    core_machine_debug_register_patch patch = {0};
    core_machine_run_result result = {0};
    lib_i32 failed = out == LIB_NULL ||
        !test_cpu_board_limit_prepare(&machine, code, bytes, LIB_TRUE,
            LIB_FALSE);

    if (!failed) {
        lib_memory_set(out, 0, sizeof(*out));
        patch.mask = CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EAX) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_ECX) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EDX) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EBX) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_ESP) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EBP) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_ESI) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EDI) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EFLAGS);
        patch.values[CORE_MACHINE_DEBUG_EAX] = eax;
        patch.values[CORE_MACHINE_DEBUG_ECX] = 0x11223344u;
        patch.values[CORE_MACHINE_DEBUG_EDX] = 0x55667788u;
        patch.values[CORE_MACHINE_DEBUG_EBX] = 0x99aabbccU;
        patch.values[CORE_MACHINE_DEBUG_ESP] = 0x8000u;
        patch.values[CORE_MACHINE_DEBUG_EBP] = 0x120u;
        patch.values[CORE_MACHINE_DEBUG_ESI] = 0x10u;
        patch.values[CORE_MACHINE_DEBUG_EDI] = 0x20u;
        patch.values[CORE_MACHINE_DEBUG_EFLAGS] = flags;
        failed = core_machine_debug_patch_registers(machine, &patch) !=
                LIB_STATUS_OK ||
            core_machine_debug_capture_cpu_snapshot(machine,
                CORE_MACHINE_CPU_SNAPSHOT_CURRENT, &out->before) !=
                LIB_STATUS_OK ||
            core_machine_run(machine, (core_machine_run_budget){1u,0u},
                &result) != LIB_STATUS_OK ||
            result.reason != CORE_MACHINE_STOP_BUDGET ||
            core_machine_get_cpu_diagnostic(machine, &out->diagnostic) !=
                LIB_STATUS_OK ||
            core_machine_debug_capture_cpu_snapshot(machine,
                CORE_MACHINE_CPU_SNAPSHOT_CURRENT, &out->after) !=
                LIB_STATUS_OK;
    }
    core_machine_destroy(machine);
    return !failed;
}

#endif
