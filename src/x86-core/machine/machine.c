#include "lib/types/types_interface.h"

#include "x86-core/machine/machine.h"
#include "x86-core/cpu/cpu_interface.h"

#define X86_CORE_MACHINE_L1_COMPATIBILITY_MAXIMUM_STEPS 16u

lib_u32 x86_core_machine_linear_pc(const x86_core_machine *machine)
{
    return x86_core_cpu_linear_pc(machine->executor_cpu_execution);
}

static lib_i32 x86_core_machine_retirement_qualification_contains(
    const x86_core_machine *machine);


/* Both immediate and externally delayed successful retirements meet here.
 * CPU timing selection is complete before this seam; board-cycle time has
 * already been added by the caller and never enters cpu_timing.c. */
static lib_i32 x86_core_machine_publish_successful_retirement(x86_core_machine *machine)
{
    if (machine == LIB_NULL) return 0;
    x86_core_machine_retirement_observation_publish(machine,
        machine->cpu_retirement_source_ticks);
    return machine->retirement_time_contract != X86_CORE_MACHINE_RETIREMENT_TIME_PHYSICAL ||
        (machine->time_axis.kind == X86_CORE_MACHINE_TIME_AXIS_VERIFIED_PHYSICAL &&
         !x86_core_cpu_capture_timing(machine->executor_cpu_execution).
             source_timing_unallocated &&
         x86_core_machine_retirement_qualification_contains(machine));
}

static void x86_core_machine_begin_retirement_wait(x86_core_machine *machine,
    lib_u64 wait_ticks, lib_u64 completion_ticks, lib_u64 source_ticks,
    lib_bool retires)
{
    machine->cpu_retirement_wait_pending = LIB_TRUE;
    machine->cpu_retirement_wait_ticks = wait_ticks;
    machine->cpu_retirement_completion_ticks = completion_ticks;
    machine->cpu_retirement_source_ticks = source_ticks;
    machine->cpu_retirement_wait_retires = retires;
}

/* A board may consume shutdown through its one owned reset.  Otherwise Core
 * returns the established shutdown boundary without publishing more time. */
static lib_bool x86_core_machine_finish_shutdown(x86_core_machine *machine,
    x86_core_machine_run_result *result)
{
    if (!x86_core_cpu_is_system_waiting(machine->executor_cpu_execution) &&
        machine->attachment.shutdown_reset != LIB_NULL &&
        machine->attachment.shutdown_reset(machine->attachment.context)) {
        return LIB_FALSE;
    }
    machine->lifecycle = X86_CORE_MACHINE_PAUSED;
    result->reason = X86_CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT;
    result->detail = VCPUINS_EXCEPT_SHUTDOWN;
    result->linear_pc = x86_core_machine_linear_pc(machine);
    result->elapsed_ticks = machine->elapsed_ticks;
    return LIB_TRUE;
}

static lib_u32 x86_core_machine_resolve_ticks_per_instruction(lib_u32 ticks)
{
    return ticks == 0u ? 1u : ticks;
}

static void x86_core_machine_resolve_instruction_timing(
    x86_core_cpu_instruction_timing *out_timing,
    const x86_core_cpu_instruction_timing *timing, lib_u32 legacy_base)
{
    *out_timing = *timing;
    if (out_timing->base_ticks == 0u) {
        out_timing->base_ticks = x86_core_machine_resolve_ticks_per_instruction(
            legacy_base);
    }
}

static lib_i32 x86_core_machine_retirement_qualification_contains(
    const x86_core_machine *machine)
{
    lib_size index;

    if (machine == LIB_NULL || !machine->retirement_eligibility_key_valid) return 0;
    for (index = 0u; index < machine->retirement_qualification_count; ++index) {
        const x86_core_machine_retirement_eligibility_key *candidate =
            &machine->retirement_qualification[index];
        const x86_core_machine_retirement_eligibility_key *key =
            &machine->retirement_eligibility_key;
        if (candidate->cpu_profile == key->cpu_profile &&
            candidate->timing_origin == key->timing_origin &&
            candidate->source_timing_form_id == key->source_timing_form_id &&
            candidate->opcode == key->opcode &&
            candidate->escape_opcode == key->escape_opcode &&
            candidate->modrm_form == key->modrm_form &&
            candidate->modrm_extension == key->modrm_extension &&
            candidate->control_outcome == key->control_outcome &&
            candidate->next_lexeme_components == key->next_lexeme_components &&
            candidate->repeat_phase == key->repeat_phase &&
            candidate->cpl == key->cpl &&
            candidate->protected_mode == key->protected_mode &&
            candidate->virtual_8086_mode == key->virtual_8086_mode &&
            candidate->operand_size_32 == key->operand_size_32 &&
            candidate->address_size_32 == key->address_size_32 &&
            candidate->lock_prefix == key->lock_prefix &&
            candidate->repeat_prefix == key->repeat_prefix) {
            return 1;
        }
    }
    return 0;
}
static lib_i32 x86_core_machine_valid_cpu_profile(x86_core_cpu_profile profile)
{
    return profile >= X86_CORE_CPU_PROFILE_8086 &&
        profile <= X86_CORE_CPU_PROFILE_80386;
}

static lib_i32 x86_core_machine_valid_fpu_profile(x86_fpu_profile profile)
{
    return profile >= X86_FPU_PROFILE_NONE &&
        profile <= X86_FPU_PROFILE_80387;
}

lib_i32 x86_core_machine_configuration_is_open(const x86_core_machine *machine)
{
    return machine != LIB_NULL &&
        machine->lifecycle == X86_CORE_MACHINE_INITIALIZED &&
        !machine->execution_provider_frozen && !machine->firmware_operation_active;
}

lib_i32 x86_core_machine_mutable_operation_is_allowed(const x86_core_machine *machine)
{
    return machine != LIB_NULL && !machine->firmware_operation_active;
}

lib_status x86_core_machine_bind_attachment(x86_core_machine *machine,
    const x86_core_machine_attachment *attachment)
{
    if (machine == LIB_NULL || attachment == LIB_NULL ||
            attachment->context == LIB_NULL ||
            attachment->finalize_devices == LIB_NULL ||
            ((attachment->special_cycle == LIB_NULL) !=
             (attachment->complete_special_cycle == LIB_NULL))) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    if (!x86_core_machine_configuration_is_open(machine) ||
            machine->attachment.context != LIB_NULL) {
        return LIB_STATUS_INVALID_STATE;
    }
    machine->attachment = *attachment;
    return LIB_STATUS_OK;
}

lib_status x86_core_machine_bind_execution_provider(x86_core_machine *machine,
    const x86_core_machine_execution_provider *provider, void *context)
{
    if (!x86_core_machine_configuration_is_open(machine)) {
        return LIB_STATUS_INVALID_STATE;
    }
    if (provider != LIB_NULL && provider->reset == LIB_NULL &&
        provider->advance_time == LIB_NULL) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    machine->execution_provider = provider;
    machine->execution_provider_context = context;
    return LIB_STATUS_OK;
}

lib_status x86_core_machine_freeze_execution_providers(x86_core_machine *machine)
{
    if (!x86_core_machine_configuration_is_open(machine)) {
        return LIB_STATUS_INVALID_STATE;
    }
    machine->execution_provider_frozen = 1;
    x86_core_machine_memory_freeze_mappings(&machine->executor_memory);
    return LIB_STATUS_OK;
}

lib_status x86_core_machine_get_cpu_state(
    const x86_core_machine *machine,
    x86_core_cpu_state *out_state)
{
    if (machine == LIB_NULL || out_state == LIB_NULL) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }

    if (machine->lifecycle == X86_CORE_MACHINE_INITIALIZED ||
        machine->lifecycle == X86_CORE_MACHINE_RUNNING) {
        return LIB_STATUS_INVALID_STATE;
    }
    x86_core_cpu_capture_state(machine->executor_cpu_execution, out_state);
    return LIB_STATUS_OK;
}

