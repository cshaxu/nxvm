#include "pic_fixture.h"
#include "ibmpc/board-common/machine_board_state.h"
#include "core_machine_board_fixture.h"
#include "x86/core/device_support_interface.h"
#include "ibmpc/board-common/machine_board_interface.h"
#include <stdio.h>

typedef struct prefix_attributes_s64_machine {
    core_machine *machine;
    core_machine_board_state *board;
} prefix_attributes_s64_machine;

static lib_i32 prefix_attributes_s64_prepare(prefix_attributes_s64_machine *state)
{
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
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EIP)
    };

    lib_memory_set(state, 0, sizeof(*state));
    return core_machine_create(&config, &state->machine, &state->board) == LIB_STATUS_OK &&
        core_machine_freeze_execution_providers(state->machine) == LIB_STATUS_OK &&
        core_machine_reset(state->machine) == LIB_STATUS_OK &&
        core_machine_debug_patch_registers(state->machine, &entry) == LIB_STATUS_OK;
}

static lib_i32 prefix_attributes_s64_test_irq_no_shadow(void)
{
    static const lib_u8 code[] = {
        0x3eu, 0x8au, 0x06u, 0x00u, 0x10u, 0x90u
    };
    static const lib_u8 hlt[] = { 0xf4u };
    const core_machine_debug_register_patch registers = {
        .mask = CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EAX) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EFLAGS),
        .values = {
            [CORE_MACHINE_DEBUG_EAX] = 0xaabbcc00u,
            [CORE_MACHINE_DEBUG_EFLAGS] = 0x202u
        }
    };
    prefix_attributes_s64_machine state;
    core_machine_pic_irq_source *source = LIB_NULL;
    core_machine_run_result result;
    core_machine_debug_cpu_snapshot after = {0};
    lib_u16 vector_offset = 0x0100u;
    lib_u16 vector_segment = 0u;
    lib_u16 frame_ip = 0u;
    lib_u8 image = 0x6du;
    lib_i32 failed = !prefix_attributes_s64_prepare(&state);

    if (!failed) {
        failed |= core_machine_memory_write(state.machine, 0x1000u, &image,
            sizeof(image)) != LIB_STATUS_OK ||
            core_machine_memory_write(state.machine, 0x0080u,
                &vector_offset, sizeof(vector_offset)) != LIB_STATUS_OK ||
            core_machine_memory_write(state.machine, 0x0082u,
                &vector_segment, sizeof(vector_segment)) != LIB_STATUS_OK ||
            core_machine_memory_write(state.machine, 0x0100u, hlt,
                sizeof(hlt)) != LIB_STATUS_OK ||
            core_machine_memory_write(state.machine, 0u, code,
                sizeof(code)) != LIB_STATUS_OK ||
            core_machine_debug_patch_registers(state.machine,
                &registers) != LIB_STATUS_OK;
    }
    if (!failed) {
        lib_memory_set(&source, 0, sizeof(source));
        test_pic_program_vector(state.board->shared_pic_master, 0x20u);
        test_pic_bind_source(&source,
            state.board->shared_pic_master,
            state.board->shared_pic_slave, 0u);
        core_machine_pic_irq_source_assert(source);
        core_machine_pic_irq_source_deassert(source);
        failed |= core_machine_run(state.machine,
            (core_machine_run_budget){ 2u, 0u }, &result) != LIB_STATUS_OK ||
            result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT ||
            core_machine_debug_capture_cpu_snapshot(state.machine,
                CORE_MACHINE_CPU_SNAPSHOT_CURRENT, &after) != LIB_STATUS_OK ||
            core_machine_memory_read(state.machine,
                after.ss.base + (lib_u16)after.esp, &frame_ip,
                sizeof(frame_ip)) != LIB_STATUS_OK ||
            after.eip != 0x0101u || frame_ip != 5u ||
            (after.eax & 0xffu) != image ||
            !CORE_MACHINE_BIT_IS_SET(
                test_pic_read(state.board->shared_pic_master, 0x0bu),
                VPIC_ISR_IRQ(0u)) ||
            CORE_MACHINE_BIT_IS_SET(
                test_pic_read(state.board->shared_pic_master, 0x0au),
                VPIC_IRR_IRQ(0u));
    }
    core_machine_destroy(state.machine);
    return !failed;
}

lib_i32 main(void)
{
    if (!prefix_attributes_s64_test_irq_no_shadow()) {
        fprintf(stderr, "S64 prefix IRQ failed\n");
        return 1;
    }
    printf("M5:T316:S64:PREFIX-ATTRIBUTES:OK\n");
    printf("M5:T401:S57:SHARED-PREFIX-PROFILES:OK\n");
    return 0;
}
