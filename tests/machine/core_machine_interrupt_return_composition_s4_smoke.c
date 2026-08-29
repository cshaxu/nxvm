#include "type.h"

#include "core/machine/machine_interface.h"
#include "core/machine/pic.h"

/* The retained CLI/STI fixture supplies the real-mode machine lifecycle. */
#define main interrupt_return_composition_s4_cli_sti_main
#include "core_machine_cli_sti_smoke.c"
#undef main

static C_VOID interrupt_return_composition_s4_seed(cli_sti_machine *state,
    type_unsigned_32 flags)
{
    t_cpu *cpu = &state->machine->executor_cpu;

    cpu->data.eax = 0xaabbccddu;
    cpu->data.ecx = 0x11223344u;
    cpu->data.edx = 0x55667788u;
    cpu->data.ebx = 0x99aabbccu;
    cpu->data.esp = 0x00008000u;
    cpu->data.ebp = 0x00000120u;
    cpu->data.esi = 0x00000010u;
    cpu->data.edi = 0x00000020u;
    cpu->data.eflags = flags;
}

static C_INT interrupt_return_composition_s4_real_irq_after_iret(C_VOID)
{
    const type_unsigned_16 int_offset = 0x0100u;
    const type_unsigned_16 irq_offset = 0x0120u;
    const type_unsigned_16 segment = 0u;
    const type_unsigned_32 flags = VCPU_EFLAGS_CF | VCPU_EFLAGS_IF;
    static const type_unsigned_8 program[] = { 0xcdu, 0x31u, 0x90u };
    static const type_unsigned_8 iret[] = { 0xcfu };
    static const type_unsigned_8 hlt[] = { 0xf4u };
    cli_sti_machine state;
    core_machine_pic_irq_source irq;
    core_machine_run_result result;
    core_machine_cpu_diagnostic diagnostic;
    t_cpu after;
    type_unsigned_16 frame[3u] = { 0u, 0u, 0u };
    C_INT failed = !cli_sti_prepare(CORE_MACHINE_CPU_PROFILE_80386, &state) ||
        !test_core_machine_fixture_prepare_real_mode_execution(state.machine, 0u);

    STD_MEMSET(&irq, 0, sizeof(irq));
    if (!failed) {
        failed |= core_machine_memory_write(state.machine, 0u, program,
                sizeof(program)) != TYPE_STATUS_OK ||
            core_machine_memory_write(state.machine, 0xc4u, &int_offset,
                sizeof(int_offset)) != TYPE_STATUS_OK ||
            core_machine_memory_write(state.machine, 0xc6u, &segment,
                sizeof(segment)) != TYPE_STATUS_OK ||
            core_machine_memory_write(state.machine, int_offset, iret,
                sizeof(iret)) != TYPE_STATUS_OK ||
            core_machine_memory_write(state.machine, 0x80u, &irq_offset,
                sizeof(irq_offset)) != TYPE_STATUS_OK ||
            core_machine_memory_write(state.machine, 0x82u, &segment,
                sizeof(segment)) != TYPE_STATUS_OK ||
            core_machine_memory_write(state.machine, irq_offset, hlt,
                sizeof(hlt)) != TYPE_STATUS_OK;
    }
    if (!failed) {
        interrupt_return_composition_s4_seed(&state, flags);
        state.machine->shared_pic_master.data.icw2 = 0x20u;
        core_machine_pic_irq_source_bind(&irq, &state.machine->shared_pic_master,
            &state.machine->shared_pic_slave, 0u);
        core_machine_pic_irq_source_assert(&irq);
        core_machine_pic_irq_source_deassert(&irq);
        failed |= core_machine_run(state.machine,
                (core_machine_run_budget){ 8u, 0u }, &result) != TYPE_STATUS_OK ||
            core_machine_get_cpu_diagnostic(state.machine, &diagnostic) !=
                TYPE_STATUS_OK;
        after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT ||
            diagnostic.first_fault.valid || after.data.eip != irq_offset + 1u ||
            after.data.esp != 0x00007ffau ||
            TYPE_GET_BIT(after.data.eflags, VCPU_EFLAGS_IF) ||
            TYPE_GET_BIT(after.data.eflags, VCPU_EFLAGS_TF) ||
            !TYPE_GET_BIT(state.machine->shared_pic_master.data.isr,
                VPIC_ISR_IRQ(0u)) || TYPE_GET_BIT(
                state.machine->shared_pic_master.data.irr, VPIC_IRR_IRQ(0u)) ||
            core_machine_memory_read_physical(&state.machine->executor_memory,
                after.data.ss.base + (type_unsigned_16)after.data.esp,
                (type_virtual_address)frame, sizeof(frame)) != TYPE_STATUS_OK ||
            frame[0] != 2u || frame[1] != 0u || frame[2] !=
                (flags | 0x02u);
    }
    core_machine_destroy(state.machine);
    return !failed;
}

C_INT main(C_VOID)
{
    if (!interrupt_return_composition_s4_real_irq_after_iret()) return 1;
    STD_PRINTF("M5:T321:S4:INTERRUPT-RETURN-COMPOSITION:OK\n");
    return 0;
}