lib_status x86_core_machine_get_cpu_profile(
    const x86_core_machine *machine, x86_core_cpu_profile *out_profile)
{
    if (machine == LIB_NULL || out_profile == LIB_NULL) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    *out_profile = machine->cpu_profile;
    return LIB_STATUS_OK;
}

lib_status x86_core_machine_get_fpu_profile(
    const x86_core_machine *machine, x86_fpu_profile *out_profile)
{
    if (machine == LIB_NULL || out_profile == LIB_NULL) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    *out_profile = x86_fpu_get_profile(machine->fpu);
    return LIB_STATUS_OK;
}

lib_status x86_core_machine_get_fpu_state(
    const x86_core_machine *machine, x86_fpu_state *out_state)
{
    if (machine == LIB_NULL || out_state == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    x86_fpu_get_state(machine->fpu, out_state);
    return LIB_STATUS_OK;
}

lib_status x86_core_machine_get_memory_bytes(
    const x86_core_machine *machine, lib_size *out_memory_bytes)
{
    if (machine == LIB_NULL || out_memory_bytes == LIB_NULL) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    *out_memory_bytes = machine->executor_memory.connect.installed_bytes;
    return LIB_STATUS_OK;
}

lib_status x86_core_machine_get_elapsed_ticks(
    const x86_core_machine *machine, lib_u64 *out_elapsed_ticks)
{
    if (machine == LIB_NULL || out_elapsed_ticks == LIB_NULL ||
        machine->lifecycle == X86_CORE_MACHINE_INITIALIZED ||
        machine->lifecycle == X86_CORE_MACHINE_RUNNING) {
        return LIB_STATUS_INVALID_STATE;
    }
    *out_elapsed_ticks = machine->elapsed_ticks;
    return LIB_STATUS_OK;
}

lib_status x86_core_machine_capture_time_observation(const x86_core_machine *machine,
    x86_core_machine_time_observation *out_observation)
{
    if (machine == LIB_NULL || out_observation == LIB_NULL ||
        machine->lifecycle == X86_CORE_MACHINE_INITIALIZED ||
        machine->lifecycle == X86_CORE_MACHINE_RUNNING) {
        return LIB_STATUS_INVALID_STATE;
    }
    x86_core_machine_capture_time_observation_private(machine, out_observation);
    return LIB_STATUS_OK;
}

lib_status x86_core_machine_get_timeline_observation(const x86_core_machine *machine,
    x86_core_machine_timeline_observation *out_observation)
{
    if (machine == LIB_NULL || out_observation == LIB_NULL) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    out_observation->now = machine->timeline.now;
    out_observation->next_sequence = machine->timeline.next_sequence;
    out_observation->pending_events = x86_core_machine_timeline_pending_count(
        &machine->timeline);
    return LIB_STATUS_OK;
}

lib_status x86_core_machine_get_cpu_diagnostic(
    const x86_core_machine *machine, x86_core_cpu_diagnostic *out_diagnostic)
{
    if (machine == LIB_NULL || out_diagnostic == LIB_NULL ||
        machine->lifecycle == X86_CORE_MACHINE_INITIALIZED ||
        machine->lifecycle == X86_CORE_MACHINE_RUNNING) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    x86_core_machine_cpu_diagnostic_capture(machine, out_diagnostic);
    return LIB_STATUS_OK;
}

lib_status x86_core_machine_capture_observation(
    const x86_core_machine *machine, x86_core_machine_observation *out_observation)
{
    if (machine == LIB_NULL || out_observation == LIB_NULL ||
        machine->lifecycle == X86_CORE_MACHINE_INITIALIZED ||
        machine->lifecycle == X86_CORE_MACHINE_RUNNING) {
        return LIB_STATUS_INVALID_STATE;
    }
    out_observation->lifecycle = machine->lifecycle;
    out_observation->elapsed_ticks = machine->elapsed_ticks;
    if (x86_core_machine_get_cpu_state(machine, &out_observation->cpu) !=
            LIB_STATUS_OK) {
        return LIB_STATUS_INVALID_STATE;
    }
    x86_core_machine_cpu_diagnostic_capture(machine, &out_observation->diagnostic);
    return LIB_STATUS_OK;
}

lib_i32 x86_core_machine_retirement_time_contract_is_valid(
    x86_core_machine_retirement_time_contract contract)
{
    return contract == X86_CORE_MACHINE_RETIREMENT_TIME_DETERMINISTIC ||
        contract == X86_CORE_MACHINE_RETIREMENT_TIME_PHYSICAL;
}

lib_i32 x86_core_machine_external_cycle_timing_is_valid(
    const x86_core_machine_external_cycle_timing *timing)
{
    if (timing == LIB_NULL || (timing->overlap_policy !=
        X86_CORE_MACHINE_EXTERNAL_CYCLE_OVERLAP_DISABLED && timing->overlap_policy !=
        X86_CORE_MACHINE_EXTERNAL_CYCLE_OVERLAP_EXPLICIT_SEQUENTIAL)) return 0;
    if (timing->page_bytes == 0u) {
        return timing->page_miss_ticks == 0u && timing->page_hit_ticks == 0u &&
            timing->overlap_policy == X86_CORE_MACHINE_EXTERNAL_CYCLE_OVERLAP_DISABLED;
    }
    return (timing->page_bytes & (timing->page_bytes - 1u)) == 0u &&
        ((timing->first_eligible_address == 0u &&
          timing->last_eligible_address == 0u) ||
         timing->first_eligible_address <= timing->last_eligible_address);
}

lib_i32 x86_core_machine_external_access_wait_windows_are_valid(
    const x86_core_machine_external_access_wait_window *windows)
{
    lib_size index;

    if (windows == LIB_NULL) return 0;
    for (index = 0u; index < X86_CORE_MACHINE_EXTERNAL_ACCESS_WAIT_WINDOW_CAPACITY;
            ++index) {
        const x86_core_machine_external_access_wait_window *window = &windows[index];
        if (window->wait_ticks == 0u) continue;
        if ((window->space != X86_CORE_CPU_EXTERNAL_CYCLE_SPACE_MEMORY &&
                window->space != X86_CORE_CPU_EXTERNAL_CYCLE_SPACE_PORT) ||
            window->first_address > window->last_address ||
            (window->space == X86_CORE_CPU_EXTERNAL_CYCLE_SPACE_PORT &&
                window->last_address > 0xffffu)) return 0;
    }
    return 1;
}

lib_i32 x86_core_machine_transaction_contract_is_valid(
    const x86_core_machine_transaction_contract *contract)
{
    return contract != LIB_NULL &&
        x86_core_machine_external_cycle_timing_is_valid(
            &contract->external_cycle_timing) &&
        x86_core_machine_external_access_wait_windows_are_valid(
            contract->external_access_wait_windows) &&
        (contract->dma_cycle_bus_ready_gate_enabled == LIB_FALSE ||
         contract->dma_cycle_bus_ready_gate_enabled == LIB_TRUE) &&
        (contract->cpu_cycle_bus_ready_gate_enabled == LIB_FALSE ||
         contract->cpu_cycle_bus_ready_gate_enabled == LIB_TRUE) &&
        (contract->cpu_prefetch_reservation_enabled == LIB_FALSE ||
         contract->cpu_prefetch_reservation_enabled == LIB_TRUE);
}

lib_i32 x86_core_machine_neutral_config_is_valid(
    const x86_core_machine_executor_config *config)
{
    return config != LIB_NULL && config->memory_bytes != 0u &&
        x86_core_machine_valid_cpu_profile(config->cpu_profile) &&
        x86_core_machine_valid_fpu_profile(config->fpu_profile) &&
        config->address_decode.first <= config->address_decode.last &&
        x86_core_machine_clock_ratio_is_valid(&config->provider_clock) &&
        x86_core_machine_retirement_time_contract_is_valid(
            config->retirement_time_contract) &&
        x86_core_machine_transaction_contract_is_valid(
            &config->transaction_contract) &&
        (config->time_axis.kind == X86_CORE_MACHINE_TIME_AXIS_UNQUALIFIED ||
         config->time_axis.kind == X86_CORE_MACHINE_TIME_AXIS_MACRO_PROPORTIONAL ||
         config->time_axis.kind == X86_CORE_MACHINE_TIME_AXIS_VERIFIED_PHYSICAL) &&
        (config->l1_compatibility_policy == X86_CORE_MACHINE_L1_COMPATIBILITY_DISABLED ||
         config->l1_compatibility_policy == X86_CORE_MACHINE_L1_COMPATIBILITY_BOUNDED_PROGRESS) &&
        (config->time_axis.kind == X86_CORE_MACHINE_TIME_AXIS_UNQUALIFIED ?
            config->time_axis.ticks_per_second == 0u :
            config->time_axis.ticks_per_second != 0u) &&
        (config->retirement_time_contract != X86_CORE_MACHINE_RETIREMENT_TIME_PHYSICAL ||
         config->time_axis.kind == X86_CORE_MACHINE_TIME_AXIS_VERIFIED_PHYSICAL);
}


lib_status x86_core_machine_neutral_create_with_test_allocation(
    const x86_core_machine_executor_config *config,
    x86_core_machine_memory_test_allocation *test_allocation,
    x86_core_machine_port_test_allocation *port_test_allocation,
    x86_core_machine **out_machine)
{
    x86_core_machine *machine;
    x86_core_cpu_instruction_timing instruction_timing;
    lib_size memory_bytes;

    if (out_machine == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    *out_machine = LIB_NULL;
    if (!x86_core_machine_neutral_config_is_valid(config))
        return LIB_STATUS_INVALID_ARGUMENT;
    memory_bytes = config->memory_bytes;

    machine = (x86_core_machine *)lib_allocate_zero(1u, sizeof(*machine));
    if (machine == LIB_NULL) {
        return LIB_STATUS_NO_MEMORY;
    }

    machine->lifecycle = X86_CORE_MACHINE_INITIALIZED;
    machine->cpu_profile = config->cpu_profile;
    machine->retirement_time_contract = config->retirement_time_contract;
    machine->transaction_contract = config->transaction_contract;
    machine->time_axis = config->time_axis;
    machine->l1_compatibility_policy = config->l1_compatibility_policy;
    machine->dma_cycle_bus_ready = LIB_TRUE;
    machine->cpu_cycle_bus_ready = LIB_TRUE;
    if (config->retirement_qualification != LIB_NULL) {
        if (config->retirement_qualification->entries == LIB_NULL ||
            config->retirement_qualification->entry_count == 0u ||
            config->retirement_qualification->entry_count >
                X86_CORE_MACHINE_RETIREMENT_QUALIFICATION_CAPACITY) {
            lib_release(machine);
            return LIB_STATUS_INVALID_ARGUMENT;
        }
        machine->retirement_qualification_count =
            config->retirement_qualification->entry_count;
        lib_memory_copy(machine->retirement_qualification,
            config->retirement_qualification->entries,
            machine->retirement_qualification_count *
                sizeof(machine->retirement_qualification[0]));
    }
    machine->cpu_80386_cr_mov_ignores_mod =
        config->cpu_80386_cr_mov_ignores_mod;
    if (machine->cpu_80386_cr_mov_ignores_mod &&
        machine->cpu_profile != X86_CORE_CPU_PROFILE_80386) {
        lib_release(machine);
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    if (x86_core_machine_timeline_initialize(&machine->timeline) != LIB_STATUS_OK) {
        lib_release(machine);
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    x86_core_machine_resolve_instruction_timing(&instruction_timing,
        &config->instruction_timing, config->ticks_per_instruction);
    machine->maximum_instruction_ticks = x86_core_cpu_timing_maximum_ticks(
        machine->cpu_profile, &instruction_timing);
    if (x86_core_machine_clock_domain_initialize(&machine->provider_clock,
            &config->provider_clock) != LIB_STATUS_OK) {
        lib_release(machine);
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    {
        lib_status status = x86_fpu_create(config->fpu_profile, &machine->fpu);
        if (status != LIB_STATUS_OK) {
            lib_release(machine);
            return status;
        }
    }
    lib_atomic_i32_initialize(&machine->stop_requested, 0);
    x86_core_machine_trace_initialize(machine);
    x86_core_machine_transaction_initialize(&machine->transaction);
    x86_core_machine_transaction_bind_trace(&machine->transaction,
        x86_core_machine_transaction_trace, machine);
    x86_core_machine_cpu_diagnostic_initialize(machine);
    x86_core_machine_retirement_observation_initialize(machine);

    {
        lib_status status = x86_core_cpu_create(&x86_core_machine_cpu_bus,
            machine, &machine->executor_cpu_execution);

        if (status != LIB_STATUS_OK) {
            x86_fpu_destroy(machine->fpu);
            lib_release(machine);
            return status;
        }
    }
    x86_core_cpu_execution_context_bind_profiles(
        machine->executor_cpu_execution, machine->cpu_profile,
        x86_fpu_get_profile(machine->fpu), machine->cpu_80386_cr_mov_ignores_mod,
        &instruction_timing);
    x86_core_cpu_execution_context_bind_fpu(
        machine->executor_cpu_execution, machine->fpu);
    x86_core_cpu_execution_context_bind_external_cycle_provider(
        machine->executor_cpu_execution, x86_core_machine_cpu_external_cycle_trace,
        machine);
    x86_core_cpu_execution_context_bind_diagnostic_provider(
        machine->executor_cpu_execution,
        machine->retirement_time_contract == X86_CORE_MACHINE_RETIREMENT_TIME_PHYSICAL ?
            &x86_core_machine_cpu_diagnostic_provider :
            &x86_core_machine_cpu_fault_diagnostic_provider,
        machine);
    x86_core_cpu_state_initialize(machine->executor_cpu_execution);
    x86_core_machine_port_initialize(&machine->executor_port);
    x86_core_machine_port_set_test_allocation(&machine->executor_port,
        port_test_allocation);
    if (x86_core_machine_memory_initialize_for(&machine->executor_memory,
            memory_bytes, test_allocation) != LIB_STATUS_OK) {
        x86_core_machine_destroy(machine);
        return LIB_STATUS_NO_MEMORY;
    }
    machine->executor_memory.connect.address_decode = config->address_decode;
    machine->executor_memory.data.address_clear_mask =
        config->address_decode.reset_clear_mask;
    machine->executor_memory.connect.cpu_snoop = x86_core_machine_cpu_bus_snoop_write;
    machine->executor_memory.connect.cpu_snoop_context = machine;
    *out_machine = machine;
    return LIB_STATUS_OK;
}


lib_status x86_core_machine_neutral_create(
    const x86_core_machine_executor_config *config, x86_core_machine **out_machine)
{
    return x86_core_machine_neutral_create_with_test_allocation(config,
        LIB_NULL, LIB_NULL, out_machine);
}

static lib_status x86_core_machine_cold_reset(x86_core_machine *machine)
{
    lib_status status;
    x86_core_cpu_state_reset(machine->executor_cpu_execution);
    x86_fpu_reset(machine->fpu);
    x86_core_machine_port_reset(&machine->executor_port);
    x86_core_machine_memory_reset(&machine->executor_memory);
    if (machine->attachment.reset_devices != LIB_NULL) {
        machine->attachment.reset_devices(machine->attachment.context);
    }

    lib_atomic_i32_store_explicit(&machine->stop_requested, 0, LIB_MEMORY_ORDER_RELEASE);
    machine->fault_detail = 0u;
    machine->elapsed_ticks = 0u;
    machine->dma_cycle_wait_remaining = 0u;
    machine->refresh_cycle_ticks_remaining = 0u;
    machine->dma_cycle_bus_ready = LIB_TRUE;
    machine->cpu_cycle_bus_ready = LIB_TRUE;
    machine->external_cycle_page_tag = 0u;
    machine->external_cycle_round_ticks = 0u;
    machine->bus_invalidation_cycles = 0u;
    machine->bus_invalidation_seen = LIB_FALSE;
    machine->cpu_retirement_wait_ticks = 0u;
    machine->cpu_retirement_completion_ticks = 0u;
    machine->cpu_retirement_source_ticks = 0u;
    machine->cpu_retirement_wait_retires = LIB_FALSE;
    machine->external_cycle_page_valid = LIB_FALSE;
    machine->external_cycle_pending_valid = LIB_FALSE;
    machine->external_cycle_pending_space = X86_CORE_CPU_EXTERNAL_CYCLE_SPACE_MEMORY;
    machine->external_cycle_pending_physical = 0u;
    machine->external_cycle_pending_bytes = 0u;
    machine->external_cycle_pending_write = LIB_FALSE;
    machine->external_cycle_pending_provenance =
        X86_CORE_CPU_MEMORY_ACCESS_DATA;
    machine->external_cycle_overlap_valid = LIB_FALSE;
    machine->external_cycle_overlap_next_physical = 0u;
    machine->external_cycle_round_overflow = LIB_FALSE;
    machine->cpu_retirement_wait_pending = LIB_FALSE;
    machine->retirement_eligibility_key_valid = LIB_FALSE;
    x86_core_machine_transaction_reset(&machine->transaction);
    x86_core_machine_timeline_reset(&machine->timeline);
    if (machine->attachment.reset_clocks != LIB_NULL) {
        machine->attachment.reset_clocks(machine->attachment.context);
    }
    x86_core_machine_clock_domain_reset(&machine->provider_clock);
    machine->entry_plan_applied = LIB_FALSE;
    x86_core_machine_cpu_diagnostic_reset(machine);
    x86_core_machine_retirement_observation_reset(machine);
    if (machine->execution_provider != LIB_NULL &&
        machine->execution_provider->reset != LIB_NULL) {
        machine->execution_provider->reset(machine->execution_provider_context);
    }
    if (machine->firmware_provider != LIB_NULL) {
        status = x86_core_machine_firmware_invoke(machine, 0, 1,
            machine->firmware_provider->reset);
        if (status != LIB_STATUS_OK) {
            machine->lifecycle = X86_CORE_MACHINE_INITIALIZED;
            return status;
        }
    }
    machine->lifecycle = X86_CORE_MACHINE_STOPPED;
    x86_core_machine_trace_record(machine, X86_CORE_MACHINE_TRACE_RESET, 0u, 0u, 0u);
    return LIB_STATUS_OK;
}

/* An 8042 reset pulse resets the processor only.  Preserve RAM, board state
 * and scheduled device work; discard only incomplete CPU execution state. */
static lib_status x86_core_machine_processor_reset(x86_core_machine *machine,
    x86_core_cpu_reset_kind kind)
{
    lib_status status;

    if (kind == X86_CORE_CPU_RESET_SOFT) {
        status = x86_core_cpu_state_soft_reset(machine->executor_cpu_execution);
        if (status != LIB_STATUS_OK) return status;
    } else {
        x86_core_cpu_state_reset(machine->executor_cpu_execution);
    }
    machine->cpu_retirement_wait_pending = LIB_FALSE;
    machine->cpu_retirement_wait_ticks = 0u;
    machine->cpu_retirement_completion_ticks = 0u;
    machine->cpu_retirement_source_ticks = 0u;
    machine->cpu_retirement_wait_retires = LIB_FALSE;
    machine->external_cycle_page_valid = LIB_FALSE;
    machine->external_cycle_pending_valid = LIB_FALSE;
    machine->external_cycle_round_ticks = 0u;
    machine->bus_invalidation_cycles = 0u;
    machine->bus_invalidation_seen = LIB_FALSE;
    machine->external_cycle_round_overflow = LIB_FALSE;
    machine->external_cycle_overlap_valid = LIB_FALSE;
    machine->retirement_eligibility_key_valid = LIB_FALSE;
    machine->fault_detail = 0u;
    return LIB_STATUS_OK;
}

lib_status x86_core_machine_reconfigure_memory(x86_core_machine *machine,
    lib_size memory_bytes)
{
    lib_uptr index;
    lib_status status;

    if (machine == LIB_NULL || !x86_core_machine_mutable_operation_is_allowed(machine) ||
        !machine->execution_provider_frozen ||
        machine->lifecycle != X86_CORE_MACHINE_STOPPED ||
        memory_bytes == 0u) {
        return LIB_STATUS_INVALID_STATE;
    }
    if (machine->attachment.memory_admission != LIB_NULL) {
        status = machine->attachment.memory_admission(machine->attachment.context,
            memory_bytes);
        if (status != LIB_STATUS_OK) return status;
    }
    for (index = 0u; index < machine->executor_memory.connect.mapping_count;
            ++index) {
        const x86_core_machine_memory_mapping *mapping =
            &machine->executor_memory.connect.mappings[index];
        if (mapping->backing_start > memory_bytes ||
            mapping->bytes > memory_bytes - mapping->backing_start) {
            return LIB_STATUS_INVALID_ARGUMENT;
        }
    }
    if (x86_core_machine_memory_allocate_for(&machine->executor_memory,
            memory_bytes) != LIB_STATUS_OK) {
        return LIB_STATUS_NO_MEMORY;
    }
    return x86_core_machine_cold_reset(machine);
}

lib_status x86_core_machine_reset(x86_core_machine *machine)
{
    if (machine == LIB_NULL) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }

    if (!x86_core_machine_mutable_operation_is_allowed(machine) ||
        !machine->execution_provider_frozen ||
        machine->lifecycle == X86_CORE_MACHINE_RUNNING) {
        return LIB_STATUS_INVALID_STATE;
    }

    return x86_core_machine_cold_reset(machine);
}

lib_status x86_core_machine_get_lifecycle(
    const x86_core_machine *machine,
    x86_core_machine_lifecycle *out_lifecycle)
{
    if (machine == LIB_NULL || out_lifecycle == LIB_NULL) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }

    *out_lifecycle = machine->lifecycle;
    return LIB_STATUS_OK;
}

static lib_status x86_core_machine_complete_run_boundary(x86_core_machine *machine,
    x86_core_machine_run_result *result)
{
    if (machine->firmware_provider != LIB_NULL &&
        machine->firmware_provider->after_run != LIB_NULL &&
        x86_core_machine_firmware_invoke(machine, 0, 0,
            machine->firmware_provider->after_run) != LIB_STATUS_OK) {
        (void)x86_core_machine_report_fault(machine, 0x46575245u);
        result->reason = X86_CORE_MACHINE_STOP_FAULT;
        result->detail = machine->fault_detail;
        return LIB_STATUS_INTERNAL_ERROR;
    }
    if (lib_atomic_i32_load_explicit(&machine->stop_requested, LIB_MEMORY_ORDER_ACQUIRE)) {
        result->reason = X86_CORE_MACHINE_STOP_REQUESTED;
    }
    x86_core_machine_trace_record(machine, X86_CORE_MACHINE_TRACE_RUN_BOUNDARY,
        result->linear_pc, (lib_u32)result->executed,
        (lib_u32)result->reason);
    return LIB_STATUS_OK;
}

static lib_status x86_core_machine_complete_cpu_fault(x86_core_machine *machine,
    x86_core_machine_run_result *result)
{
    const lib_status status = x86_core_machine_report_fault(machine,
        machine->fault_detail);
    result->reason = X86_CORE_MACHINE_STOP_FAULT;
    result->linear_pc = x86_core_machine_linear_pc(machine);
    result->detail = machine->fault_detail;
    result->elapsed_ticks = machine->elapsed_ticks;
    return status == LIB_STATUS_OK ? LIB_STATUS_INTERNAL_ERROR : status;
}

lib_status x86_core_machine_run(
    x86_core_machine *machine,
    x86_core_machine_run_budget budget,
    x86_core_machine_run_result *result)
{
    if (machine == LIB_NULL || result == LIB_NULL ||
        !x86_core_machine_mutable_operation_is_allowed(machine)) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }

    result->reason = X86_CORE_MACHINE_STOP_NONE;
    result->executed = 0u;
    result->ticks = 0u;
    result->elapsed_ticks = machine->elapsed_ticks;
    result->linear_pc = x86_core_machine_linear_pc(machine);
    result->detail = 0u;

    if (machine->lifecycle == X86_CORE_MACHINE_FAULTED) {
        result->reason = X86_CORE_MACHINE_STOP_FAULT;
        result->detail = machine->fault_detail;
        return LIB_STATUS_INTERNAL_ERROR;
    }

    if (machine->lifecycle != X86_CORE_MACHINE_STOPPED &&
        machine->lifecycle != X86_CORE_MACHINE_PAUSED) {
        return LIB_STATUS_INVALID_STATE;
    }

    if (budget.instructions == 0u && budget.ticks == 0u) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }

    if (lib_atomic_i32_load_explicit(&machine->stop_requested, LIB_MEMORY_ORDER_ACQUIRE)) {
        lib_status status = x86_core_machine_cold_reset(machine);
        if (status != LIB_STATUS_OK) return status;
        result->reason = X86_CORE_MACHINE_STOP_REQUESTED;
        result->linear_pc = x86_core_machine_linear_pc(machine);
        x86_core_machine_trace_record(machine, X86_CORE_MACHINE_TRACE_STOP, 0u, 0u,
                               (lib_u32)result->reason);
        return LIB_STATUS_OK;
    }

    machine->lifecycle = X86_CORE_MACHINE_RUNNING;
    {
        while ((machine->cpu_retirement_wait_pending || budget.instructions == 0u ||
                result->executed < budget.instructions) &&
               (budget.ticks == 0u || result->ticks < budget.ticks)) {
            const lib_bool cpu_fault_pending = machine->fault_detail != 0u;

            if (machine->lifecycle == X86_CORE_MACHINE_FAULTED) {
                result->reason = X86_CORE_MACHINE_STOP_FAULT;
                result->linear_pc = x86_core_machine_linear_pc(machine);
                result->detail = machine->fault_detail;
                return LIB_STATUS_INTERNAL_ERROR;
            }
            /* Only an explicit board binding converts shutdown to CPU reset. */
            if (machine->attachment.shutdown_reset != LIB_NULL &&
                machine->attachment.shutdown_reset(machine->attachment.context) &&
                x86_core_cpu_execution_consume_shutdown_request(
                    machine->executor_cpu_execution)) {
                machine->lifecycle = X86_CORE_MACHINE_PAUSED;
                const lib_status status = x86_core_machine_processor_reset(machine,
                    X86_CORE_CPU_RESET_HARD);
                if (status != LIB_STATUS_OK) return status;
                machine->lifecycle = X86_CORE_MACHINE_STOPPED;
                result->reason = X86_CORE_MACHINE_STOP_RESET_REQUESTED;
                result->linear_pc = x86_core_machine_linear_pc(machine);
                return LIB_STATUS_OK;
            }
            if (x86_core_cpu_execution_consume_debug_pause_request(
                    machine->executor_cpu_execution)) {
                machine->lifecycle = X86_CORE_MACHINE_PAUSED;
                result->reason = X86_CORE_MACHINE_STOP_PAUSED;
                result->linear_pc = x86_core_machine_linear_pc(machine);
                return x86_core_machine_complete_run_boundary(machine, result);
            }
            if (lib_atomic_i32_load_explicit(&machine->stop_requested, LIB_MEMORY_ORDER_ACQUIRE) ||
                (!cpu_fault_pending && x86_core_cpu_execution_consume_stop_request(
                    machine->executor_cpu_execution))) {
                machine->lifecycle = X86_CORE_MACHINE_PAUSED;
                {
                    lib_status status = x86_core_machine_cold_reset(machine);
                    if (status != LIB_STATUS_OK) return status;
                }
                result->reason = X86_CORE_MACHINE_STOP_REQUESTED;
                result->linear_pc = x86_core_machine_linear_pc(machine);
                x86_core_machine_trace_record(machine, X86_CORE_MACHINE_TRACE_STOP, 0u,
                    0u, (lib_u32)result->reason);
                return LIB_STATUS_OK;
            }
            {
                const x86_core_cpu_reset_kind reset =
                    x86_core_cpu_execution_consume_reset_request(machine->executor_cpu_execution);

                if (reset != X86_CORE_CPU_RESET_NONE) {
                    const lib_status status = x86_core_machine_processor_reset(machine, reset);

                    if (status != LIB_STATUS_OK) return status;
                    machine->lifecycle = X86_CORE_MACHINE_STOPPED;
                    result->reason = X86_CORE_MACHINE_STOP_RESET_REQUESTED;
                    result->linear_pc = x86_core_machine_linear_pc(machine);
                    return LIB_STATUS_OK;
                }
            }
            if (machine->transaction_contract.refresh_cycle_source_ticks != 0u)
                x86_core_machine_refresh_hold_advance(machine, 0u);
            if (machine->refresh_cycle_ticks_remaining != 0u) {
                lib_u64 ticks = machine->refresh_cycle_ticks_remaining;

                if (budget.ticks != 0u && ticks > budget.ticks - result->ticks)
                    ticks = budget.ticks - result->ticks;
                if (LIB_UINT64_MAX - result->ticks < ticks ||
                    LIB_UINT64_MAX - machine->elapsed_ticks < ticks) {
                    (void)x86_core_machine_report_fault(machine, 0x54494d45u);
                    result->reason = X86_CORE_MACHINE_STOP_FAULT;
                    result->linear_pc = x86_core_machine_linear_pc(machine);
                    result->detail = machine->fault_detail;
                    return LIB_STATUS_INTERNAL_ERROR;
                }
                if (x86_core_machine_publish_elapsed_ticks(machine, ticks,
                        X86_CORE_MACHINE_TIME_PUBLICATION_DEADLINE) != LIB_STATUS_OK)
                    return LIB_STATUS_INTERNAL_ERROR;
                result->ticks += ticks;
                result->elapsed_ticks = machine->elapsed_ticks;
                continue;
            }
            if (machine->attachment.cpu_hold_asserted != LIB_NULL &&
                machine->attachment.cpu_hold_asserted(machine->attachment.context)) {
                if (result->ticks == LIB_UINT64_MAX || machine->elapsed_ticks == LIB_UINT64_MAX) {
                    (void)x86_core_machine_report_fault(machine, 0x54494d45u);
                    result->reason = X86_CORE_MACHINE_STOP_FAULT;
                    result->linear_pc = x86_core_machine_linear_pc(machine);
                    result->detail = machine->fault_detail;
                    return LIB_STATUS_INTERNAL_ERROR;
                }
                ++result->ticks;
                if (x86_core_machine_publish_elapsed_ticks(machine, 1u,
                        X86_CORE_MACHINE_TIME_PUBLICATION_EXTERNAL_WAIT) !=
                    LIB_STATUS_OK) return LIB_STATUS_INTERNAL_ERROR;
                result->elapsed_ticks = machine->elapsed_ticks;
                continue;
            }
            if (machine->cpu_retirement_wait_pending) {
                const lib_u64 event_ticks = machine->cpu_retirement_wait_retires ?
                    x86_core_cpu_capture_timing(machine->executor_cpu_execution).event_ticks : 0u;
                const lib_u64 retiring_ticks = machine->cpu_retirement_completion_ticks >= event_ticks ?
                    machine->cpu_retirement_completion_ticks - event_ticks : 0u;

                if (machine->transaction_contract.cpu_cycle_bus_ready_gate_enabled &&
                    !machine->cpu_cycle_bus_ready) {
                    if (result->ticks == LIB_UINT64_MAX || machine->elapsed_ticks == LIB_UINT64_MAX) {
                        (void)x86_core_machine_report_fault(machine, 0x54494d45u);
                        result->reason = X86_CORE_MACHINE_STOP_FAULT;
                        result->linear_pc = x86_core_machine_linear_pc(machine);
                        result->detail = machine->fault_detail;
                        return LIB_STATUS_INTERNAL_ERROR;
                    }
                    ++result->ticks;
                    if (x86_core_machine_publish_elapsed_ticks(machine, 1u,
                            X86_CORE_MACHINE_TIME_PUBLICATION_EXTERNAL_WAIT) !=
                        LIB_STATUS_OK) return LIB_STATUS_INTERNAL_ERROR;
                    result->elapsed_ticks = machine->elapsed_ticks;
                    continue;
                }
                if (machine->cpu_retirement_wait_ticks != 0u) {
                    if (result->ticks == LIB_UINT64_MAX ||
                        machine->elapsed_ticks == LIB_UINT64_MAX) {
                        (void)x86_core_machine_report_fault(machine, 0x54494d45u);
                        result->reason = X86_CORE_MACHINE_STOP_FAULT;
                        result->linear_pc = x86_core_machine_linear_pc(machine);
                        result->detail = machine->fault_detail;
                        result->elapsed_ticks = machine->elapsed_ticks;
                        return LIB_STATUS_INTERNAL_ERROR;
                    }
                    ++result->ticks;
                    --machine->cpu_retirement_wait_ticks;
                    if (x86_core_machine_publish_elapsed_ticks(machine, 1u,
                            X86_CORE_MACHINE_TIME_PUBLICATION_EXTERNAL_WAIT) !=
                        LIB_STATUS_OK) return LIB_STATUS_INTERNAL_ERROR;
                    result->elapsed_ticks = machine->elapsed_ticks;
                    if (machine->cpu_retirement_wait_retires &&
                        x86_core_cpu_is_halted(machine->executor_cpu_execution)) {
                        machine->lifecycle = X86_CORE_MACHINE_PAUSED;
                        result->reason = X86_CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT;
                        result->linear_pc = x86_core_machine_linear_pc(machine);
                        return x86_core_machine_complete_run_boundary(machine, result);
                    }
                    if (machine->cpu_retirement_wait_ticks != 0u) continue;
                }
                if (machine->cpu_retirement_wait_retires && budget.ticks != 0u &&
                    retiring_ticks >
                    budget.ticks - result->ticks) {
                    machine->lifecycle = X86_CORE_MACHINE_PAUSED;
                    result->reason = X86_CORE_MACHINE_STOP_BUDGET;
                    result->linear_pc = x86_core_machine_linear_pc(machine);
                    result->elapsed_ticks = machine->elapsed_ticks;
                    return LIB_STATUS_OK;
                }
                if (event_ticks > machine->cpu_retirement_completion_ticks ||
                    LIB_UINT64_MAX - result->ticks < machine->cpu_retirement_completion_ticks ||
                    LIB_UINT64_MAX - machine->elapsed_ticks <
                        machine->cpu_retirement_completion_ticks) {
                    (void)x86_core_machine_report_fault(machine, 0x54494d45u);
                    result->reason = X86_CORE_MACHINE_STOP_FAULT;
                    result->linear_pc = x86_core_machine_linear_pc(machine);
                    result->detail = machine->fault_detail;
                    result->elapsed_ticks = machine->elapsed_ticks;
                    return LIB_STATUS_INTERNAL_ERROR;
                }
                if (!machine->cpu_retirement_wait_retires) {
                    lib_u64 event_ticks = machine->cpu_retirement_completion_ticks;

                    if (budget.ticks != 0u && event_ticks > budget.ticks - result->ticks)
                        event_ticks = budget.ticks - result->ticks;

                    if (event_ticks != 0u) {
                        if (x86_core_machine_publish_elapsed_ticks(machine, event_ticks,
                                X86_CORE_MACHINE_TIME_PUBLICATION_CPU_EVENT) != LIB_STATUS_OK)
                            return LIB_STATUS_INTERNAL_ERROR;
                        result->ticks += event_ticks;
                        result->elapsed_ticks = machine->elapsed_ticks;
                    }
                    machine->cpu_retirement_completion_ticks -= event_ticks;
                    if (machine->cpu_retirement_completion_ticks != 0u) continue;
                    machine->cpu_retirement_wait_pending = LIB_FALSE;
                    machine->cpu_retirement_source_ticks = 0u;
                    if (cpu_fault_pending)
                        return x86_core_machine_complete_cpu_fault(machine, result);
                    if (x86_core_cpu_is_shutdown(machine->executor_cpu_execution)) {
                        if (x86_core_machine_finish_shutdown(machine, result))
                            return x86_core_machine_complete_run_boundary(machine, result);
                        continue;
                    }
                    machine->lifecycle = X86_CORE_MACHINE_PAUSED;
                    result->reason = (x86_core_cpu_is_system_waiting(machine->executor_cpu_execution) ||
                        x86_core_cpu_execution_consume_wait_stall(machine->executor_cpu_execution)) ?
                        X86_CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT : X86_CORE_MACHINE_STOP_BUDGET;
                    result->linear_pc = x86_core_machine_linear_pc(machine);
                    result->elapsed_ticks = machine->elapsed_ticks;
                    return LIB_STATUS_OK;
                }
                if (!x86_core_machine_publish_successful_retirement(machine)) {
                    (void)x86_core_machine_report_fault(machine, 0x54494d55u);
                    result->reason = X86_CORE_MACHINE_STOP_FAULT;
                    result->linear_pc = x86_core_machine_linear_pc(machine);
                    result->detail = machine->fault_detail;
                    result->elapsed_ticks = machine->elapsed_ticks;
                    return LIB_STATUS_INTERNAL_ERROR;
                }
                ++result->executed;
                result->ticks += retiring_ticks;
                if (x86_core_machine_publish_elapsed_ticks(machine,
                        retiring_ticks,
                        X86_CORE_MACHINE_TIME_PUBLICATION_CPU_RETIREMENT) !=
                    LIB_STATUS_OK) return LIB_STATUS_INTERNAL_ERROR;
                machine->cpu_retirement_completion_ticks = event_ticks;
                machine->cpu_retirement_source_ticks = 0u;
                machine->cpu_retirement_wait_retires = LIB_FALSE;
                result->elapsed_ticks = machine->elapsed_ticks;
                if (event_ticks != 0u) continue;
                machine->cpu_retirement_wait_pending = LIB_FALSE;
                if (cpu_fault_pending)
                    return x86_core_machine_complete_cpu_fault(machine, result);
                continue;
            }
            if (budget.ticks != 0u && machine->maximum_instruction_ticks >
                budget.ticks - result->ticks) {
                machine->lifecycle = X86_CORE_MACHINE_PAUSED;
                result->reason = X86_CORE_MACHINE_STOP_BUDGET;
                result->linear_pc = x86_core_machine_linear_pc(machine);
                result->elapsed_ticks = machine->elapsed_ticks;
                return x86_core_machine_complete_run_boundary(machine, result);
            }
            {
                lib_bool was_halted = x86_core_cpu_is_halted(
                    machine->executor_cpu_execution);

                machine->external_cycle_round_ticks = 0u;
                /* A completed instruction round cannot inherit an undeclared
                 * external-cycle overlap into the next CPU refresh. */
                x86_core_machine_external_cycle_invalidate(machine);
                x86_core_cpu_execution_refresh(machine->executor_cpu_execution);
                if (machine->lifecycle == X86_CORE_MACHINE_FAULTED) {
                    result->reason = X86_CORE_MACHINE_STOP_FAULT;
                    result->linear_pc = x86_core_machine_linear_pc(machine);
                    result->detail = machine->fault_detail;
                    result->elapsed_ticks = machine->elapsed_ticks;
                    return LIB_STATUS_INTERNAL_ERROR;
                }
                if ((machine->fault_detail != 0u ||
                    x86_core_cpu_capture_timing(machine->executor_cpu_execution).event_ticks != 0u ||
                    x86_core_cpu_execution_consume_instruction_fault_delivery(
                        machine->executor_cpu_execution)) &&
                    x86_core_cpu_capture_timing(machine->executor_cpu_execution).ticks == 0u) {
                    const lib_u64 event_ticks = x86_core_cpu_capture_timing(
                        machine->executor_cpu_execution).event_ticks;

                    /* Committed entry did not retire the interrupted/faulting
                     * instruction. Its CPU subtotal and separate board wait use
                     * the existing pending owner without synthetic retirement. */
                    if (machine->external_cycle_round_overflow) {
                        (void)x86_core_machine_report_fault(machine, 0x54494d45u);
                        result->reason = X86_CORE_MACHINE_STOP_FAULT;
                        result->linear_pc = x86_core_machine_linear_pc(machine);
                        result->detail = machine->fault_detail;
                        result->elapsed_ticks = machine->elapsed_ticks;
                        return LIB_STATUS_INTERNAL_ERROR;
                    }
                    if (machine->external_cycle_round_ticks != 0u || event_ticks != 0u) {
                        x86_core_machine_begin_retirement_wait(machine,
                            machine->external_cycle_round_ticks, event_ticks, 0u, LIB_FALSE);
                        continue;
                    }
                    if (machine->fault_detail != 0u)
                        return x86_core_machine_complete_cpu_fault(machine, result);
                    if (!x86_core_cpu_is_shutdown(machine->executor_cpu_execution)) {
                        machine->lifecycle = X86_CORE_MACHINE_PAUSED;
                        result->reason = X86_CORE_MACHINE_STOP_BUDGET;
                        result->linear_pc = x86_core_machine_linear_pc(machine);
                        result->elapsed_ticks = machine->elapsed_ticks;
                        return LIB_STATUS_OK;
                    }
                }
                if (x86_core_cpu_is_shutdown(machine->executor_cpu_execution)) {
                    if (x86_core_machine_finish_shutdown(machine, result))
                        return x86_core_machine_complete_run_boundary(machine, result);
                    continue;
                }
                if (x86_core_cpu_execution_consume_wait_stall(
                        machine->executor_cpu_execution)) {
                    /* Unretired WAIT or system-bus/power work leaves time
                     * with the existing device/board deadline owner. */
                    machine->lifecycle = X86_CORE_MACHINE_PAUSED;
                    result->reason = X86_CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT;
                    result->linear_pc = x86_core_machine_linear_pc(machine);
                    result->elapsed_ticks = machine->elapsed_ticks;
                    return x86_core_machine_complete_run_boundary(machine, result);
                }
                if (was_halted && x86_core_cpu_is_halted(
                        machine->executor_cpu_execution)) {
                    machine->lifecycle = X86_CORE_MACHINE_PAUSED;
                    result->reason = X86_CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT;
                    result->linear_pc = x86_core_machine_linear_pc(machine);
                    result->elapsed_ticks = machine->elapsed_ticks;
                    return x86_core_machine_complete_run_boundary(machine, result);
                }
                if (machine->fault_detail == 0u &&
                    machine->transaction_contract.cpu_prefetch_reservation_enabled &&
                    !x86_core_cpu_is_system_waiting(machine->executor_cpu_execution)) {
                    x86_core_cpu_execution_reserve_prefetch(
                        machine->executor_cpu_execution);
                }
            }
            {
                const x86_core_cpu_timing_result timing_result =
                    x86_core_cpu_capture_timing(machine->executor_cpu_execution);
                lib_u64 instruction_ticks;

                if (timing_result.ticks == 0u ||
                    machine->external_cycle_round_overflow) {
                    (void)x86_core_machine_report_fault(machine, 0x54494d45u);
                    result->reason = X86_CORE_MACHINE_STOP_FAULT;
                    result->linear_pc = x86_core_machine_linear_pc(machine);
                    result->detail = machine->fault_detail;
                    result->elapsed_ticks = machine->elapsed_ticks;
                    return LIB_STATUS_INTERNAL_ERROR;
                }
                instruction_ticks = timing_result.ticks;
                if (!x86_core_cpu_timing_add_ticks(&instruction_ticks, timing_result.event_ticks) ||
                    !x86_core_cpu_timing_add_ticks(&instruction_ticks,
                        machine->external_cycle_round_ticks) ||
                    LIB_UINT64_MAX - result->ticks < instruction_ticks ||
                    LIB_UINT64_MAX - machine->elapsed_ticks < instruction_ticks) {
                    (void)x86_core_machine_report_fault(machine, 0x54494d45u);
                    result->reason = X86_CORE_MACHINE_STOP_FAULT;
                    result->linear_pc = x86_core_machine_linear_pc(machine);
                    result->detail = machine->fault_detail;
                    result->elapsed_ticks = machine->elapsed_ticks;
                    return LIB_STATUS_INTERNAL_ERROR;
                }
                x86_core_machine_retirement_observation_capture_eligibility_key(machine);
                if (machine->external_cycle_round_ticks != 0u || timing_result.event_ticks != 0u) {
                    x86_core_machine_begin_retirement_wait(machine,
                        machine->external_cycle_round_ticks,
                        instruction_ticks - machine->external_cycle_round_ticks,
                        instruction_ticks, LIB_TRUE);
                    continue;
                }
                machine->cpu_retirement_source_ticks = instruction_ticks;
                if (!x86_core_machine_publish_successful_retirement(machine)) {
                    (void)x86_core_machine_report_fault(machine, 0x54494d55u);
                    result->reason = X86_CORE_MACHINE_STOP_FAULT;
                    result->linear_pc = x86_core_machine_linear_pc(machine);
                    result->detail = machine->fault_detail;
                    result->elapsed_ticks = machine->elapsed_ticks;
                    return LIB_STATUS_INTERNAL_ERROR;
                }
                ++result->executed;
                result->ticks += instruction_ticks;
                if (x86_core_machine_publish_elapsed_ticks(machine,
                        instruction_ticks,
                        X86_CORE_MACHINE_TIME_PUBLICATION_CPU_RETIREMENT) !=
                    LIB_STATUS_OK) {
                    (void)x86_core_machine_report_fault(machine, 0x54494d45u);
                    result->reason = X86_CORE_MACHINE_STOP_FAULT;
                    result->linear_pc = x86_core_machine_linear_pc(machine);
                    result->detail = machine->fault_detail;
                    result->elapsed_ticks = machine->elapsed_ticks;
                    return LIB_STATUS_INTERNAL_ERROR;
                }
                machine->cpu_retirement_source_ticks = 0u;
                result->elapsed_ticks = machine->elapsed_ticks;
                if (machine->fault_detail != 0u)
                    return x86_core_machine_complete_cpu_fault(machine, result);
            }
            if (x86_core_cpu_execution_consume_debug_pause_request(
                    machine->executor_cpu_execution)) {
                machine->lifecycle = X86_CORE_MACHINE_PAUSED;
                result->reason = X86_CORE_MACHINE_STOP_PAUSED;
                result->linear_pc = x86_core_machine_linear_pc(machine);
                return x86_core_machine_complete_run_boundary(machine, result);
            }
            x86_core_cpu_execution_poll_wait(machine->executor_cpu_execution);
            if (x86_core_cpu_is_halted(machine->executor_cpu_execution) ||
                x86_core_cpu_is_system_waiting(machine->executor_cpu_execution)) {
                machine->lifecycle = X86_CORE_MACHINE_PAUSED;
                result->reason = X86_CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT;
                result->linear_pc = x86_core_machine_linear_pc(machine);
                return x86_core_machine_complete_run_boundary(machine, result);
            }
        }
        machine->lifecycle = X86_CORE_MACHINE_PAUSED;
        result->reason = X86_CORE_MACHINE_STOP_BUDGET;
        result->linear_pc = x86_core_machine_linear_pc(machine);
        return x86_core_machine_complete_run_boundary(machine, result);
    }
}

lib_status x86_core_machine_advance_time(x86_core_machine *machine,
    lib_u64 source_ticks)
{
    if (machine == LIB_NULL || machine->retirement_time_contract ==
        X86_CORE_MACHINE_RETIREMENT_TIME_PHYSICAL ||
        !x86_core_machine_mutable_operation_is_allowed(machine) ||
        (machine->lifecycle != X86_CORE_MACHINE_STOPPED &&
        machine->lifecycle != X86_CORE_MACHINE_PAUSED)) {
        return LIB_STATUS_INVALID_STATE;
    }
    return x86_core_machine_publish_elapsed_ticks(machine, source_ticks,
        X86_CORE_MACHINE_TIME_PUBLICATION_DETERMINISTIC_ADVANCE);
}

lib_status x86_core_machine_advance_to_next_deadline(x86_core_machine *machine,
    lib_u8 *out_advanced)
{
    x86_core_machine_time_observation observation;
    lib_status status;

    if (machine == LIB_NULL || out_advanced == LIB_NULL ||
        !x86_core_machine_mutable_operation_is_allowed(machine) ||
        (machine->lifecycle != X86_CORE_MACHINE_STOPPED &&
        machine->lifecycle != X86_CORE_MACHINE_PAUSED)) {
        return LIB_STATUS_INVALID_STATE;
    }
    *out_advanced = LIB_FALSE;
    x86_core_machine_capture_time_observation_private(machine, &observation);
    if (!observation.next_deadline_valid ||
        observation.next_deadline_tick <= observation.elapsed_ticks) {
        return LIB_STATUS_OK;
    }
    status = x86_core_machine_publish_elapsed_ticks(machine,
        observation.next_deadline_tick - observation.elapsed_ticks,
        X86_CORE_MACHINE_TIME_PUBLICATION_DEADLINE);
    if (status != LIB_STATUS_OK) return status;
    *out_advanced = LIB_TRUE;
    return LIB_STATUS_OK;
}

lib_status x86_core_machine_advance_l1_compatibility(x86_core_machine *machine,
    lib_u8 *out_advanced)
{
    x86_core_machine_time_observation observation;
    lib_u8 step;

    if (machine == LIB_NULL || out_advanced == LIB_NULL ||
        !x86_core_machine_mutable_operation_is_allowed(machine) ||
        (machine->lifecycle != X86_CORE_MACHINE_STOPPED &&
        machine->lifecycle != X86_CORE_MACHINE_PAUSED)) {
        return LIB_STATUS_INVALID_STATE;
    }
    *out_advanced = LIB_FALSE;
    if (machine->l1_compatibility_policy !=
            X86_CORE_MACHINE_L1_COMPATIBILITY_BOUNDED_PROGRESS ||
        machine->retirement_time_contract == X86_CORE_MACHINE_RETIREMENT_TIME_PHYSICAL) {
        return LIB_STATUS_OK;
    }
    /* This is a host-control bound, not an emulated duration. Each iteration
     * follows the normal Core scheduler and reconsiders every known event. */
    for (step = 0u; step < X86_CORE_MACHINE_L1_COMPATIBILITY_MAXIMUM_STEPS; ++step) {
        x86_core_machine_capture_time_observation_private(machine, &observation);
        if (observation.progress_disposition !=
            X86_CORE_MACHINE_TIME_PROGRESS_L1_COMPATIBILITY) {
            return LIB_STATUS_OK;
        }
        if (x86_core_machine_publish_elapsed_ticks(machine, 1u,
                X86_CORE_MACHINE_TIME_PUBLICATION_L1_COMPATIBILITY) != LIB_STATUS_OK) {
            return LIB_STATUS_INVALID_STATE;
        }
        *out_advanced = LIB_TRUE;
    }
    return LIB_STATUS_OK;
}

lib_status x86_core_machine_request_stop(x86_core_machine *machine)
{
    if (machine == LIB_NULL) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    if (!x86_core_machine_mutable_operation_is_allowed(machine)) return LIB_STATUS_INVALID_STATE;

    lib_atomic_i32_store_explicit(&machine->stop_requested, 1, LIB_MEMORY_ORDER_RELEASE);
    return LIB_STATUS_OK;
}

lib_bool x86_core_machine_signal_nmi(x86_core_machine *machine)
{
    return machine != LIB_NULL &&
        x86_core_cpu_request_nmi(machine->executor_cpu_execution);
}

void x86_core_machine_signal_processor_reset(x86_core_machine *machine)
{
    if (machine != LIB_NULL)
        x86_core_cpu_execution_request_reset(machine->executor_cpu_execution);
}

lib_status x86_core_machine_signal_soft_processor_reset(x86_core_machine *machine)
{
    if (machine == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    return x86_core_cpu_execution_request_soft_reset(machine->executor_cpu_execution);
}

lib_status x86_core_machine_set_smi(x86_core_machine *machine, lib_bool asserted)
{
    if (machine == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    return x86_core_cpu_set_smi(machine->executor_cpu_execution, asserted);
}

lib_status x86_core_machine_set_stop_clock(x86_core_machine *machine, lib_bool asserted)
{
    if (machine == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    return x86_core_cpu_set_stop_clock(machine->executor_cpu_execution, asserted);
}

lib_status x86_core_machine_set_clock_running(x86_core_machine *machine, lib_bool running)
{
    if (machine == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    return x86_core_cpu_set_clock_running(machine->executor_cpu_execution, running);
}

lib_status x86_core_machine_set_flush(x86_core_machine *machine, lib_bool asserted)
{
    if (machine == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    return x86_core_cpu_set_flush(machine->executor_cpu_execution, asserted);
}

lib_status x86_core_machine_set_nmi(x86_core_machine *machine, lib_bool asserted)
{
    if (machine == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    return x86_core_cpu_set_nmi(machine->executor_cpu_execution, asserted);
}

lib_status x86_core_machine_set_nmi_mask(x86_core_machine *machine, lib_i32 masked)
{
    if (machine == LIB_NULL || !x86_core_machine_mutable_operation_is_allowed(machine)) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    x86_core_cpu_set_nmi_mask(machine->executor_cpu_execution,
        masked ? LIB_TRUE : LIB_FALSE);
    if (!masked && machine->attachment.refresh_nmi != LIB_NULL) {
        machine->attachment.refresh_nmi(machine->attachment.context);
    }
    return LIB_STATUS_OK;
}

lib_status x86_core_machine_get_nmi_mask(const x86_core_machine *machine,
    lib_i32 *out_masked)
{
    if (machine == LIB_NULL || out_masked == LIB_NULL) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    *out_masked = x86_core_cpu_nmi_is_masked(machine->executor_cpu_execution);
    return LIB_STATUS_OK;
}

lib_status x86_core_machine_report_fault(
    x86_core_machine *machine,
    lib_u32 detail)
{
    if (machine == LIB_NULL || !x86_core_machine_mutable_operation_is_allowed(machine)) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }

    if (machine->lifecycle != X86_CORE_MACHINE_STOPPED &&
        machine->lifecycle != X86_CORE_MACHINE_PAUSED &&
        machine->lifecycle != X86_CORE_MACHINE_RUNNING) {
        return LIB_STATUS_INVALID_STATE;
    }

    machine->fault_detail = detail;
    machine->lifecycle = X86_CORE_MACHINE_FAULTED;
    x86_core_machine_trace_record(machine, X86_CORE_MACHINE_TRACE_FAULT, 0u, 0u, detail);
    return LIB_STATUS_OK;
}

void x86_core_machine_destroy(x86_core_machine *machine)
{
    if (machine != LIB_NULL) {
        machine->firmware_context.active = 0;
        machine->firmware_context.machine = LIB_NULL;
        machine->firmware_provider = LIB_NULL;
        machine->firmware_provider_context = LIB_NULL;
        x86_core_cpu_execution_finalize(machine->executor_cpu_execution);
        if (machine->attachment.finalize_devices != LIB_NULL) {
            machine->attachment.finalize_devices(machine->attachment.context);
        }
        machine->executor_memory.connect.cpu_snoop = LIB_NULL;
        machine->executor_memory.connect.cpu_snoop_context = LIB_NULL;
        x86_core_cpu_destroy(machine->executor_cpu_execution);
        x86_fpu_destroy(machine->fpu);
        x86_core_machine_port_finalize(&machine->executor_port);
        x86_core_machine_rollback_immutable_rom_mappings(machine, 0u);
        x86_core_machine_memory_finalize(&machine->executor_memory);
    }
    x86_core_machine_trace_finalize(machine);
    lib_release(machine);
}
