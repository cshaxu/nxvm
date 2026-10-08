#include "lib/types/test.h"
#include "lib/types/file.h"
#include "lib/types/types_interface.h"
#include "ibmpc/board-common/machine_board_state.h"
#include "x86/core/device_support_interface.h"
#include "pic_fixture.h"
#include "core_machine_board_fixture.h"

typedef struct fpu_interface_machine {
    core_machine *machine;
    core_machine_board_state *board;
} fpu_interface_machine;

static core_machine_debug_cpu_snapshot fpu_interface_capture(core_machine *machine)
{
    core_machine_debug_cpu_snapshot snapshot = {0};
    if (core_machine_debug_capture_cpu_snapshot(machine, CORE_MACHINE_CPU_SNAPSHOT_CURRENT,
            &snapshot) != LIB_STATUS_OK) lib_test_assert(LIB_FALSE);
    return snapshot;
}

static lib_i32 fpu_interface_prepare(core_machine_cpu_profile profile,
    x86_fpu_profile fpu_profile, fpu_interface_machine *state)
{
    const core_machine_config config = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
        .cpu_profile = profile, .fpu_profile = fpu_profile
    };
    const core_machine_debug_register_patch entry = {
        .mask = CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_CS) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_DS) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_ES) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_SS) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EIP),
        .values = {0}
    };
    lib_memory_set(state, 0, sizeof(*state));
    return test_core_machine_fixture_create_bind_freeze_reset(&config, LIB_NULL,
        LIB_NULL, &state->machine, &state->board) &&
        core_machine_debug_patch_registers(state->machine, &entry) == LIB_STATUS_OK;
}

static lib_i32 fpu_interface_irq(const lib_u8 *instruction,
    lib_size instruction_size)
{
    static const lib_u8 hlt = 0xf4u;
    const lib_u16 offset = 0x0100u;
    const lib_u16 segment = 0u;
    fpu_interface_machine state = {0};
    core_machine_pic_irq_source *irq = LIB_NULL;
    core_machine_run_result result;
    core_machine_debug_cpu_snapshot before;
    core_machine_debug_cpu_snapshot after;
    lib_u8 code[8] = { 0u };
    lib_u16 frame_ip = 0u;
    lib_i32 failed = instruction_size >= sizeof(code) ||
        !fpu_interface_prepare(CORE_MACHINE_CPU_PROFILE_80386,
            X86_FPU_PROFILE_NONE, &state);

    if (!failed) {
        lib_memory_copy(code, instruction, instruction_size);
        code[instruction_size] = 0x90u;
        failed |= core_machine_memory_write(state.machine, 0u, code,
            instruction_size + 1u) != LIB_STATUS_OK;
        failed |= core_machine_memory_write(state.machine, 0x80u, &offset,
            sizeof(offset)) != LIB_STATUS_OK;
        failed |= core_machine_memory_write(state.machine, 0x82u, &segment,
            sizeof(segment)) != LIB_STATUS_OK;
        failed |= core_machine_memory_write(state.machine, offset, &hlt,
            sizeof(hlt)) != LIB_STATUS_OK;
    }
    if (!failed) {
        test_core_machine_fixture_write_register(state.machine, CORE_MACHINE_DEBUG_EFLAGS,
            test_core_machine_fixture_read_register(state.machine, CORE_MACHINE_DEBUG_EFLAGS) | CORE_MACHINE_DEBUG_EFLAGS_IF);
        before = fpu_interface_capture(state.machine);
        lib_memory_set(&irq, 0, sizeof(irq));
        test_pic_program_vector(state.board->shared_pic_master, 0x20u);
        test_pic_bind_source(&irq, state.board->shared_pic_master,
            state.board->shared_pic_slave, 0u);
        core_machine_pic_irq_source_assert(irq);
        core_machine_pic_irq_source_deassert(irq);
        failed |= core_machine_run(state.machine,
            (core_machine_run_budget){ 2u, 0u }, &result) != LIB_STATUS_OK ||
            result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT;
        after = fpu_interface_capture(state.machine);
        failed |= after.eip != offset + 1u || frame_ip != 0u ||
            core_machine_memory_read(state.machine,
                after.ss.base + (lib_u16)after.esp,
                (void *)CORE_MACHINE_REFERENCE_OF(frame_ip), sizeof(frame_ip)) != LIB_STATUS_OK ||
            frame_ip != instruction_size || after.eax != before.eax ||
            after.ebx != before.ebx || after.ecx != before.ecx ||
            after.edx != before.edx || after.ebp != before.ebp ||
            after.esi != before.esi || after.edi != before.edi ||
            !CORE_MACHINE_BIT_IS_SET(test_pic_read(state.board->shared_pic_master, 0x0bu),
                VPIC_ISR_IRQ(0u)) || CORE_MACHINE_BIT_IS_SET(
                test_pic_read(state.board->shared_pic_master, 0x0au), VPIC_IRR_IRQ(0u));
    }
    core_machine_destroy(state.machine);
    return !failed;
}

int main(void)
{
    const lib_u8 wait[] = {0x9bu};
    const lib_u8 fninit[] = {0xdbu,0xe3u};
    if (!fpu_interface_irq(wait, sizeof(wait)) ||
        !fpu_interface_irq(fninit, sizeof(fninit))) return 1;
    lib_c_printf("%s\n", "FPU WAIT/ESC Board IRQ retirement: PASS");
    return 0;
}
