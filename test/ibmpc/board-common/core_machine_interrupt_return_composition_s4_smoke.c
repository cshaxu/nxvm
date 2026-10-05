#include "lib/types/test.h"
#include "lib/types/file.h"
#include "pic_fixture.h"
#include "ibmpc/board-common/machine_board_state.h"
#include "lib/types/types_interface.h"
#include "x86/core/device_support_interface.h"

#include "x86/core/machine_interface.h"
#include "ibmpc/board-common/pic_bus_interface.h"

/* The retained CLI/STI fixture supplies the real-mode machine lifecycle. */
#define main interrupt_return_composition_s4_cli_sti_main
#include "machine_cli_sti_interrupt_smoke.c"
#undef main

static void interrupt_return_composition_s4_seed(cli_sti_machine *state,
    lib_u32 flags)
{
    const core_machine_debug_register_patch patch = {
        .mask = CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EAX) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_ECX) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EDX) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EBX) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_ESP) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EBP) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_ESI) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EDI) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EFLAGS),
        .values = {
            [CORE_MACHINE_DEBUG_EAX] = 0xaabbccddu,
            [CORE_MACHINE_DEBUG_ECX] = 0x11223344u,
            [CORE_MACHINE_DEBUG_EDX] = 0x55667788u,
            [CORE_MACHINE_DEBUG_EBX] = 0x99aabbccu,
            [CORE_MACHINE_DEBUG_ESP] = 0x00008000u,
            [CORE_MACHINE_DEBUG_EBP] = 0x00000120u,
            [CORE_MACHINE_DEBUG_ESI] = 0x00000010u,
            [CORE_MACHINE_DEBUG_EDI] = 0x00000020u,
            [CORE_MACHINE_DEBUG_EFLAGS] = flags
        }
    };
    if (core_machine_debug_patch_registers(state->machine, &patch) != LIB_STATUS_OK)
        lib_test_assert(LIB_FALSE);
}

static lib_i32 interrupt_return_composition_s4_real_irq_after_iret(void)
{
    const lib_u16 int_offset = 0x0100u;
    const lib_u16 irq_offset = 0x0120u;
    const lib_u16 segment = 0u;
    const lib_u32 flags = CORE_MACHINE_DEBUG_EFLAGS_CF | CORE_MACHINE_DEBUG_EFLAGS_IF;
    static const lib_u8 program[] = { 0xcdu, 0x31u, 0x90u };
    static const lib_u8 iret[] = { 0xcfu };
    static const lib_u8 hlt[] = { 0xf4u };
    cli_sti_machine state;
    core_machine_pic_irq_source *irq = LIB_NULL;
    core_machine_run_result result;
    core_machine_cpu_diagnostic diagnostic;
    core_machine_debug_cpu_snapshot after;
    lib_u16 frame[3u] = { 0u, 0u, 0u };
    lib_i32 failed = !cli_sti_prepare(CORE_MACHINE_CPU_PROFILE_80386, &state, LIB_FALSE) ||
        !cli_sti_real_entry(state.machine, 0u);

    lib_memory_set(&irq, 0, sizeof(irq));
    if (!failed) {
        failed |= core_machine_memory_write(state.machine, 0u, program,
                sizeof(program)) != LIB_STATUS_OK ||
            core_machine_memory_write(state.machine, 0xc4u, &int_offset,
                sizeof(int_offset)) != LIB_STATUS_OK ||
            core_machine_memory_write(state.machine, 0xc6u, &segment,
                sizeof(segment)) != LIB_STATUS_OK ||
            core_machine_memory_write(state.machine, int_offset, iret,
                sizeof(iret)) != LIB_STATUS_OK ||
            core_machine_memory_write(state.machine, 0x80u, &irq_offset,
                sizeof(irq_offset)) != LIB_STATUS_OK ||
            core_machine_memory_write(state.machine, 0x82u, &segment,
                sizeof(segment)) != LIB_STATUS_OK ||
            core_machine_memory_write(state.machine, irq_offset, hlt,
                sizeof(hlt)) != LIB_STATUS_OK;
    }
    if (!failed) {
        interrupt_return_composition_s4_seed(&state, flags);
        test_pic_program_vector(state.board->shared_pic_master, 0x20u);
        test_pic_bind_source(&irq, state.board->shared_pic_master,
            state.board->shared_pic_slave, 0u);
        core_machine_pic_irq_source_assert(irq);
        core_machine_pic_irq_source_deassert(irq);
        failed |= core_machine_run(state.machine,
                (core_machine_run_budget){ 8u, 0u }, &result) != LIB_STATUS_OK ||
            core_machine_get_cpu_diagnostic(state.machine, &diagnostic) !=
                LIB_STATUS_OK;
        after = cli_sti_capture(state.machine);
        if (!failed) failed |= result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT ||
            diagnostic.first_fault.valid || after.eip != irq_offset + 1u ||
            after.esp != 0x00007ffau ||
            CORE_MACHINE_BIT_IS_SET(after.eflags, CORE_MACHINE_DEBUG_EFLAGS_IF) ||
            CORE_MACHINE_BIT_IS_SET(after.eflags, CORE_MACHINE_DEBUG_EFLAGS_TF) ||
            !CORE_MACHINE_BIT_IS_SET(test_pic_read(state.board->shared_pic_master, 0x0bu),
                VPIC_ISR_IRQ(0u)) || CORE_MACHINE_BIT_IS_SET(
                test_pic_read(state.board->shared_pic_master, 0x0au), VPIC_IRR_IRQ(0u)) ||
            core_machine_memory_read(state.machine,
                after.ss.base + (lib_u16)after.esp,
                (void *)frame, sizeof(frame)) != LIB_STATUS_OK ||
            frame[0] != 2u || frame[1] != 0u || frame[2] !=
                (flags | 0x02u);
    }
    core_machine_destroy(state.machine);
    return !failed;
}

lib_i32 main(void)
{
    if (!interrupt_return_composition_s4_real_irq_after_iret()) return 1;
    lib_c_printf("M5:T321:S4:INTERRUPT-RETURN-COMPOSITION:OK\n");
    return 0;
}
