#include "support/pic_fixture.h"
#include "app-nxvm/devices/machine_board_interface.h"
#include "app-nxvm/devices/machine_board_state.h"
#include "lib/types/types_interface.h"
#include <stdio.h>
#include "app-nxvm/devices/device_support.h"
#include "app-nxvm/devices/pic_bus.h"
#include "support/core_machine_board_fixture.h"

static lib_i32 lea_test_irq_no_shadow(void)
{
    static const lib_u8 code[] = { 0x8du, 0x40u, 0x10u, 0x90u };
    static const lib_u8 hlt = 0xf4u;
    core_machine *machine = LIB_NULL;
    core_machine_board_state *board = LIB_NULL;
    const core_machine_config config = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
        .cpu_profile = CORE_MACHINE_CPU_PROFILE_80386,
        .fpu_profile = X86_FPU_PROFILE_NONE
    };
    const core_machine_debug_register_patch entry = {
        .mask = CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_CS) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_DS) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_ES) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_SS) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EIP) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EAX) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EBX) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_ESI) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EBP) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EFLAGS),
        .values = {
            [CORE_MACHINE_DEBUG_EAX] = 0xaabb0000u,
            [CORE_MACHINE_DEBUG_EBX] = 0x11112000u,
            [CORE_MACHINE_DEBUG_ESI] = 0x12345000u,
            [CORE_MACHINE_DEBUG_EBP] = 0x56780000u,
            [CORE_MACHINE_DEBUG_EFLAGS] = 0x241u /* CF | ZF | IF */
        }
    };
    core_machine_pic_irq_source source;
    core_machine_run_result result;
    core_machine_debug_cpu_snapshot after;
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
        test_pic_program_vector(&board->shared_pic_master, 0x20u);
        core_machine_pic_irq_source_bind(&source,
            &board->shared_pic_master, &board->shared_pic_slave,
            0u);
        core_machine_pic_irq_source_assert(&source);
        core_machine_pic_irq_source_deassert(&source);
        failed |= core_machine_run(machine,
                (core_machine_run_budget){ 2u, 0u }, &result) != LIB_STATUS_OK ||
            result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT;
        failed |= core_machine_debug_capture_cpu_snapshot(machine,
            CORE_MACHINE_CPU_SNAPSHOT_CURRENT, &after) != LIB_STATUS_OK;
        failed |= core_machine_memory_read(machine,
                after.ss.base + (lib_u16)after.esp,
                &frame_ip, sizeof(frame_ip)) != LIB_STATUS_OK ||
            after.eip != 0x0101u || !CORE_MACHINE_BIT_IS_SET(
                test_pic_read(&board->shared_pic_master, 0x0bu), VPIC_ISR_IRQ(0u)) ||
            CORE_MACHINE_BIT_IS_SET(test_pic_read(&board->shared_pic_master, 0x0au),
                VPIC_IRR_IRQ(0u)) || frame_ip != 3u;
    }
    core_machine_destroy(machine);
    return !failed;
}

lib_i32 main(void)
{
    if (!lea_test_irq_no_shadow()) return 1;
    printf("M5:T316:S26:LEA:OK\n");
    printf("M5:T401:S44:LEA-PROFILES:OK\n");
    return 0;
}
