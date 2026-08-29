#include "type.h"

#include "core/machine/cpu.h"
#include "core/machine/machine_interface.h"
#include "core/machine/pic.h"
#include "../support/core_machine_cpu_fixture.h"

/* Retained owners supply the detailed gate and VM86 frame matrices. */
#define main hardware_delivery_s3_interrupt_entry_main
#include "core_machine_interrupt_entry_smoke.c"
#undef main
#define main hardware_delivery_s3_vm86_delivery_main
#include "core_machine_vm86_delivery_smoke.c"
#undef main

typedef struct hardware_delivery_s3_real_machine {
    core_machine *machine;
} hardware_delivery_s3_real_machine;

static C_VOID hardware_delivery_s3_real_reset(C_VOID *opaque)
{
    hardware_delivery_s3_real_machine *state =
        (hardware_delivery_s3_real_machine *)opaque;

    if (state != STD_NULL) {
        (C_VOID)test_core_machine_fixture_reset_real_mode(state->machine);
    }
}

static const core_machine_execution_provider hardware_delivery_s3_real_provider = {
    hardware_delivery_s3_real_reset, STD_NULL
};

static C_INT hardware_delivery_s3_real_priority(C_VOID)
{
    const core_machine_config config = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
        .cpu_profile = CORE_MACHINE_CPU_PROFILE_80386,
        .fpu_profile = CORE_MACHINE_FPU_PROFILE_NONE
    };
    static const type_unsigned_8 program[] = { 0x90u };
    static const type_unsigned_8 handler[] = { 0xf4u };
    static const type_unsigned_8 nmi_vector[] = { 0x00u, 0x01u, 0x00u, 0x00u };
    static const type_unsigned_8 irq_vector[] = { 0x20u, 0x01u, 0x00u, 0x00u };
    hardware_delivery_s3_real_machine state;
    core_machine_pic_irq_source irq;
    core_machine_run_result result;
    type_unsigned_16 frame[3u] = { 0u, 0u, 0u };
    type_status status;
    C_INT failed = 0;

    STD_MEMSET(&state, 0, sizeof(state));
    STD_MEMSET(&irq, 0, sizeof(irq));
    if (!test_core_machine_fixture_create_bind_freeze_reset(&config,
            &hardware_delivery_s3_real_provider, &state, &state.machine)) {
        return 0;
    }
    failed |= core_machine_memory_write(state.machine, 0u, program,
            sizeof(program)) != TYPE_STATUS_OK ||
        core_machine_memory_write(state.machine, 8u, nmi_vector,
            sizeof(nmi_vector)) != TYPE_STATUS_OK ||
        core_machine_memory_write(state.machine, 0x80u, irq_vector,
            sizeof(irq_vector)) != TYPE_STATUS_OK ||
        core_machine_memory_write(state.machine, 0x0100u, handler,
            sizeof(handler)) != TYPE_STATUS_OK ||
        core_machine_memory_write(state.machine, 0x0120u, handler,
            sizeof(handler)) != TYPE_STATUS_OK;
    if (!failed) {
        state.machine->executor_cpu.data.esp = 0x00008000u;
        state.machine->executor_cpu.data.eflags = VCPU_EFLAGS_CF | VCPU_EFLAGS_IF;
        state.machine->executor_cpu.data.flagNMI = TYPE_TRUE;
        state.machine->shared_pic_master.data.icw2 = 0x20u;
        core_machine_pic_irq_source_bind(&irq, &state.machine->shared_pic_master,
            &state.machine->shared_pic_slave, 0u);
        core_machine_pic_irq_source_assert(&irq);
        core_machine_pic_irq_source_deassert(&irq);
        status = core_machine_run(state.machine, (core_machine_run_budget){ 8u, 0u },
            &result);
        failed |= status != TYPE_STATUS_OK ||
            result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT ||
            state.machine->executor_cpu.data.eip != 0x0101u ||
            state.machine->executor_cpu.data.esp != 0x00007ffau ||
            TYPE_GET_BIT(state.machine->executor_cpu.data.eflags, VCPU_EFLAGS_IF) ||
            TYPE_GET_BIT(state.machine->executor_cpu.data.eflags, VCPU_EFLAGS_TF) ||
            state.machine->executor_cpu.data.flagNMI ||
            !TYPE_GET_BIT(state.machine->shared_pic_master.data.irr, VPIC_IRR_IRQ(0u)) ||
            TYPE_GET_BIT(state.machine->shared_pic_master.data.isr, VPIC_ISR_IRQ(0u)) ||
            core_machine_memory_read_physical(&state.machine->executor_memory,
                0x00007ffau, (type_virtual_address)frame, sizeof(frame)) !=
                TYPE_STATUS_OK ||
            frame[0] != 1u || frame[1] != 0u ||
            frame[2] != (VCPU_EFLAGS_CF | VCPU_EFLAGS_IF | 0x02u);
    }
    core_machine_destroy(state.machine);
    return !failed;
}

