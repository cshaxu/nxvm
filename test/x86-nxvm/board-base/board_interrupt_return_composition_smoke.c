#include "lib/types/test.h"
#include "lib/types/file.h"
#include "pic_fixture.h"
#include "x86-nxvm/board-base/machine_board_state.h"
#include "lib/types/types_interface.h"

#include "x86-core/machine/machine_interface.h"
#include "x86-nxvm/board-base/pic_bus_interface.h"

/* The retained CLI/STI fixture supplies the real-mode machine lifecycle. */
#define main interrupt_return_composition_cli_sti_main
#include "machine_cli_sti_interrupt_smoke.c"
#undef main

static void interrupt_return_composition_seed(cli_sti_machine *state,
    lib_u32 flags)
{
    const x86_core_cpu_debug_register_patch patch = {
        .mask = X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_EAX) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_ECX) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_EDX) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_EBX) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_ESP) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_EBP) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_ESI) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_EDI) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_EFLAGS),
        .values = {
            [X86_CORE_CPU_DEBUG_EAX] = 0xaabbccddu,
            [X86_CORE_CPU_DEBUG_ECX] = 0x11223344u,
            [X86_CORE_CPU_DEBUG_EDX] = 0x55667788u,
            [X86_CORE_CPU_DEBUG_EBX] = 0x99aabbccu,
            [X86_CORE_CPU_DEBUG_ESP] = 0x00008000u,
            [X86_CORE_CPU_DEBUG_EBP] = 0x00000120u,
            [X86_CORE_CPU_DEBUG_ESI] = 0x00000010u,
            [X86_CORE_CPU_DEBUG_EDI] = 0x00000020u,
            [X86_CORE_CPU_DEBUG_EFLAGS] = flags
        }
    };
    if (x86_core_machine_debug_patch_registers(state->machine, &patch) != LIB_STATUS_OK)
        lib_test_assert(LIB_FALSE);
}

static lib_i32 interrupt_return_composition_real_irq_after_iret(void)
{
    const lib_u16 int_offset = 0x0100u;
    const lib_u16 irq_offset = 0x0120u;
    const lib_u16 segment = 0u;
    const lib_u32 flags = X86_CORE_CPU_DEBUG_EFLAGS_CF | X86_CORE_CPU_DEBUG_EFLAGS_IF;
    static const lib_u8 program[] = { 0xcdu, 0x31u, 0x90u };
    static const lib_u8 iret[] = { 0xcfu };
    static const lib_u8 hlt[] = { 0xf4u };
    cli_sti_machine state;
    x86_nxvm_board_pic_irq_source *irq = LIB_NULL;
    x86_core_machine_run_result result;
    x86_core_cpu_diagnostic diagnostic;
    x86_core_cpu_debug_cpu_snapshot after;
    lib_u16 frame[3u] = { 0u, 0u, 0u };
    lib_i32 failed = !cli_sti_prepare(X86_CORE_CPU_PROFILE_80386, &state, LIB_FALSE) ||
        !cli_sti_real_entry(state.machine, 0u);

    lib_memory_set(&irq, 0, sizeof(irq));
    if (!failed) {
        failed |= x86_core_machine_memory_write(state.machine, 0u, program,
                sizeof(program)) != LIB_STATUS_OK ||
            x86_core_machine_memory_write(state.machine, 0xc4u, &int_offset,
                sizeof(int_offset)) != LIB_STATUS_OK ||
            x86_core_machine_memory_write(state.machine, 0xc6u, &segment,
                sizeof(segment)) != LIB_STATUS_OK ||
            x86_core_machine_memory_write(state.machine, int_offset, iret,
                sizeof(iret)) != LIB_STATUS_OK ||
            x86_core_machine_memory_write(state.machine, 0x80u, &irq_offset,
                sizeof(irq_offset)) != LIB_STATUS_OK ||
            x86_core_machine_memory_write(state.machine, 0x82u, &segment,
                sizeof(segment)) != LIB_STATUS_OK ||
            x86_core_machine_memory_write(state.machine, irq_offset, hlt,
                sizeof(hlt)) != LIB_STATUS_OK;
    }
    if (!failed) {
        interrupt_return_composition_seed(&state, flags);
        test_pic_program_vector(state.board->shared_pic_master, 0x20u);
        test_pic_bind_source(&irq, state.board->shared_pic_master,
            state.board->shared_pic_slave, 0u);
        x86_nxvm_board_pic_irq_source_assert(irq);
        x86_nxvm_board_pic_irq_source_deassert(irq);
        failed |= x86_core_machine_run(state.machine,
                (x86_core_machine_run_budget){ 8u, 0u }, &result) != LIB_STATUS_OK ||
            x86_core_machine_get_cpu_diagnostic(state.machine, &diagnostic) !=
                LIB_STATUS_OK;
        after = cli_sti_capture(state.machine);
        if (!failed) failed |= result.reason != X86_CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT ||
            diagnostic.first_fault.valid || after.eip != irq_offset + 1u ||
            after.esp != 0x00007ffau ||
            (((after.eflags) & (X86_CORE_CPU_DEBUG_EFLAGS_IF)) != 0u) ||
            (((after.eflags) & (X86_CORE_CPU_DEBUG_EFLAGS_TF)) != 0u) ||
            !(((test_pic_read(state.board->shared_pic_master, 0x0bu)) & (VPIC_ISR_IRQ(0u))) != 0u) || (((test_pic_read(state.board->shared_pic_master, 0x0au)) & (VPIC_IRR_IRQ(0u))) != 0u) ||
            x86_core_machine_memory_read(state.machine,
                after.ss.base + (lib_u16)after.esp,
                (void *)frame, sizeof(frame)) != LIB_STATUS_OK ||
            frame[0] != 2u || frame[1] != 0u || frame[2] !=
                (flags | 0x02u);
    }
    x86_core_machine_destroy(state.machine);
    return !failed;
}

lib_i32 main(void)
{
    if (!interrupt_return_composition_real_irq_after_iret()) return 1;
    lib_c_printf("INTERRUPT-RETURN-COMPOSITION:OK\n");
    return 0;
}
