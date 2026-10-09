#include "lib/types/types_interface.h"


#include "core/x86/debug_interface.h"
#include "core/x86/machine.h"

static lib_status core_machine_debug_require_boundary(
    const core_machine *machine)
{
    core_machine_lifecycle lifecycle;
    lib_status status;

    status = core_machine_get_lifecycle(machine, &lifecycle);
    if (status != LIB_STATUS_OK) {
        return status;
    }
    return lifecycle == CORE_MACHINE_PAUSED || lifecycle == CORE_MACHINE_STOPPED ||
           lifecycle == CORE_MACHINE_FAULTED ? LIB_STATUS_OK :
                                                LIB_STATUS_INVALID_STATE;
}

lib_status core_machine_debug_read_cpu(
    const core_machine *machine,
    core_machine_cpu_state *out_state)
{
    lib_status status = core_machine_debug_require_boundary(machine);

    return status == LIB_STATUS_OK ?
               core_machine_get_cpu_state(machine, out_state) : status;
}

lib_status core_machine_debug_capture_cpu_snapshot(const core_machine *machine,
    core_machine_cpu_snapshot_point point,
    core_machine_debug_cpu_snapshot *out_snapshot)
{
    lib_status status = core_machine_debug_require_boundary(machine);

    if (out_snapshot == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    return status == LIB_STATUS_OK ?
        core_machine_cpu_debug_capture_snapshot(machine->executor_cpu_execution,
            point, out_snapshot) : status;
}

lib_status core_machine_debug_read_memory(
    const core_machine *machine,
    lib_u32 physical,
    void *out_data,
    lib_size size)
{
    lib_status status = core_machine_debug_require_boundary(machine);

    return status == LIB_STATUS_OK ?
               core_machine_memory_read(machine, physical, out_data, size) :
               status;
}

lib_status core_machine_debug_step(
    core_machine *machine,
    core_machine_run_result *out_result)
{
    const core_machine_run_budget budget = { 1u, 0u };
    lib_status status = core_machine_debug_require_boundary(machine);

    return status == LIB_STATUS_OK ?
               core_machine_run(machine, budget, out_result) : status;
}

lib_status core_machine_debug_continue(
    core_machine *machine,
    core_machine_run_budget budget,
    core_machine_run_result *out_result)
{
    lib_status status = core_machine_debug_require_boundary(machine);

    return status == LIB_STATUS_OK ?
               core_machine_run(machine, budget, out_result) : status;
}

lib_status core_machine_debug_capture_instruction_observation(const core_machine *machine,
    core_machine_debug_instruction_observation *out_observation)
{
    lib_status status = core_machine_debug_require_boundary(machine);

    return status == LIB_STATUS_OK ?
        core_machine_cpu_debug_capture_instruction(machine->executor_cpu_execution,
            out_observation) : status;
}

lib_status core_machine_debug_read_register(const core_machine *machine,
    core_machine_debug_register register_id, lib_u32 *out_value)
{
    lib_status status = core_machine_debug_require_boundary(machine);

    return status == LIB_STATUS_OK ?
        core_machine_cpu_debug_read_register(machine->executor_cpu_execution,
            register_id, out_value) : status;
}

lib_status core_machine_debug_write_register(core_machine *machine,
    core_machine_debug_register register_id, lib_u32 value)
{
    core_machine_debug_register_patch patch = {0};

    if (register_id >= CORE_MACHINE_DEBUG_REGISTER_COUNT)
        return LIB_STATUS_INVALID_ARGUMENT;
    patch.mask = CORE_MACHINE_DEBUG_REGISTER_MASK(register_id);
    patch.values[register_id] = value;
    return core_machine_debug_patch_registers(machine, &patch);
}

lib_status core_machine_debug_patch_registers(core_machine *machine,
    const core_machine_debug_register_patch *patch)
{
    lib_status status = core_machine_debug_require_boundary(machine);

    return status == LIB_STATUS_OK ?
        core_machine_cpu_debug_patch_registers(machine->executor_cpu_execution,
            patch) : status;
}

lib_status core_machine_debug_get_code_default_size(const core_machine *machine,
    lib_i32 *out_default_size)
{
    lib_status status = core_machine_debug_require_boundary(machine);
    if (status != LIB_STATUS_OK || out_default_size == LIB_NULL) return
        status == LIB_STATUS_OK ? LIB_STATUS_INVALID_ARGUMENT : status;
    *out_default_size = core_machine_cpu_get_code_default_size(
        machine->executor_cpu_execution);
    return LIB_STATUS_OK;
}

lib_status core_machine_debug_get_code_base(const core_machine *machine,
    lib_u32 *out_base)
{
    lib_status status = core_machine_debug_require_boundary(machine);
    if (status != LIB_STATUS_OK || out_base == LIB_NULL) return
        status == LIB_STATUS_OK ? LIB_STATUS_INVALID_ARGUMENT : status;
    *out_base = core_machine_cpu_get_code_base(machine->executor_cpu_execution);
    return LIB_STATUS_OK;
}

lib_status core_machine_debug_read_linear(core_machine *machine, lib_u32 address,
    void *out_data, lib_u8 size)
{
    lib_status status = core_machine_debug_require_boundary(machine);
    if (status != LIB_STATUS_OK) return status;
    return core_machine_cpu_read_linear(machine->executor_cpu_execution, address,
        out_data, size) == 0 ? LIB_STATUS_OK : LIB_STATUS_INVALID_STATE;
}

lib_status core_machine_debug_write_linear(core_machine *machine, lib_u32 address,
    const void *data, lib_u8 size)
{
    lib_status status = core_machine_debug_require_boundary(machine);
    if (status != LIB_STATUS_OK) return status;
    return core_machine_cpu_write_linear(machine->executor_cpu_execution, address,
        data, size) == 0 ? LIB_STATUS_OK : LIB_STATUS_INVALID_STATE;
}

lib_status core_machine_debug_read_real(core_machine *machine, lib_u16 segment,
    lib_u16 offset, void *out_data, lib_size size)
{
    lib_status status = core_machine_debug_require_boundary(machine);
    if (status != LIB_STATUS_OK) return status;
    return core_machine_memory_read_real_from(&machine->executor_memory, segment,
        offset, out_data, size);
}

lib_status core_machine_debug_write_real(core_machine *machine, lib_u16 segment,
    lib_u16 offset, const void *data, lib_size size)
{
    lib_status status = core_machine_debug_require_boundary(machine);
    if (status != LIB_STATUS_OK) return status;
    return core_machine_memory_write_real_to(&machine->executor_memory, segment,
        offset, data, size);
}

lib_status core_machine_debug_read_port(core_machine *machine, lib_u16 port,
    lib_u32 *out_value)
{
    lib_status status = core_machine_debug_require_boundary(machine);
    if (status != LIB_STATUS_OK || out_value == LIB_NULL) return
        status == LIB_STATUS_OK ? LIB_STATUS_INVALID_ARGUMENT : status;
    status = core_machine_port_execute_read(&machine->executor_port, port,
        machine->elapsed_ticks);
    if (status != LIB_STATUS_OK) return status;
    *out_value = machine->executor_port.data.ioDWord;
    return LIB_STATUS_OK;
}

lib_status core_machine_debug_write_port(core_machine *machine, lib_u16 port,
    lib_u32 value)
{
    lib_status status = core_machine_debug_require_boundary(machine);
    if (status != LIB_STATUS_OK) return status;
    {
        lib_u32 prior_value = machine->executor_port.data.ioDWord;

        machine->executor_port.data.ioDWord = value;
        status = core_machine_port_execute_write(&machine->executor_port, port);
        if (status != LIB_STATUS_OK) machine->executor_port.data.ioDWord =
            prior_value;
    }
    return status;
}

lib_status core_machine_debug_set_watchpoint(core_machine *machine,
    core_machine_debug_watch_kind kind, lib_u32 address)
{
    lib_status status = core_machine_debug_require_boundary(machine);
    if (status != LIB_STATUS_OK || kind > CORE_MACHINE_DEBUG_WATCH_EXECUTE)
        return status != LIB_STATUS_OK ? status : LIB_STATUS_INVALID_ARGUMENT;
    core_machine_cpu_set_watchpoint(machine->executor_cpu_execution,
        (core_machine_cpu_watchpoint)kind, address);
    return LIB_STATUS_OK;
}

lib_status core_machine_debug_clear_watchpoint(core_machine *machine,
    core_machine_debug_watch_kind kind)
{
    lib_status status = core_machine_debug_require_boundary(machine);
    if (status != LIB_STATUS_OK || kind > CORE_MACHINE_DEBUG_WATCH_EXECUTE)
        return status != LIB_STATUS_OK ? status : LIB_STATUS_INVALID_ARGUMENT;
    core_machine_cpu_clear_watchpoint(machine->executor_cpu_execution,
        (core_machine_cpu_watchpoint)kind);
    return LIB_STATUS_OK;
}

lib_status core_machine_debug_get_watchpoint(core_machine *machine,
    core_machine_debug_watch_kind kind, lib_u8 *out_enabled,
    lib_u32 *out_address)
{
    lib_status status = core_machine_debug_require_boundary(machine);

    if (status != LIB_STATUS_OK || kind > CORE_MACHINE_DEBUG_WATCH_EXECUTE ||
        out_enabled == LIB_NULL || out_address == LIB_NULL)
        return status != LIB_STATUS_OK ? status : LIB_STATUS_INVALID_ARGUMENT;
    core_machine_cpu_get_watchpoint(machine->executor_cpu_execution,
        (core_machine_cpu_watchpoint)kind, out_enabled, out_address);
    return LIB_STATUS_OK;
}

static void core_machine_cpu_diagnostic_record_instruction(void *opaque,
    const core_machine_cpu_instruction_observation *observation)
{
    core_machine *machine = (core_machine *)opaque;
#if CORE_MACHINE_RUNTIME_TRACE_ENABLED
    core_machine_cpu_diagnostic_state *state;
#endif

    if (machine == LIB_NULL) return;
#if CORE_MACHINE_RUNTIME_TRACE_ENABLED
    state = &machine->cpu_diagnostic;
    /* Recent instruction history is a development diagnostic.  The retained
     * runtime debugger reads the current machine state through its explicit
     * copied operations, while faults retain their own snapshots below. */
    state->snapshot.recent[state->next_index] = observation->point;
    state->next_index = (state->next_index + 1u) % CORE_MACHINE_CPU_DIAGNOSTIC_WINDOW_CAPACITY;
    if (state->snapshot.recent_count < CORE_MACHINE_CPU_DIAGNOSTIC_WINDOW_CAPACITY) {
        ++state->snapshot.recent_count;
    }
#endif
    /* The copied observation is needed either by an explicit subscriber or
     * by the verified-physical scheduler: the latter freezes the pre-retire
     * identity used by its qualification key.  Ordinary non-physical runs
     * keep the zero-overhead path. */
    if (machine->retirement_observation.provider.callback != LIB_NULL ||
        machine->retirement_time_contract == CORE_MACHINE_RETIREMENT_TIME_PHYSICAL) {
        core_machine_retirement_observation_capture_instruction(machine, observation);
    }
}

static void core_machine_cpu_diagnostic_record_fault(void *opaque,
    const core_machine_cpu_fault_snapshot *snapshot)
{
    core_machine *machine = (core_machine *)opaque;
    core_machine_cpu_fault_snapshot *fault;

    if (machine == LIB_NULL || snapshot == LIB_NULL) return;
    fault = &machine->cpu_diagnostic.snapshot.first_fault;
    if (fault->valid) return;
    *fault = *snapshot;
    (void)core_machine_report_fault(machine, fault->exception_mask);
}

static void core_machine_cpu_diagnostic_record_delivered_exception(
    void *opaque, const core_machine_cpu_fault_snapshot *snapshot)
{
    core_machine *machine = (core_machine *)opaque;
    core_machine_cpu_fault_snapshot *exception;

    if (machine == LIB_NULL || snapshot == LIB_NULL) return;
    exception = &machine->cpu_diagnostic.snapshot.first_delivered_exception;
    if (!exception->valid) {
        *exception = *snapshot;
    }
    exception = &machine->cpu_diagnostic.snapshot.last_delivered_exception;
    *exception = *snapshot;
    machine->cpu_diagnostic.snapshot.delivered_exception_count++;
}

const core_machine_cpu_execution_diagnostic_provider
    core_machine_cpu_diagnostic_provider = {
        core_machine_cpu_diagnostic_record_instruction,
        core_machine_cpu_diagnostic_record_delivered_exception,
        core_machine_cpu_diagnostic_record_fault
    };

const core_machine_cpu_execution_diagnostic_provider
    core_machine_cpu_fault_diagnostic_provider = {
        LIB_NULL,
        core_machine_cpu_diagnostic_record_delivered_exception,
        core_machine_cpu_diagnostic_record_fault
    };

static void core_machine_cpu_diagnostic_ordered_copy(
    const core_machine_cpu_diagnostic_state *state,
    core_machine_cpu_diagnostic *out_diagnostic)
{
    lib_size index;
    lib_size first;

    *out_diagnostic = state->snapshot;
    if (state->snapshot.recent_count < CORE_MACHINE_CPU_DIAGNOSTIC_WINDOW_CAPACITY ||
        state->next_index == 0u) return;
    first = state->next_index;
    for (index = 0u; index < CORE_MACHINE_CPU_DIAGNOSTIC_WINDOW_CAPACITY; ++index) {
        out_diagnostic->recent[index] = state->snapshot.recent[
            (first + index) % CORE_MACHINE_CPU_DIAGNOSTIC_WINDOW_CAPACITY];
    }
}

void core_machine_cpu_diagnostic_capture(const core_machine *machine,
    core_machine_cpu_diagnostic *out_diagnostic)
{
    if (machine != LIB_NULL && out_diagnostic != LIB_NULL) {
        core_machine_cpu_diagnostic_ordered_copy(&machine->cpu_diagnostic,
            out_diagnostic);
    }
}

void core_machine_cpu_diagnostic_initialize(core_machine *machine)
{
    if (machine != LIB_NULL) {
        lib_memory_set(&machine->cpu_diagnostic, 0, sizeof(machine->cpu_diagnostic));
    }
}

void core_machine_cpu_diagnostic_reset(core_machine *machine)
{
    core_machine_cpu_diagnostic_initialize(machine);
}
