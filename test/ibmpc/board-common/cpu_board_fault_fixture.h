#ifndef TEST_CPU_BOARD_FAULT_FIXTURE_H
#define TEST_CPU_BOARD_FAULT_FIXTURE_H

#include "lib/types/types_interface.h"
#include "x86/core/debug_interface.h"
#include "x86/core/machine_interface.h"
#include "cpu_board_limit_fixture.h"

typedef struct test_cpu_board_fault_case {
    lib_u8 code[6];
    lib_u8 bytes;
    lib_bool writable;
    lib_bool out_of_limit;
    lib_u16 memory;
    lib_u32 eax;
    lib_u32 edx;
    lib_u32 flags;
} test_cpu_board_fault_case;

static lib_i32 test_cpu_board_faults(const test_cpu_board_fault_case *cases,
    lib_size count)
{
    for (lib_size index = 0u; index < count; ++index) {
        const test_cpu_board_fault_case *entry = &cases[index];
        core_machine *machine = LIB_NULL;
        core_machine_debug_cpu_snapshot after = {0};
        core_machine_run_result result = {0};
        core_machine_debug_register_patch patch = {0};
        lib_u16 observed = 0u;
        lib_i32 failed = !test_cpu_board_limit_prepare(&machine, entry->code,
            entry->bytes, entry->writable, entry->out_of_limit);

        if (!failed) {
            patch.mask = CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EAX) |
                CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EDX) |
                CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EFLAGS);
            patch.values[CORE_MACHINE_DEBUG_EAX] = entry->eax;
            patch.values[CORE_MACHINE_DEBUG_EDX] = entry->edx;
            patch.values[CORE_MACHINE_DEBUG_EFLAGS] = entry->flags;
            failed = core_machine_debug_patch_registers(machine, &patch) !=
                    LIB_STATUS_OK ||
                core_machine_memory_write(machine, 0x3010u, &entry->memory,
                    sizeof(entry->memory)) != LIB_STATUS_OK ||
                !test_core_machine_fixture_shutdown_wait(core_machine_run(machine,
                    (core_machine_run_budget){1u,0u}, &result), &result) ||
                core_machine_debug_capture_cpu_snapshot(machine,
                    CORE_MACHINE_CPU_SNAPSHOT_CURRENT, &after) != LIB_STATUS_OK ||
                core_machine_debug_read_real(machine, 0x0301u, 0u,
                    &observed, sizeof(observed)) != LIB_STATUS_OK ||
                observed != entry->memory || after.eax != entry->eax ||
                after.edx != entry->edx || after.eflags != entry->flags ||
                after.eip != 0u;
        }
        core_machine_destroy(machine);
        if (failed) return 0;
    }
    return 1;
}

#endif
