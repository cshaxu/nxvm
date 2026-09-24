#include "lib/types/types_interface.h"
#include <stdio.h>
#include "app-nxvm/devices/device_support.h"

#include "app-nxvm/devices/cpu.h"
#include "app-nxvm/devices/machine_interface.h"
#include "app-nxvm/devices/pic.h"
#include "support/core_machine_cpu_fixture.h"

typedef struct pic_phase_s2_state {
    core_machine *machine;
    core_machine_trace_event events[256u];
    lib_u32 count;
} pic_phase_s2_state;

static void pic_phase_s2_reset(void *opaque)
{
    pic_phase_s2_state *state = (pic_phase_s2_state *)opaque;

    if (state != LIB_NULL) {
        (void)test_core_machine_fixture_reset_real_mode(state->machine);
    }
}

static const core_machine_execution_provider pic_phase_s2_provider = {
    pic_phase_s2_reset, LIB_NULL
};

static void pic_phase_s2_trace(void *opaque,
    const core_machine_trace_event *event)
{
    pic_phase_s2_state *state = (pic_phase_s2_state *)opaque;

    if (state != LIB_NULL && event != LIB_NULL && state->count < 256u) {
        state->events[state->count++] = *event;
    }
}

static lib_i32 pic_phase_s2_has_acknowledgement_before_frame(
    const pic_phase_s2_state *state)
{
    lib_u32 index;
    lib_u32 acknowledgement = 0u;

    if (state == LIB_NULL) return 0;
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

lib_i32 main(void)
{
    const core_machine_config config = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
        .cpu_profile = CORE_MACHINE_CPU_PROFILE_80386,
        .fpu_profile = CORE_MACHINE_FPU_PROFILE_NONE
    };
    static const lib_u8 program[] = { 0x90u };
    static const lib_u8 handler[] = { 0xf4u };
    static const lib_u8 vector[] = { 0x00u, 0x01u, 0x00u, 0x00u };
    pic_phase_s2_state state;
    core_machine_pic_irq_source irq;
    core_machine_run_result result;
    const core_machine_trace_provider trace = { pic_phase_s2_trace, &state };
    lib_i32 failed = 0;

    lib_memory_set(&state, 0, sizeof(state));
    lib_memory_set(&irq, 0, sizeof(irq));
    if (!test_core_machine_fixture_create_bind_freeze_reset(&config,
            &pic_phase_s2_provider, &state, &state.machine)) return 1;
    failed |= core_machine_set_trace_provider(state.machine, &trace) !=
        LIB_STATUS_OK || core_machine_memory_write(state.machine, 0u, program,
            sizeof(program)) != LIB_STATUS_OK || core_machine_memory_write(
            state.machine, 0x80u, vector, sizeof(vector)) != LIB_STATUS_OK ||
        core_machine_memory_write(state.machine, 0x0100u, handler,
            sizeof(handler)) != LIB_STATUS_OK;
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
                &result) != LIB_STATUS_OK || result.reason !=
                CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT ||
            state.machine->executor_cpu.data.eip != 0x0101u ||
            CORE_MACHINE_BIT_IS_SET(state.machine->shared_pic_master.data.irr,
                VPIC_IRR_IRQ(0u)) || !CORE_MACHINE_BIT_IS_SET(
                state.machine->shared_pic_master.data.isr, VPIC_ISR_IRQ(0u)) ||
            !pic_phase_s2_has_acknowledgement_before_frame(&state) ||
            core_machine_reset(state.machine) != LIB_STATUS_OK ||
            state.machine->transaction.owner != CORE_MACHINE_TRANSACTION_OWNER_NONE ||
            state.machine->transaction.committed_count != 0u ||
            state.machine->transaction.cancelled_count != 0u;
    }
    core_machine_destroy(state.machine);
    if (failed) return 1;
    printf("M5:T456:S2:PIC-PHASE:OK\\n");
    return 0;
}
