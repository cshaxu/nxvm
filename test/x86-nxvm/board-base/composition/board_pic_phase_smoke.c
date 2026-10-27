#include "lib/types/file.h"
#include "../pic_fixture.h"
#include "x86-core/machine/machine_interface.h"
#include "x86-core/machine/attachment_interface.h"
#include "lib/types/types_interface.h"

#include "x86-core/machine/debug_interface.h"
#include "x86-core/machine/trace_interface.h"
#include "x86-nxvm/board-base/pic_bus_interface.h"

typedef struct pic_phase_state {
    x86_core_machine *machine;
    x86_nxvm_board_pic_bus *master;
    x86_nxvm_board_pic_bus *slave;
    x86_core_machine_trace_event events[256u];
    lib_u32 count;
    lib_status reset_status;
} pic_phase_state;


static lib_bool pic_phase_pending(void *opaque)
{
    pic_phase_state *state = opaque;
    return x86_nxvm_board_pic_scan_interrupt(state->master, state->slave) ?
        LIB_TRUE : LIB_FALSE;
}

static lib_u8 pic_phase_acknowledge(void *opaque)
{
    pic_phase_state *state = opaque;
    return x86_nxvm_board_pic_get_interrupt(state->master, state->slave);
}

static void pic_phase_reset_pic(void *opaque)
{
    pic_phase_state *state = opaque;
    x86_nxvm_board_pic_reset(state->master, state->slave);
}

static void pic_phase_finalize(void *opaque)
{
    pic_phase_state *state = opaque;
    x86_nxvm_board_pic_finalize(state->master, state->slave);
}

static lib_status pic_phase_create(const x86_core_machine_executor_config *config,
    pic_phase_state *state)
{
    const x86_core_machine_attachment attachment = {
        .interrupt_pending = pic_phase_pending,
        .interrupt_acknowledge = pic_phase_acknowledge,
        .reset_devices = pic_phase_reset_pic,
        .finalize_devices = pic_phase_finalize,
        .context = state
    };
    lib_status status = x86_core_machine_neutral_create(config, &state->machine);
    if (status == LIB_STATUS_OK)
        status = x86_nxvm_board_pic_initialize(&state->master, &state->slave,
            state->machine, X86_NXVM_BOARD_PIC_TOPOLOGY_CASCADED);
    if (status == LIB_STATUS_OK)
        status = x86_core_machine_bind_attachment(state->machine, &attachment);
    if (status != LIB_STATUS_OK) {
        x86_nxvm_board_pic_finalize(state->master, state->slave);
        x86_core_machine_destroy(state->machine);
        state->machine = LIB_NULL;
    }
    return status;
}

static void pic_phase_reset(void *opaque)
{
    pic_phase_state *state = (pic_phase_state *)opaque;
    const x86_core_cpu_debug_register_patch entry = {
        .mask = X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_CS) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_DS) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_ES) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_SS) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_EIP)
    };

    if (state != LIB_NULL) {
        state->reset_status = x86_core_machine_debug_patch_registers(
            state->machine, &entry);
    }
}

static void pic_phase_trace(void *opaque,
    const x86_core_machine_trace_event *event)
{
    pic_phase_state *state = (pic_phase_state *)opaque;

    if (state != LIB_NULL && event != LIB_NULL && state->count < 256u) {
        state->events[state->count++] = *event;
    }
}

static lib_i32 pic_phase_has_acknowledgement_before_frame(
    const pic_phase_state *state)
{
    lib_u32 index;
    lib_u32 acknowledgement = 0u;

    if (state == LIB_NULL) return 0;
    for (index = 0u; index + 1u < state->count; ++index) {
        const x86_core_machine_trace_event *begin = &state->events[index];
        const x86_core_machine_trace_event *commit = &state->events[index + 1u];

        if (begin->type == X86_CORE_MACHINE_TRACE_TRANSACTION_BEGIN &&
            commit->type == X86_CORE_MACHINE_TRACE_TRANSACTION_COMMIT &&
            (begin->detail & 0xffu) == X86_CORE_MACHINE_TRANSACTION_OWNER_CPU &&
            ((begin->detail >> 8u) & 0xffu) ==
                X86_CORE_MACHINE_TRANSACTION_CPU_INTERRUPT_ACKNOWLEDGE &&
            begin->detail == commit->detail) {
            acknowledgement = index + 2u;
            break;
        }
    }
    if (acknowledgement == 0u) return 0;
    for (index = acknowledgement; index < state->count; ++index) {
        const x86_core_machine_trace_event *event = &state->events[index];

        if (event->type == X86_CORE_MACHINE_TRACE_TRANSACTION_BEGIN &&
            (event->detail & 0xffu) == X86_CORE_MACHINE_TRANSACTION_OWNER_CPU &&
            ((event->detail >> 8u) & 0xffu) ==
                X86_CORE_MACHINE_TRANSACTION_CPU_MEMORY_WRITE) return 1;
    }
    return 0;
}

