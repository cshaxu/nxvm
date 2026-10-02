#include "support/pic_fixture.h"
#include "lib/types/types_interface.h"
#include <stdio.h>
#include "app-nxvm/devices/device_support.h"

#include "app-nxvm/devices/debug_interface.h"
#include "app-nxvm/devices/machine.h"
#include "app-nxvm/devices/machine_board_state.h"
#include "app-nxvm/devices/pic_bus.h"
#include "support/core_machine_board_fixture.h"

typedef struct pic_phase_s2_state {
    core_machine *machine;
    core_machine_trace_event events[256u];
    lib_u32 count;
    lib_status reset_status;
} pic_phase_s2_state;

static void pic_phase_s2_reset(void *opaque)
{
    pic_phase_s2_state *state = (pic_phase_s2_state *)opaque;
    const core_machine_debug_register_patch entry = {
        .mask = CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_CS) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_DS) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_ES) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_SS) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EIP)
    };

    if (state != LIB_NULL) {
        state->reset_status = core_machine_cpu_debug_patch_registers(
            state->machine->executor_cpu_execution, &entry);
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
    if (acknowledgement == 0u) return 0;
    for (index = acknowledgement; index < state->count; ++index) {
        const core_machine_trace_event *event = &state->events[index];

        if (event->type == CORE_MACHINE_TRACE_TRANSACTION_BEGIN &&
            (event->detail & 0xffu) == CORE_MACHINE_TRANSACTION_OWNER_CPU &&
            ((event->detail >> 8u) & 0xffu) ==
                CORE_MACHINE_TRANSACTION_CPU_MEMORY_WRITE) return 1;
    }
    return 0;
}

static lib_i32 pic_phase_s2_cascaded_bus(void)
{
    const core_machine_config config = {0};
    pic_phase_s2_state state = {0};
    const core_machine_trace_provider trace = { pic_phase_s2_trace, &state };
    core_machine_pic_irq_source irq = {0};
    lib_u8 vector = 0xffu;
    lib_i32 failed;

    if (core_machine_create(&config, &state.machine) != LIB_STATUS_OK) return 1;
    failed = core_machine_freeze_execution_providers(state.machine) != LIB_STATUS_OK ||
        core_machine_reset(state.machine) != LIB_STATUS_OK ||
        core_machine_set_trace_provider(state.machine, &trace) != LIB_STATUS_OK;
    if (!failed) {
        x86_pic *master = state.machine->board->shared_pic_master.device;
        x86_pic *slave = state.machine->board->shared_pic_slave.device;
        x86_pic_write_register(master, 0u, 0x11u);
        x86_pic_write_register(master, 1u, 0x20u);
        x86_pic_write_register(master, 1u, 0x04u);
        x86_pic_write_register(master, 1u, 0x01u);
        x86_pic_write_register(slave, 0u, 0x11u);
        x86_pic_write_register(slave, 1u, 0x28u);
        x86_pic_write_register(slave, 1u, 0x02u);
        x86_pic_write_register(slave, 1u, 0x01u);
        core_machine_pic_irq_source_bind(&irq, &state.machine->board->shared_pic_master,
            &state.machine->board->shared_pic_slave, 14u);
        core_machine_pic_irq_source_assert(&irq);
        core_machine_pic_irq_source_deassert(&irq);
        core_machine_pic_refresh(&state.machine->board->shared_pic_master,
            &state.machine->board->shared_pic_slave);
        failed |= !core_machine_cpu_bus.interrupt_pending(state.machine);
        failed |= core_machine_transaction_begin(&state.machine->transaction,
            CORE_MACHINE_TRANSACTION_OWNER_DMA,
            CORE_MACHINE_TRANSACTION_DMA_MEMORY_WRITE, 0u, 0u, 0u) != LIB_STATUS_OK;
        state.count = 0u;
        failed |= core_machine_cpu_bus.acknowledge_interrupt(state.machine, &vector) !=
                LIB_STATUS_INVALID_ARGUMENT || vector != 0xffu || state.count != 0u ||
            state.machine->transaction.owner != CORE_MACHINE_TRANSACTION_OWNER_DMA ||
            !core_machine_cpu_bus.interrupt_pending(state.machine) ||
            test_pic_read(&state.machine->board->shared_pic_master, 0x0bu) != 0u ||
            test_pic_read(&state.machine->board->shared_pic_slave, 0x0bu) != 0u;
        core_machine_transaction_cancel(&state.machine->transaction);
        state.count = 0u;
        failed |= core_machine_cpu_bus.acknowledge_interrupt(state.machine, &vector) !=
                LIB_STATUS_OK || vector != 0x2eu || state.count != 2u ||
            state.events[0].type != CORE_MACHINE_TRACE_TRANSACTION_BEGIN ||
            state.events[1].type != CORE_MACHINE_TRACE_TRANSACTION_COMMIT ||
            state.events[1].value != 0x2eu ||
            ((state.events[0].detail >> 8u) & 0xffu) !=
                CORE_MACHINE_TRANSACTION_CPU_INTERRUPT_ACKNOWLEDGE ||
            test_pic_read(&state.machine->board->shared_pic_master, 0x0bu) != 0x04u ||
            test_pic_read(&state.machine->board->shared_pic_slave, 0x0bu) != 0x40u ||
            state.machine->transaction.owner != CORE_MACHINE_TRANSACTION_OWNER_NONE;
    }
    core_machine_destroy(state.machine);
    return failed;
}