static C_INT hardware_delivery_s3_protected_priority(C_VOID)
{
    interrupt_entry_machine state;
    core_machine_pic_irq_source irq;
    core_machine_cpu_diagnostic diagnostic;
    t_cpu after;
    type_unsigned_32 frame[3u] = { 0u, 0u, 0u };
    static const type_unsigned_8 program[] = { 0x90u };
    C_INT failed = !ie_prepare(&state, INTERRUPT_ENTRY_NEGATIVE_NONE,
        VCPU_DESC_SYS_TYPE_INTGATE_32);

    STD_MEMSET(&irq, 0, sizeof(irq));
    if (!failed) {
        state.machine->executor_cpu.data.eflags = VCPU_EFLAGS_CF | VCPU_EFLAGS_IF;
        state.machine->executor_cpu.data.flagNMI = TYPE_TRUE;
        state.machine->shared_pic_master.data.icw2 = IE_VECTOR;
        core_machine_pic_irq_source_bind(&irq, &state.machine->shared_pic_master,
            &state.machine->shared_pic_slave, 0u);
        core_machine_pic_irq_source_assert(&irq);
        core_machine_pic_irq_source_deassert(&irq);
        failed |= !ie_install_gate(&state, 0x02u, 0x0008u,
                (type_unsigned_8)(0x80u | VCPU_DESC_SYS_TYPE_INTGATE_32)) ||
            !ie_write(&state, IE_CODE_BASE, program, sizeof(program)) ||
            !ie_run(&state, 0, &after, &diagnostic) ||
            diagnostic.first_fault.valid || after.data.eip != IE_HANDLER_OFFSET + 1u ||
            after.data.esp != IE_STACK_BASE - 12u ||
            TYPE_GET_BIT(after.data.eflags, VCPU_EFLAGS_IF) ||
            TYPE_GET_BIT(after.data.eflags, VCPU_EFLAGS_TF) ||
            state.machine->executor_cpu.data.flagNMI ||
            !TYPE_GET_BIT(state.machine->shared_pic_master.data.irr, VPIC_IRR_IRQ(0u)) ||
            TYPE_GET_BIT(state.machine->shared_pic_master.data.isr, VPIC_ISR_IRQ(0u)) ||
            !ie_read(&state, IE_STACK_BASE - 12u, frame, sizeof(frame)) ||
            frame[0] != 1u || frame[1] != 0x0008u ||
            frame[2] != (VCPU_EFLAGS_CF | VCPU_EFLAGS_IF);
    }
    core_machine_destroy(state.machine);
    return !failed;
}

static C_INT hardware_delivery_s3_vm86_install_gate(
    vm86_delivery_state *state, type_unsigned_8 vector)
{
    type_unsigned_8 gate[8u] = { 0u };

    gate[0] = 0u;
    gate[1] = 0x01u;
    gate[2] = 0x08u;
    gate[5] = 0x8eu;
    return core_machine_memory_write(state->machine,
        VM86_IDT_BASE + (type_unsigned_32)vector * 8u, gate,
        sizeof(gate)) == TYPE_STATUS_OK;
}

