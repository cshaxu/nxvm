#ifndef TEST_CPU_BOARD_DE_FIXTURE_H
#define TEST_CPU_BOARD_DE_FIXTURE_H

#include "lib/types/types_interface.h"
#include "app-nxvm/devices/cpu.h"
#include "app-nxvm/devices/debug_interface.h"
#include "app-nxvm/devices/machine_interface.h"

typedef struct test_cpu_board_de_case {
    lib_u8 code[3];
    lib_u8 bytes;
    lib_u32 eax;
    lib_u32 edx;
    lib_u32 ecx;
    lib_u32 flags;
} test_cpu_board_de_case;

static lib_i32 test_cpu_board_de_delivery(const test_cpu_board_de_case *entry)
{
    static const lib_u8 handler[] = {0xf4u};
    const lib_u16 handler_offset = 0x0100u;
    const lib_u16 code_offset = 0x0200u;
    const lib_u16 handler_segment = 0u;
    const lib_u8 frame_width = entry->code[0] == 0x66u ? 4u : 2u;
    const core_machine_config config = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
        .cpu_profile = CORE_MACHINE_CPU_PROFILE_80386,
        .fpu_profile = X86_FPU_PROFILE_NONE
    };
    core_machine *machine = LIB_NULL;
    core_machine_debug_register_patch patch = {0};
    core_machine_debug_cpu_snapshot before = {0}, after = {0};
    core_machine_cpu_diagnostic diagnostic = {0};
    core_machine_run_result result = {0};
    lib_u16 frame16[3] = {0u};
    lib_u32 frame32[3] = {0u};
    lib_i32 failed = core_machine_create(&config, &machine) != LIB_STATUS_OK ||
        core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK ||
        core_machine_reset(machine) != LIB_STATUS_OK;

    if (!failed) {
        patch.mask = CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_CS) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_DS) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_ES) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_SS) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EIP) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_ESP) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EAX) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EDX) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_ECX) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EFLAGS);
        patch.values[CORE_MACHINE_DEBUG_EIP] = code_offset;
        patch.values[CORE_MACHINE_DEBUG_ESP] = 0x8000u;
        patch.values[CORE_MACHINE_DEBUG_EAX] = entry->eax;
        patch.values[CORE_MACHINE_DEBUG_EDX] = entry->edx;
        patch.values[CORE_MACHINE_DEBUG_ECX] = entry->ecx;
        patch.values[CORE_MACHINE_DEBUG_EFLAGS] = entry->flags;
        failed = core_machine_debug_patch_registers(machine, &patch) !=
                LIB_STATUS_OK ||
            core_machine_memory_write(machine, code_offset, entry->code,
                entry->bytes) != LIB_STATUS_OK ||
            core_machine_memory_write(machine, 0u, &handler_offset,
                sizeof(handler_offset)) != LIB_STATUS_OK ||
            core_machine_memory_write(machine, 2u, &handler_segment,
                sizeof(handler_segment)) != LIB_STATUS_OK ||
            core_machine_memory_write(machine, handler_offset, handler,
                sizeof(handler)) != LIB_STATUS_OK ||
            core_machine_debug_capture_cpu_snapshot(machine,
                CORE_MACHINE_CPU_SNAPSHOT_CURRENT, &before) != LIB_STATUS_OK ||
            core_machine_run(machine, (core_machine_run_budget){1u,0u},
                &result) != LIB_STATUS_OK ||
            result.reason != CORE_MACHINE_STOP_BUDGET ||
            core_machine_get_cpu_diagnostic(machine, &diagnostic) !=
                LIB_STATUS_OK ||
            core_machine_debug_capture_cpu_snapshot(machine,
                CORE_MACHINE_CPU_SNAPSHOT_CURRENT, &after) != LIB_STATUS_OK ||
            diagnostic.first_fault.valid ||
            !diagnostic.last_delivered_exception.valid ||
            !(diagnostic.last_delivered_exception.exception_mask &
                VCPUINS_EXCEPT_DE) ||
            after.eip != handler_offset || after.eax != entry->eax ||
            after.edx != entry->edx || after.ecx != entry->ecx ||
            after.eflags != entry->flags ||
            after.esp != ((before.esp & 0xffff0000u) |
                (lib_u16)(before.esp - 3u * frame_width));
        if (!failed && frame_width == 2u)
            failed = core_machine_debug_read_real(machine, 0u,
                (lib_u16)after.esp, frame16, sizeof(frame16)) !=
                LIB_STATUS_OK || frame16[0] != code_offset ||
                frame16[1] != before.cs.selector ||
                frame16[2] != (lib_u16)((before.eflags &
                    ~VCPU_EFLAGS_RESERVED) | 0x02u);
        if (!failed && frame_width == 4u)
            failed = core_machine_debug_read_real(machine, 0u,
                (lib_u16)after.esp, frame32, sizeof(frame32)) !=
                LIB_STATUS_OK || frame32[0] != code_offset ||
                frame32[1] != before.cs.selector ||
                frame32[2] != ((before.eflags &
                    ~VCPU_EFLAGS_RESERVED) | 0x02u);
    }
    core_machine_destroy(machine);
    return !failed;
}

#endif
