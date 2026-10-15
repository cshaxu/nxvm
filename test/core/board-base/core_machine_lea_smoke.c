#include "lib/types/file.h"
#include "pic_fixture.h"
#include "core/board-base/machine_board_interface.h"
#include "core/board-base/machine_board_state.h"
#include "lib/types/types_interface.h"
#include "core/x86/device_support_interface.h"
#include "core/board-base/pic_bus_interface.h"
#include "core_machine_board_fixture.h"

static lib_i32 lea_test_irq_no_shadow(void)
{
    static const lib_u8 code[] = { 0x8du, 0x40u, 0x10u, 0x90u };
    static const lib_u8 hlt = 0xf4u;
    core_machine *machine = LIB_NULL;
    core_machine_board_state *board = LIB_NULL;
    const core_machine_config config = {
        .memory_bytes = (2u * 1024u * 1024u),
        .cpu_profile = X86_CORE_CPU_PROFILE_80386,
        .fpu_profile = X86_FPU_PROFILE_NONE
    };
    const x86_core_cpu_debug_register_patch entry = {
        .mask = X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_CS) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_DS) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_ES) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_SS) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_EIP) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_EAX) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_EBX) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_ESI) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_EBP) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_EFLAGS),
        .values = {
            [X86_CORE_CPU_DEBUG_EAX] = 0xaabb0000u,
            [X86_CORE_CPU_DEBUG_EBX] = 0x11112000u,
            [X86_CORE_CPU_DEBUG_ESI] = 0x12345000u,
            [X86_CORE_CPU_DEBUG_EBP] = 0x56780000u,
            [X86_CORE_CPU_DEBUG_EFLAGS] = 0x241u /* CF | ZF | IF */
        }
    };
    core_machine_pic_irq_source *source = LIB_NULL;
    core_machine_run_result result;
    x86_core_cpu_debug_cpu_snapshot after;
    lib_u16 vector_offset = 0x0100u;
    lib_u16 vector_segment = 0u;
    lib_u16 frame_ip = 0u;
    lib_i32 failed = core_machine_create(&config, &machine, &board) != LIB_STATUS_OK ||
        core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK ||
        core_machine_reset(machine) != LIB_STATUS_OK;

    if (!failed) {
        failed |= core_machine_debug_patch_registers(machine, &entry) != LIB_STATUS_OK ||
            core_machine_memory_write(machine, 0u, code,
                sizeof(code)) != LIB_STATUS_OK ||
            core_machine_memory_write(machine, 0x20u * 4u,
                &vector_offset, sizeof(vector_offset)) != LIB_STATUS_OK ||
            core_machine_memory_write(machine, 0x20u * 4u + 2u,
                &vector_segment, sizeof(vector_segment)) != LIB_STATUS_OK ||
            core_machine_memory_write(machine, 0x0100u, &hlt,
                sizeof(hlt)) != LIB_STATUS_OK;
    }
    if (!failed) {
        lib_memory_set(&source, 0, sizeof(source));
        test_pic_program_vector(board->shared_pic_master, 0x20u);
        test_pic_bind_source(&source,
            board->shared_pic_master, board->shared_pic_slave,
            0u);
        core_machine_pic_irq_source_assert(source);
        core_machine_pic_irq_source_deassert(source);
        failed |= core_machine_run(machine,
                (core_machine_run_budget){ 2u, 0u }, &result) != LIB_STATUS_OK ||
            result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT;
        failed |= core_machine_debug_capture_cpu_snapshot(machine,
            X86_CORE_CPU_SNAPSHOT_CURRENT, &after) != LIB_STATUS_OK;
        if (!failed) failed |= core_machine_memory_read(machine,
                after.ss.base + (lib_u16)after.esp,
                &frame_ip, sizeof(frame_ip)) != LIB_STATUS_OK ||
            after.eip != 0x0101u || !CORE_MACHINE_BIT_IS_SET(
                test_pic_read(board->shared_pic_master, 0x0bu), VPIC_ISR_IRQ(0u)) ||
            CORE_MACHINE_BIT_IS_SET(test_pic_read(board->shared_pic_master, 0x0au),
                VPIC_IRR_IRQ(0u)) || frame_ip != 3u;
    }
    core_machine_destroy(machine);
    return !failed;
}

lib_i32 main(void)
{
    if (!lea_test_irq_no_shadow()) return 1;
    lib_c_printf("LEA:OK\n");
    lib_c_printf("LEA-PROFILES:OK\n");
    return 0;
}