static C_INT hardware_delivery_s3_vm86_pic_matches(
    const vm86_delivery_state *state, C_INT nmi_masked)
{
    type_bool irq_pending = TYPE_GET_BIT(state->machine->shared_pic_master.data.irr,
        VPIC_IRR_IRQ(0u));
    type_bool irq_active = TYPE_GET_BIT(state->machine->shared_pic_master.data.isr,
        VPIC_ISR_IRQ(0u));

    if (nmi_masked) {
        return state->machine->executor_cpu.data.flagNMI && !irq_pending && irq_active;
    }
    return !state->machine->executor_cpu.data.flagNMI && irq_pending && !irq_active;
}

static C_INT hardware_delivery_s3_vm86_priority(C_INT mask_nmi)
{
    vm86_delivery_state state;
    core_machine_pic_irq_source irq;
    core_machine_run_result result;
    type_unsigned_32 frame[9u] = { 0u };
    type_status status;
    C_INT failed = !vm86_delivery_prepare(&state, mask_nmi ? 0x20u : 0x02u);

    STD_MEMSET(&irq, 0, sizeof(irq));
    if (!failed) {
        state.machine->executor_cpu.data.flagNMI = TYPE_TRUE;
        state.machine->executor_cpu.data.flagMaskNMI = mask_nmi ? TYPE_TRUE : TYPE_FALSE;
        state.machine->shared_pic_master.data.icw2 = 0x20u;
        core_machine_pic_irq_source_bind(&irq, &state.machine->shared_pic_master,
            &state.machine->shared_pic_slave, 0u);
        core_machine_pic_irq_source_assert(&irq);
        core_machine_pic_irq_source_deassert(&irq);
        failed |= !hardware_delivery_s3_vm86_install_gate(&state, 0x20u) ||
            core_machine_memory_write(state.machine, 0x2000u,
                (const type_unsigned_8[]){ 0x90u }, 1u) != TYPE_STATUS_OK;
        status = failed ? TYPE_STATUS_FAULT : core_machine_run(state.machine,
            (core_machine_run_budget){ 8u, 0u }, &result);
        failed |= status != TYPE_STATUS_OK ||
            result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT ||
            state.machine->executor_cpu.data.eip != 0x0101u ||
            state.machine->executor_cpu.data.esp != VM86_STACK_TOP - 36u ||
            TYPE_GET_BIT(state.machine->executor_cpu.data.eflags, VCPU_EFLAGS_VM) ||
            TYPE_GET_BIT(state.machine->executor_cpu.data.eflags, VCPU_EFLAGS_IF) ||
            TYPE_GET_BIT(state.machine->executor_cpu.data.eflags, VCPU_EFLAGS_TF) ||
            !hardware_delivery_s3_vm86_pic_matches(&state, mask_nmi) ||
            core_machine_memory_read_physical(&state.machine->executor_memory,
                VM86_STACK_TOP - 36u, (type_virtual_address)frame,
                sizeof(frame)) != TYPE_STATUS_OK || frame[0] != 1u ||
            frame[1] != 0x0200u || frame[2] != (VCPU_EFLAGS_VM |
                VCPU_EFLAGS_IF);
    }
    core_machine_destroy(state.machine);
    return !failed;
}

C_INT main(C_VOID)
{
    if (!hardware_delivery_s3_real_priority() ||
        !hardware_delivery_s3_protected_priority() ||
        !hardware_delivery_s3_vm86_priority(0) ||
        !hardware_delivery_s3_vm86_priority(1)) {
        return 1;
    }
    STD_PRINTF("M5:T321:S3:HARDWARE-DELIVERY:OK\n");
    return 0;
}
