#ifndef TEST_CPU_BOARD_FLAGS_PROTECTED_FIXTURE_H
#define TEST_CPU_BOARD_FLAGS_PROTECTED_FIXTURE_H

#include "lib/types/types_interface.h"
#include "cpu_board_limit_fixture.h"

typedef struct test_cpu_board_flags_protected_result {
    x86_core_cpu_debug_cpu_snapshot before;
    x86_core_cpu_debug_cpu_snapshot after;
    x86_core_cpu_diagnostic diagnostic;
} test_cpu_board_flags_protected_result;

static lib_i32 test_cpu_board_flags_protected_run(const lib_u8 *code,
    lib_size bytes, lib_u32 eax, lib_u32 flags,
    test_cpu_board_flags_protected_result *out)
{
    core_machine *machine = LIB_NULL;
    x86_core_cpu_debug_register_patch patch = {0};
    core_machine_run_result result = {0};
    lib_i32 failed = out == LIB_NULL ||
        !test_cpu_board_limit_prepare(&machine, code, bytes, LIB_TRUE,
            LIB_FALSE);

    if (!failed) {
        lib_memory_set(out, 0, sizeof(*out));
        patch.mask = X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_EAX) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_ECX) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_EDX) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_EBX) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_ESP) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_EBP) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_ESI) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_EDI) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_EFLAGS);
        patch.values[X86_CORE_CPU_DEBUG_EAX] = eax;
        patch.values[X86_CORE_CPU_DEBUG_ECX] = 0x11223344u;
        patch.values[X86_CORE_CPU_DEBUG_EDX] = 0x55667788u;
        patch.values[X86_CORE_CPU_DEBUG_EBX] = 0x99aabbccU;
        patch.values[X86_CORE_CPU_DEBUG_ESP] = 0x8000u;
        patch.values[X86_CORE_CPU_DEBUG_EBP] = 0x120u;
        patch.values[X86_CORE_CPU_DEBUG_ESI] = 0x10u;
        patch.values[X86_CORE_CPU_DEBUG_EDI] = 0x20u;
        patch.values[X86_CORE_CPU_DEBUG_EFLAGS] = flags;
        failed = core_machine_debug_patch_registers(machine, &patch) !=
                LIB_STATUS_OK ||
            core_machine_debug_capture_cpu_snapshot(machine,
                X86_CORE_CPU_SNAPSHOT_CURRENT, &out->before) !=
                LIB_STATUS_OK ||
            core_machine_run(machine, (core_machine_run_budget){1u,0u},
                &result) != LIB_STATUS_OK ||
            result.reason != CORE_MACHINE_STOP_BUDGET ||
            core_machine_get_cpu_diagnostic(machine, &out->diagnostic) !=
                LIB_STATUS_OK ||
            core_machine_debug_capture_cpu_snapshot(machine,
                X86_CORE_CPU_SNAPSHOT_CURRENT, &out->after) !=
                LIB_STATUS_OK;
    }
    core_machine_destroy(machine);
    return !failed;
}

#endif
