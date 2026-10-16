#ifndef TEST_CPU_BOARD_IRQ_FIXTURE_H
#define TEST_CPU_BOARD_IRQ_FIXTURE_H

#include "lib/types/types_interface.h"
#include "core/board-base/machine_board_interface.h"
#include "x86-core/machine/debug_interface.h"
#include "core/board-base/machine_board_state.h"
#include "pic_fixture.h"

typedef struct test_cpu_board_irq_case {
    lib_u8 code[2];
    lib_u32 eax;
    lib_u32 flags;
    lib_u16 stack_image;
    lib_bool write_stack_image;
    lib_u8 budget;
} test_cpu_board_irq_case;

typedef struct test_cpu_board_irq_result {
    x86_core_cpu_debug_cpu_snapshot before;
    x86_core_cpu_debug_cpu_snapshot after;
    lib_u16 frame_ip;
    lib_u8 isr;
    lib_u8 irr;
    lib_u16 stack_word;
} test_cpu_board_irq_result;

static lib_i32 test_cpu_board_irq_run(const test_cpu_board_irq_case *entry,
    test_cpu_board_irq_result *out)
{
    static const lib_u8 hlt = 0xf4u;
    const core_machine_config config = {
        .memory_bytes = (2u * 1024u * 1024u),
        .cpu_profile = X86_CORE_CPU_PROFILE_80386,
        .fpu_profile = X86_FPU_PROFILE_NONE
    };
    x86_core_machine *machine = LIB_NULL;
    core_machine_board_state *board = LIB_NULL;
    core_machine_pic_irq_source *irq = LIB_NULL;
    x86_core_cpu_debug_register_patch patch = {0};
    x86_core_machine_run_result result = {0};
    lib_u16 offset = 0x100u;
    lib_u16 segment = 0u;
    lib_i32 failed = entry == LIB_NULL || out == LIB_NULL ||
        core_machine_create(&config, &machine, &board) != LIB_STATUS_OK;

    if (!failed) {
        lib_memory_set(out, 0, sizeof(*out));
        patch.mask = X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_CS) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_SS) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_EIP) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_ESP) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_EFLAGS) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_EAX) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_ECX) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_EDX) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_EBX) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_EBP) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_ESI) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_EDI);
        patch.values[X86_CORE_CPU_DEBUG_ESP] = 0x8000u;
        patch.values[X86_CORE_CPU_DEBUG_EFLAGS] = entry->flags;
        patch.values[X86_CORE_CPU_DEBUG_EAX] = entry->eax;
        patch.values[X86_CORE_CPU_DEBUG_ECX] = 0x11223344u;
        patch.values[X86_CORE_CPU_DEBUG_EDX] = 0x55667788u;
        patch.values[X86_CORE_CPU_DEBUG_EBX] = 0x99aabbccU;
        patch.values[X86_CORE_CPU_DEBUG_EBP] = 0x120u;
        patch.values[X86_CORE_CPU_DEBUG_ESI] = 0x10u;
        patch.values[X86_CORE_CPU_DEBUG_EDI] = 0x20u;
        failed = x86_core_machine_freeze_execution_providers(machine) !=
                LIB_STATUS_OK || x86_core_machine_reset(machine) !=
                LIB_STATUS_OK ||
            x86_core_machine_debug_patch_registers(machine, &patch) !=
                LIB_STATUS_OK ||
            x86_core_machine_memory_write(machine, 0u, entry->code,
                sizeof(entry->code)) != LIB_STATUS_OK ||
            x86_core_machine_memory_write(machine, 0x80u, &offset,
                sizeof(offset)) != LIB_STATUS_OK ||
            x86_core_machine_memory_write(machine, 0x82u, &segment,
                sizeof(segment)) != LIB_STATUS_OK ||
            x86_core_machine_memory_write(machine, 0x100u, &hlt,
                sizeof(hlt)) != LIB_STATUS_OK ||
            (entry->write_stack_image &&
                x86_core_machine_memory_write(machine, 0x8000u,
                    &entry->stack_image, sizeof(entry->stack_image)) !=
                    LIB_STATUS_OK) ||
            x86_core_machine_debug_capture_cpu_snapshot(machine,
                X86_CORE_CPU_SNAPSHOT_CURRENT, &out->before) !=
                LIB_STATUS_OK;
    }
    if (!failed) {
        test_pic_program_vector(board->shared_pic_master, 0x20u);
        if (core_machine_pic_irq_source_bind(&irq,
                board->shared_pic_master, board->shared_pic_slave, 0u) !=
                LIB_STATUS_OK) { failed = 1; goto done; }
        core_machine_pic_irq_source_assert(irq);
        core_machine_pic_irq_source_deassert(irq);
        failed = x86_core_machine_run(machine,
                (x86_core_machine_run_budget){entry->budget,0u}, &result) !=
                LIB_STATUS_OK ||
            result.reason != X86_CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT ||
            x86_core_machine_debug_capture_cpu_snapshot(machine,
                X86_CORE_CPU_SNAPSHOT_CURRENT, &out->after) !=
                LIB_STATUS_OK ||
            x86_core_machine_debug_read_memory(machine,
                out->after.ss.base + (lib_u16)out->after.esp,
                &out->frame_ip, sizeof(out->frame_ip)) != LIB_STATUS_OK;
        if (!failed) {
            out->isr = test_pic_read(board->shared_pic_master, 0x0bu);
            out->irr = test_pic_read(board->shared_pic_master, 0x0au);
            failed = x86_core_machine_debug_read_memory(machine, 0x7ffeu,
                &out->stack_word, sizeof(out->stack_word)) != LIB_STATUS_OK;
        }
    }
done:
    x86_core_machine_destroy(machine);
    return !failed;
}

#endif