static lib_i32 pic_phase_cascaded_bus(void)
{
    const x86_core_machine_executor_config config = { .memory_bytes = 16u * 1024u * 1024u, .cpu_profile = X86_CORE_CPU_PROFILE_80386 };
    pic_phase_state state = {0};
    const x86_core_machine_trace_provider trace = { pic_phase_trace, &state };
    x86_nxvm_board_pic_irq_source *irq = LIB_NULL;
    const lib_u8 vector[] = {0x00u, 0x01u, 0x00u, 0x00u};
    const lib_u8 handler = 0xf4u;
    const x86_core_cpu_debug_register_patch entry = {
        .mask = X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_CS) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_SS) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_EIP) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_ESP) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_EFLAGS),
        .values = {[X86_CORE_CPU_DEBUG_ESP] = 0x8000u,
            [X86_CORE_CPU_DEBUG_EFLAGS] = 0x202u}
    };
    x86_core_machine_run_result run;
    x86_core_cpu_state cpu;
    lib_i32 failed;

    if (pic_phase_create(&config, &state) != LIB_STATUS_OK) return 1;
    failed = x86_core_machine_freeze_execution_providers(state.machine) != LIB_STATUS_OK ||
        x86_core_machine_reset(state.machine) != LIB_STATUS_OK ||
        x86_core_machine_debug_patch_registers(state.machine, &entry) != LIB_STATUS_OK ||
        x86_core_machine_memory_write(state.machine, 0xb8u, vector, sizeof(vector)) != LIB_STATUS_OK ||
        x86_core_machine_memory_write(state.machine, 0x100u, &handler, 1u) != LIB_STATUS_OK ||
        x86_core_machine_set_trace_provider(state.machine, &trace) != LIB_STATUS_OK;
    if (!failed) {
        x86_nxvm_board_pic_bus *master = state.master;
        x86_nxvm_board_pic_bus *slave = state.slave;
        x86_nxvm_board_pic_write_register(master, 0u, 0x11u);
        x86_nxvm_board_pic_write_register(master, 1u, 0x20u);
        x86_nxvm_board_pic_write_register(master, 1u, 0x04u);
        x86_nxvm_board_pic_write_register(master, 1u, 0x01u);
        x86_nxvm_board_pic_write_register(slave, 0u, 0x11u);
        x86_nxvm_board_pic_write_register(slave, 1u, 0x28u);
        x86_nxvm_board_pic_write_register(slave, 1u, 0x02u);
        x86_nxvm_board_pic_write_register(slave, 1u, 0x01u);
        test_pic_bind_source(&irq, state.master,
            state.slave, 14u);
        x86_nxvm_board_pic_irq_source_assert(irq);
        x86_nxvm_board_pic_refresh(state.master,
            state.slave);
        failed |= !x86_nxvm_board_pic_scan_interrupt(state.master, state.slave) ||
            test_pic_read(state.master, 0x0bu) != 0u ||
            test_pic_read(state.slave, 0x0bu) != 0u;
        state.count = 0u;
        failed |= x86_core_machine_run(state.machine,
                (x86_core_machine_run_budget){8u, 0u}, &run) != LIB_STATUS_OK ||
            run.reason != X86_CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT ||
            x86_core_machine_get_cpu_state(state.machine, &cpu) != LIB_STATUS_OK ||
            cpu.eip != 0x101u ||
            test_pic_read(state.master, 0x0bu) != 0x04u ||
            test_pic_read(state.slave, 0x0bu) != 0x40u ||
            !pic_phase_has_acknowledgement_before_frame(&state);
        x86_nxvm_board_pic_irq_source_deassert(irq);
    }
    x86_core_machine_destroy(state.machine);
    return failed;
}