lib_i32 main(void)
{
    const core_machine_config config = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
        .cpu_profile = CORE_MACHINE_CPU_PROFILE_80386,
        .fpu_profile = X86_FPU_PROFILE_NONE
    };
    static const lib_u8 program[] = { 0x90u };
    static const lib_u8 handler[] = { 0xf4u };
    static const lib_u8 vector[] = { 0x00u, 0x01u, 0x00u, 0x00u };
    pic_phase_s2_state state;
    core_machine_pic_irq_source irq;
    core_machine_run_result result;
    core_machine_cpu_state cpu;
    const core_machine_debug_register_patch interrupt_entry = {
        .mask = CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_ESP) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EFLAGS),
        .values = {
            [CORE_MACHINE_DEBUG_ESP] = 0x00008000u,
            [CORE_MACHINE_DEBUG_EFLAGS] = 0x00000200u
        }
    };
    const core_machine_trace_provider trace = { pic_phase_s2_trace, &state };
    lib_i32 failed = 0;

    lib_memory_set(&state, 0, sizeof(state));
    lib_memory_set(&irq, 0, sizeof(irq));
    if (!test_core_machine_fixture_create_bind_freeze_reset(&config,
            &pic_phase_s2_provider, &state, &state.machine) ||
        state.reset_status != LIB_STATUS_OK) {
        core_machine_destroy(state.machine);
        return 1;
    }
    failed |= core_machine_set_trace_provider(state.machine, &trace) !=
        LIB_STATUS_OK || core_machine_memory_write(state.machine, 0u, program,
            sizeof(program)) != LIB_STATUS_OK || core_machine_memory_write(
            state.machine, 0x80u, vector, sizeof(vector)) != LIB_STATUS_OK ||
        core_machine_memory_write(state.machine, 0x0100u, handler,
            sizeof(handler)) != LIB_STATUS_OK;
    if (!failed) {
        failed |= core_machine_debug_patch_registers(state.machine,
            &interrupt_entry) != LIB_STATUS_OK;
        test_pic_program_vector(&state.machine->board->shared_pic_master, 0x20u);
        core_machine_pic_irq_source_bind(&irq, &state.machine->board->shared_pic_master,
            &state.machine->board->shared_pic_slave, 0u);
        core_machine_pic_irq_source_assert(&irq);
        core_machine_pic_irq_source_deassert(&irq);
        failed |= !core_machine_pic_scan_interrupt(
            &state.machine->board->shared_pic_master, &state.machine->board->shared_pic_slave) ||
            core_machine_run(state.machine, (core_machine_run_budget){ 8u, 0u },
                &result) != LIB_STATUS_OK || result.reason !=
                CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT ||
            core_machine_get_cpu_state(state.machine, &cpu) != LIB_STATUS_OK ||
            cpu.eip != 0x0101u ||
            CORE_MACHINE_BIT_IS_SET(test_pic_read(&state.machine->board->shared_pic_master, 0x0au),
                VPIC_IRR_IRQ(0u)) || !CORE_MACHINE_BIT_IS_SET(
                test_pic_read(&state.machine->board->shared_pic_master, 0x0bu), VPIC_ISR_IRQ(0u)) ||
            !pic_phase_s2_has_acknowledgement_before_frame(&state) ||
            core_machine_reset(state.machine) != LIB_STATUS_OK ||
            state.reset_status != LIB_STATUS_OK ||
            state.machine->transaction.owner != CORE_MACHINE_TRANSACTION_OWNER_NONE ||
            state.machine->transaction.committed_count != 0u ||
            state.machine->transaction.cancelled_count != 0u;
    }
    core_machine_destroy(state.machine);
    failed |= pic_phase_s2_cascaded_bus();
    if (failed) return 1;
    printf("M5:T456:S2:PIC-PHASE:OK\\n");
    return 0;
}
