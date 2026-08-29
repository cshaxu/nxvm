#include "type.h"

#include "core/machine/cpu.h"
#include "core/machine/machine_interface.h"
#include "core/machine/pic.h"
#include "../support/core_machine_cpu_fixture.h"

typedef struct pic_phase_s2_state {
    core_machine *machine;
    core_machine_trace_event events[256u];
    type_unsigned_32 count;
} pic_phase_s2_state;

static C_VOID pic_phase_s2_reset(C_VOID *opaque)
{
    pic_phase_s2_state *state = (pic_phase_s2_state *)opaque;

    if (state != STD_NULL) {
        (C_VOID)test_core_machine_fixture_reset_real_mode(state->machine);
    }
}

static const core_machine_execution_provider pic_phase_s2_provider = {
    pic_phase_s2_reset, STD_NULL
};

static C_VOID pic_phase_s2_trace(C_VOID *opaque,
    const core_machine_trace_event *event)
{
    pic_phase_s2_state *state = (pic_phase_s2_state *)opaque;

    if (state != STD_NULL && event != STD_NULL && state->count < 256u) {
        state->events[state->count++] = *event;
    }
}

static C_INT pic_phase_s2_has_acknowledgement_before_frame(
    const pic_phase_s2_state *state)
{
    type_unsigned_32 index;
    type_unsigned_32 acknowledgement = 0u;

    if (state == STD_NULL) return 0;
    for (index = 0u; index + 1u < state->count; ++index) {
        const core_machine_trace_event *begin = &state->events[index];
        const core_machine_trace_event *commit = &state->events[index + 1u];

        if (begin->type == CORE_MACHINE_TRACE_TRANSACTION_BEGIN &&
            commit->type == CORE_MACHINE_TRACE_TRANSACTION_COMMIT &&
            (begin->detail & 0xffu) == CORE_MACHINE_TRANSACTION_OWNER_CPU &&
            ((begin->detail >> 8u) & 0xffu) ==
                CORE_MACHINE_TRANSACTION_CPU_INTERRUPT_ACKNOWLEDGE &&
            begin->detail == commit->detail) {
            acknowledgement = index + 2u;
            break;
        }
    }
    for (index = acknowledgement; index < state->count; ++index) {
        const core_machine_trace_event *event = &state->events[index];

        if (event->type == CORE_MACHINE_TRACE_TRANSACTION_BEGIN &&
            (event->detail & 0xffu) == CORE_MACHINE_TRANSACTION_OWNER_CPU &&
            ((event->detail >> 8u) & 0xffu) ==
                CORE_MACHINE_TRANSACTION_CPU_MEMORY_WRITE) return 1;
    }
    return 0;
}

C_INT main(C_VOID)
{
    const core_machine_config config = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
        .cpu_profile = CORE_MACHINE_CPU_PROFILE_80386,
        .fpu_profile = CORE_MACHINE_FPU_PROFILE_NONE
    };
    static const type_unsigned_8 program[] = { 0x90u };
    static const type_unsigned_8 handler[] = { 0xf4u };
    static const type_unsigned_8 vector[] = { 0x00u, 0x01u, 0x00u, 0x00u };
    pic_phase_s2_state state;
    core_machine_pic_irq_source irq;
    core_machine_run_result result;
    const core_machine_trace_provider trace = { pic_phase_s2_trace, &state };
    C_INT failed = 0;

    STD_MEMSET(&state, 0, sizeof(state));
    STD_MEMSET(&irq, 0, sizeof(irq));
    if (!test_core_machine_fixture_create_bind_freeze_reset(&config,
            &pic_phase_s2_provider, &state, &state.machine)) return 1;
    failed |= core_machine_set_trace_provider(state.machine, &trace) !=
        TYPE_STATUS_OK || core_machine_memory_write(state.machine, 0u, program,
            sizeof(program)) != TYPE_STATUS_OK || core_machine_memory_write(
            state.machine, 0x80u, vector, sizeof(vector)) != TYPE_STATUS_OK ||
        core_machine_memory_write(state.machine, 0x0100u, handler,
            sizeof(handler)) != TYPE_STATUS_OK;
    if (!failed) {
        state.machine->executor_cpu.data.esp = 0x00008000u;
        state.machine->executor_cpu.data.eflags = VCPU_EFLAGS_IF;
        state.machine->shared_pic_master.data.icw2 = 0x20u;
        core_machine_pic_irq_source_bind(&irq, &state.machine->shared_pic_master,
            &state.machine->shared_pic_slave, 0u);
        core_machine_pic_irq_source_assert(&irq);
        core_machine_pic_irq_source_deassert(&irq);
        failed |= !core_machine_pic_scan_interrupt(
            &state.machine->shared_pic_master, &state.machine->shared_pic_slave) ||
            core_machine_run(state.machine, (core_machine_run_budget){ 8u, 0u },
                &result) != TYPE_STATUS_OK || result.reason !=
                CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT ||
            state.machine->executor_cpu.data.eip != 0x0101u ||
            TYPE_GET_BIT(state.machine->shared_pic_master.data.irr,
                VPIC_IRR_IRQ(0u)) || !TYPE_GET_BIT(
                state.machine->shared_pic_master.data.isr, VPIC_ISR_IRQ(0u)) ||
            !pic_phase_s2_has_acknowledgement_before_frame(&state) ||
            core_machine_reset(state.machine) != TYPE_STATUS_OK ||
            state.machine->transaction.owner != CORE_MACHINE_TRANSACTION_OWNER_NONE ||
            state.machine->transaction.committed_count != 0u ||
            state.machine->transaction.cancelled_count != 0u;
    }
    core_machine_destroy(state.machine);
    if (failed) return 1;
    STD_PRINTF("M5:T456:S2:PIC-PHASE:OK\\n");
    return 0;
}