lib_i32 main(void)
{
    const x86_core_machine_executor_config config = {
        .memory_bytes = (2u * 1024u * 1024u),
        .cpu_profile = X86_CORE_CPU_PROFILE_80386,
        .fpu_profile = X86_FPU_PROFILE_NONE
    };
    static const lib_u8 program[] = { 0x90u };
    static const lib_u8 handler[] = { 0xf4u };
    static const lib_u8 vector[] = { 0x00u, 0x01u, 0x00u, 0x00u };
    pic_phase_state state;
    x86_nxvm_board_pic_irq_source *irq = LIB_NULL;
    x86_core_machine_run_result result;
    x86_core_cpu_state cpu;
    const x86_core_cpu_debug_register_patch interrupt_entry = {
        .mask = X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_ESP) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_EFLAGS),
        .values = {
            [X86_CORE_CPU_DEBUG_ESP] = 0x00008000u,
            [X86_CORE_CPU_DEBUG_EFLAGS] = 0x00000200u
        }
    };
    const x86_core_machine_trace_provider trace = { pic_phase_trace, &state };
    lib_i32 failed = 0;

    lib_memory_set(&state, 0, sizeof(state));
    lib_memory_set(&irq, 0, sizeof(irq));
    if (pic_phase_create(&config, &state) != LIB_STATUS_OK ||
        x86_core_machine_freeze_execution_providers(state.machine) != LIB_STATUS_OK ||
        x86_core_machine_reset(state.machine) != LIB_STATUS_OK ||
        state.reset_status != LIB_STATUS_OK) {
        x86_core_machine_destroy(state.machine);
        return 1;
    }
    pic_phase_reset(&state);
    failed |= state.reset_status != LIB_STATUS_OK;
    failed |= x86_core_machine_set_trace_provider(state.machine, &trace) !=
        LIB_STATUS_OK || x86_core_machine_memory_write(state.machine, 0u, program,
            sizeof(program)) != LIB_STATUS_OK || x86_core_machine_memory_write(
            state.machine, 0x80u, vector, sizeof(vector)) != LIB_STATUS_OK ||
        x86_core_machine_memory_write(state.machine, 0x0100u, handler,
            sizeof(handler)) != LIB_STATUS_OK;
    if (!failed) {
        failed |= x86_core_machine_debug_patch_registers(state.machine,
            &interrupt_entry) != LIB_STATUS_OK;
        test_pic_program_vector(state.master, 0x20u);
        test_pic_bind_source(&irq, state.master,
            state.slave, 0u);
        x86_nxvm_board_pic_irq_source_assert(irq);
        failed |= !x86_nxvm_board_pic_scan_interrupt(
            state.master, state.slave) ||
            x86_core_machine_run(state.machine, (x86_core_machine_run_budget){ 8u, 0u },
                &result) != LIB_STATUS_OK || result.reason !=
                X86_CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT ||
            x86_core_machine_get_cpu_state(state.machine, &cpu) != LIB_STATUS_OK ||
            cpu.eip != 0x0101u ||
            (((test_pic_read(state.master, 0x0au)) & (VPIC_IRR_IRQ(0u))) != 0u) || !(((test_pic_read(state.master, 0x0bu)) & (VPIC_ISR_IRQ(0u))) != 0u) ||
            !pic_phase_has_acknowledgement_before_frame(&state);
        x86_nxvm_board_pic_irq_source_deassert(irq);
        failed |= x86_core_machine_reset(state.machine) != LIB_STATUS_OK;
        if (!failed) {
            pic_phase_reset(&state);
            failed |= state.reset_status != LIB_STATUS_OK;
        }
    }
    x86_core_machine_destroy(state.machine);
    failed |= pic_phase_cascaded_bus();
    if (failed) return 1;
    lib_c_printf("PIC-PHASE:OK\\n");
    return 0;
}
