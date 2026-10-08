#include "lib/types/types_interface.h"

#include "x86/core/machine.h"
#include "x86/chips/cpu/cpu_interface.h"

#define CORE_MACHINE_L1_COMPATIBILITY_MAXIMUM_STEPS 16u

_Static_assert(CORE_MACHINE_TIMING_CAPABILITY_PRODUCT_DEBUG + 1u ==
    CORE_MACHINE_TIMING_CAPABILITY_COUNT,
    "timing capability count must match the frozen T433 universe");

lib_u32 core_machine_linear_pc(const core_machine *machine)
{
    return core_machine_cpu_linear_pc(machine->executor_cpu_execution);
}

static lib_i32 core_machine_retirement_qualification_contains(
    const core_machine *machine);


/* Both immediate and externally delayed successful retirements meet here.
 * CPU timing selection is complete before this seam; board-cycle time has
 * already been added by the caller and never enters cpu_timing.c. */
static lib_i32 core_machine_publish_successful_retirement(core_machine *machine)
{
    if (machine == LIB_NULL) return 0;
    core_machine_retirement_observation_publish(machine,
        machine->cpu_retirement_source_ticks);
    return machine->retirement_time_contract != CORE_MACHINE_RETIREMENT_TIME_PHYSICAL ||
        (machine->time_axis.kind == CORE_MACHINE_TIME_AXIS_VERIFIED_PHYSICAL &&
         !core_machine_cpu_capture_timing(machine->executor_cpu_execution).
             source_timing_unallocated &&
         core_machine_retirement_qualification_contains(machine));
}

static core_machine_cpu_profile core_machine_resolve_cpu_profile(
    core_machine_cpu_profile profile)
{
    return profile == CORE_MACHINE_CPU_PROFILE_DEFAULT ?
        CORE_MACHINE_CPU_PROFILE_80386 : profile;
}

static lib_u32 core_machine_resolve_ticks_per_instruction(lib_u32 ticks)
{
    return ticks == 0u ? 1u : ticks;
}

static void core_machine_resolve_instruction_timing(
    core_machine_instruction_timing *out_timing,
    const core_machine_instruction_timing *timing, lib_u32 legacy_base)
{
    *out_timing = *timing;
    if (out_timing->base_ticks == 0u) {
        out_timing->base_ticks = core_machine_resolve_ticks_per_instruction(
            legacy_base);
    }
}

static lib_i32 core_machine_retirement_qualification_contains(
    const core_machine *machine)
{
    lib_size index;

    if (machine == LIB_NULL || !machine->retirement_eligibility_key_valid) return 0;
    for (index = 0u; index < machine->retirement_qualification_count; ++index) {
        const core_machine_retirement_eligibility_key *candidate =
            &machine->retirement_qualification[index];
        const core_machine_retirement_eligibility_key *key =
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
static lib_i32 core_machine_valid_cpu_profile(core_machine_cpu_profile profile)
{
    return profile >= CORE_MACHINE_CPU_PROFILE_8086 &&
        profile <= CORE_MACHINE_CPU_PROFILE_80386;
}

static lib_i32 core_machine_valid_fpu_profile(x86_fpu_profile profile)
{
    return profile >= X86_FPU_PROFILE_NONE &&
        profile <= X86_FPU_PROFILE_80387;
}

static lib_size core_machine_resolve_memory_bytes(
    const core_machine_executor_config *config)
{
    return config->memory_bytes == 0u ?
        CORE_MACHINE_DEFAULT_MEMORY_BYTES : config->memory_bytes;
}

lib_i32 core_machine_configuration_is_open(const core_machine *machine)
{
    return machine != LIB_NULL &&
        machine->lifecycle == CORE_MACHINE_INITIALIZED &&
        !machine->execution_provider_frozen && !machine->firmware_operation_active;
}

lib_i32 core_machine_mutable_operation_is_allowed(const core_machine *machine)
{
    return machine != LIB_NULL && !machine->firmware_operation_active;
}

lib_status core_machine_bind_attachment(core_machine *machine,
    const core_machine_attachment *attachment)
{
    if (machine == LIB_NULL || attachment == LIB_NULL ||
            attachment->context == LIB_NULL ||
            attachment->finalize_devices == LIB_NULL) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    if (!core_machine_configuration_is_open(machine) ||
            machine->attachment.context != LIB_NULL) {
        return LIB_STATUS_INVALID_STATE;
    }
    machine->attachment = *attachment;
    return LIB_STATUS_OK;
}

lib_status core_machine_bind_execution_provider(core_machine *machine,
    const core_machine_execution_provider *provider, void *context)
{
    if (!core_machine_configuration_is_open(machine)) {
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

lib_status core_machine_freeze_execution_providers(core_machine *machine)
{
    if (!core_machine_configuration_is_open(machine)) {
        return LIB_STATUS_INVALID_STATE;
    }
    machine->execution_provider_frozen = 1;
    core_machine_memory_freeze_mappings(&machine->executor_memory);
    return LIB_STATUS_OK;
}

lib_status core_machine_get_cpu_state(
    const core_machine *machine,
    core_machine_cpu_state *out_state)
{
    if (machine == LIB_NULL || out_state == LIB_NULL) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }

    if (machine->lifecycle == CORE_MACHINE_INITIALIZED ||
        machine->lifecycle == CORE_MACHINE_RUNNING) {
        return LIB_STATUS_INVALID_STATE;
    }
    core_machine_cpu_capture_state(machine->executor_cpu_execution, out_state);
    return LIB_STATUS_OK;
}

lib_status core_machine_get_cpu_profile(
    const core_machine *machine, core_machine_cpu_profile *out_profile)
{
    if (machine == LIB_NULL || out_profile == LIB_NULL) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    *out_profile = machine->cpu_profile;
    return LIB_STATUS_OK;
}

lib_status core_machine_get_fpu_profile(
    const core_machine *machine, x86_fpu_profile *out_profile)
{
    if (machine == LIB_NULL || out_profile == LIB_NULL) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    *out_profile = x86_fpu_get_profile(machine->fpu);
    return LIB_STATUS_OK;
}

lib_status core_machine_get_fpu_state(
    const core_machine *machine, x86_fpu_state *out_state)
{
    if (machine == LIB_NULL || out_state == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    x86_fpu_get_state(machine->fpu, out_state);
    return LIB_STATUS_OK;
}

lib_status core_machine_get_memory_bytes(
    const core_machine *machine, lib_size *out_memory_bytes)
{
    if (machine == LIB_NULL || out_memory_bytes == LIB_NULL) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    *out_memory_bytes = machine->executor_memory.connect.installed_bytes;
    return LIB_STATUS_OK;
}

lib_status core_machine_get_elapsed_ticks(
    const core_machine *machine, lib_u64 *out_elapsed_ticks)
{
    if (machine == LIB_NULL || out_elapsed_ticks == LIB_NULL ||
        machine->lifecycle == CORE_MACHINE_INITIALIZED ||
        machine->lifecycle == CORE_MACHINE_RUNNING) {
        return LIB_STATUS_INVALID_STATE;
    }
    *out_elapsed_ticks = machine->elapsed_ticks;
    return LIB_STATUS_OK;
}

lib_status core_machine_capture_time_observation(const core_machine *machine,
    core_machine_time_observation *out_observation)
{
    if (machine == LIB_NULL || out_observation == LIB_NULL ||
        machine->lifecycle == CORE_MACHINE_INITIALIZED ||
        machine->lifecycle == CORE_MACHINE_RUNNING) {
        return LIB_STATUS_INVALID_STATE;
    }
    core_machine_capture_time_observation_private(machine, out_observation);
    return LIB_STATUS_OK;
}

lib_status core_machine_get_timeline_observation(const core_machine *machine,
    core_machine_timeline_observation *out_observation)
{
    if (machine == LIB_NULL || out_observation == LIB_NULL) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    out_observation->now = machine->timeline.now;
    out_observation->next_sequence = machine->timeline.next_sequence;
    out_observation->pending_events = core_machine_timeline_pending_count(
        &machine->timeline);
    return LIB_STATUS_OK;
}

lib_status core_machine_get_cpu_diagnostic(
    const core_machine *machine, core_machine_cpu_diagnostic *out_diagnostic)
{
    if (machine == LIB_NULL || out_diagnostic == LIB_NULL ||
        machine->lifecycle == CORE_MACHINE_INITIALIZED ||
        machine->lifecycle == CORE_MACHINE_RUNNING) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    core_machine_cpu_diagnostic_capture(machine, out_diagnostic);
    return LIB_STATUS_OK;
}

lib_status core_machine_capture_observation(
    const core_machine *machine, core_machine_observation *out_observation)
{
    if (machine == LIB_NULL || out_observation == LIB_NULL ||
        machine->lifecycle == CORE_MACHINE_INITIALIZED ||
        machine->lifecycle == CORE_MACHINE_RUNNING) {
        return LIB_STATUS_INVALID_STATE;
    }
    out_observation->lifecycle = machine->lifecycle;
    out_observation->elapsed_ticks = machine->elapsed_ticks;
    if (core_machine_get_cpu_state(machine, &out_observation->cpu) !=
            LIB_STATUS_OK) {
        return LIB_STATUS_INVALID_STATE;
    }
    core_machine_cpu_diagnostic_capture(machine, &out_observation->diagnostic);
    return LIB_STATUS_OK;
}

lib_i32 core_machine_retirement_time_contract_is_valid(
    core_machine_retirement_time_contract contract)
{
    return contract == CORE_MACHINE_RETIREMENT_TIME_DETERMINISTIC ||
        contract == CORE_MACHINE_RETIREMENT_TIME_PHYSICAL;
}

lib_i32 core_machine_timing_capability_is_valid(
    core_machine_timing_capability capability)
{
    return capability >= CORE_MACHINE_TIMING_CAPABILITY_CPU_EXEC &&
        capability <= CORE_MACHINE_TIMING_CAPABILITY_PRODUCT_DEBUG;
}

lib_i32 core_machine_external_cycle_timing_is_valid(
    const core_machine_external_cycle_timing *timing)
{
    if (timing == LIB_NULL || (timing->overlap_policy !=
        CORE_MACHINE_EXTERNAL_CYCLE_OVERLAP_DISABLED && timing->overlap_policy !=
        CORE_MACHINE_EXTERNAL_CYCLE_OVERLAP_EXPLICIT_SEQUENTIAL)) return 0;
    if (timing->page_bytes == 0u) {
        return timing->page_miss_ticks == 0u && timing->page_hit_ticks == 0u &&
            timing->overlap_policy == CORE_MACHINE_EXTERNAL_CYCLE_OVERLAP_DISABLED;
    }
    return (timing->page_bytes & (timing->page_bytes - 1u)) == 0u &&
        ((timing->first_eligible_address == 0u &&
          timing->last_eligible_address == 0u) ||
         timing->first_eligible_address <= timing->last_eligible_address);
}

lib_i32 core_machine_external_access_wait_windows_are_valid(
    const core_machine_external_access_wait_window *windows)
{
    lib_size index;

    if (windows == LIB_NULL) return 0;
    for (index = 0u; index < CORE_MACHINE_EXTERNAL_ACCESS_WAIT_WINDOW_CAPACITY;
            ++index) {
        const core_machine_external_access_wait_window *window = &windows[index];
        if (window->wait_ticks == 0u) continue;
        if ((window->space != CORE_MACHINE_CPU_EXTERNAL_CYCLE_SPACE_MEMORY &&
                window->space != CORE_MACHINE_CPU_EXTERNAL_CYCLE_SPACE_PORT) ||
            window->first_address > window->last_address ||
            (window->space == CORE_MACHINE_CPU_EXTERNAL_CYCLE_SPACE_PORT &&
                window->last_address > 0xffffu)) return 0;
    }
    return 1;
}

lib_i32 core_machine_transaction_contract_is_valid(
    const core_machine_transaction_contract *contract)
{
    return contract != LIB_NULL &&
        core_machine_external_cycle_timing_is_valid(
            &contract->external_cycle_timing) &&
        core_machine_external_access_wait_windows_are_valid(
            contract->external_access_wait_windows) &&
        (contract->dma_cycle_bus_ready_gate_enabled == LIB_FALSE ||
         contract->dma_cycle_bus_ready_gate_enabled == LIB_TRUE) &&
        (contract->cpu_cycle_bus_ready_gate_enabled == LIB_FALSE ||
         contract->cpu_cycle_bus_ready_gate_enabled == LIB_TRUE) &&
        (contract->cpu_prefetch_reservation_enabled == LIB_FALSE ||
         contract->cpu_prefetch_reservation_enabled == LIB_TRUE);
}

lib_i32 core_machine_neutral_config_is_valid(
    const core_machine_executor_config *config)
{
    return config != LIB_NULL &&
        core_machine_valid_cpu_profile(
            core_machine_resolve_cpu_profile(config->cpu_profile)) &&
        core_machine_valid_fpu_profile(config->fpu_profile) &&
        (config->a20_wrap_policy == CORE_MACHINE_A20_WRAP_GLOBAL_MASK ||
         config->a20_wrap_policy == CORE_MACHINE_A20_WRAP_FIRST_TO_SECOND_MIB) &&
        core_machine_clock_ratio_is_valid(&config->provider_clock) &&
        core_machine_retirement_time_contract_is_valid(
            config->retirement_time_contract) &&
        core_machine_transaction_contract_is_valid(
            &config->transaction_contract) &&
        (config->time_axis.kind == CORE_MACHINE_TIME_AXIS_UNQUALIFIED ||
         config->time_axis.kind == CORE_MACHINE_TIME_AXIS_MACRO_PROPORTIONAL ||
         config->time_axis.kind == CORE_MACHINE_TIME_AXIS_VERIFIED_PHYSICAL) &&
        (config->l1_compatibility_policy == CORE_MACHINE_L1_COMPATIBILITY_DISABLED ||
         config->l1_compatibility_policy == CORE_MACHINE_L1_COMPATIBILITY_BOUNDED_PROGRESS) &&
        (config->time_axis.kind == CORE_MACHINE_TIME_AXIS_UNQUALIFIED ?
            config->time_axis.ticks_per_second == 0u :
            config->time_axis.ticks_per_second != 0u) &&
        (config->retirement_time_contract != CORE_MACHINE_RETIREMENT_TIME_PHYSICAL ||
         config->time_axis.kind == CORE_MACHINE_TIME_AXIS_VERIFIED_PHYSICAL);
}


lib_status core_machine_neutral_create_with_test_allocation(
    const core_machine_executor_config *config,
    core_machine_memory_test_allocation *test_allocation,
    core_machine_port_test_allocation *port_test_allocation,
    core_machine **out_machine)
{
    core_machine *machine;
    core_machine_instruction_timing instruction_timing;
    lib_size memory_bytes;

    if (out_machine == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    *out_machine = LIB_NULL;
    if (!core_machine_neutral_config_is_valid(config))
        return LIB_STATUS_INVALID_ARGUMENT;
    memory_bytes = core_machine_resolve_memory_bytes(config);

    machine = (core_machine *)lib_allocate_zero(1u, sizeof(*machine));
    if (machine == LIB_NULL) {
        return LIB_STATUS_NO_MEMORY;
    }

    machine->lifecycle = CORE_MACHINE_INITIALIZED;
    machine->cpu_profile = core_machine_resolve_cpu_profile(config->cpu_profile);
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
                CORE_MACHINE_RETIREMENT_QUALIFICATION_CAPACITY) {
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
        machine->cpu_profile != CORE_MACHINE_CPU_PROFILE_80386) {
        lib_release(machine);
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    if (core_machine_timeline_initialize(&machine->timeline) != LIB_STATUS_OK) {
        lib_release(machine);
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    core_machine_resolve_instruction_timing(&instruction_timing,
        &config->instruction_timing, config->ticks_per_instruction);
    machine->maximum_instruction_ticks = core_machine_cpu_timing_maximum_ticks(
        machine->cpu_profile, &instruction_timing);
    if (core_machine_clock_domain_initialize(&machine->provider_clock,
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
    core_machine_trace_initialize(machine);
    core_machine_transaction_initialize(&machine->transaction);
    core_machine_transaction_bind_trace(&machine->transaction,
        core_machine_transaction_trace, machine);
    core_machine_cpu_diagnostic_initialize(machine);
    core_machine_retirement_observation_initialize(machine);

    {
        lib_status status = core_machine_cpu_create(&core_machine_cpu_bus,
            machine, &machine->executor_cpu_execution);

        if (status != LIB_STATUS_OK) {
            x86_fpu_destroy(machine->fpu);
            lib_release(machine);
            return status;
        }
    }
    core_machine_cpu_execution_context_bind_profiles(
        machine->executor_cpu_execution, machine->cpu_profile,
        x86_fpu_get_profile(machine->fpu), machine->cpu_80386_cr_mov_ignores_mod,
        &instruction_timing);
    core_machine_cpu_execution_context_bind_fpu(
        machine->executor_cpu_execution, machine->fpu);
    core_machine_cpu_execution_context_bind_external_cycle_provider(
        machine->executor_cpu_execution, core_machine_cpu_external_cycle_trace,
        machine);
    core_machine_cpu_execution_context_bind_diagnostic_provider(
        machine->executor_cpu_execution,
        machine->retirement_time_contract == CORE_MACHINE_RETIREMENT_TIME_PHYSICAL ?
            &core_machine_cpu_diagnostic_provider :
            &core_machine_cpu_fault_diagnostic_provider,
        machine);
    core_machine_cpu_state_initialize(machine->executor_cpu_execution);
    core_machine_port_initialize(&machine->executor_port);
    core_machine_port_set_test_allocation(&machine->executor_port,
        port_test_allocation);
    if (core_machine_memory_initialize_for(&machine->executor_memory,
            memory_bytes, test_allocation) != LIB_STATUS_OK) {
        core_machine_destroy(machine);
        return LIB_STATUS_NO_MEMORY;
    }
    if (core_machine_memory_set_a20_wrap_policy(&machine->executor_memory,
            config->a20_wrap_policy) != LIB_STATUS_OK) {
        core_machine_destroy(machine);
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    *out_machine = machine;
    return LIB_STATUS_OK;
}


lib_status core_machine_neutral_create(
    const core_machine_executor_config *config, core_machine **out_machine)
{
    return core_machine_neutral_create_with_test_allocation(config,
        LIB_NULL, LIB_NULL, out_machine);
}

lib_status core_machine_get_timing_disposition(const core_machine *machine,
    core_machine_timing_capability capability,
    core_machine_timing_disposition *out_disposition)
{
    if (machine == LIB_NULL || out_disposition == LIB_NULL ||
        !machine->timing_declarations_copied ||
        !core_machine_timing_capability_is_valid(capability)) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    *out_disposition = machine->timing_declarations[capability].disposition;
    return LIB_STATUS_OK;
}

lib_status core_machine_validate_timing_declarations(
    const core_machine_timing_declaration *declarations, lib_size count)
{
    lib_bool seen[CORE_MACHINE_TIMING_CAPABILITY_COUNT] = {LIB_FALSE};
    lib_size index;

    if (declarations == LIB_NULL || count != CORE_MACHINE_TIMING_CAPABILITY_COUNT)
        return LIB_STATUS_INVALID_ARGUMENT;
    for (index = 0u; index < count; ++index) {
        const core_machine_timing_declaration *declaration = &declarations[index];

        if (!core_machine_timing_capability_is_valid(declaration->capability) ||
            declaration->disposition < CORE_MACHINE_TIMING_DISPOSITION_L2_FALLBACK ||
            declaration->disposition > CORE_MACHINE_TIMING_DISPOSITION_L3_REQUIRED ||
            declaration->seam < CORE_MACHINE_TIMING_SEAM_CPU_PROGRAM ||
            declaration->seam > CORE_MACHINE_TIMING_SEAM_OBSERVATION ||
            seen[declaration->capability]) return LIB_STATUS_INVALID_ARGUMENT;
        seen[declaration->capability] = LIB_TRUE;
    }
    return LIB_STATUS_OK;
}

lib_status core_machine_install_timing_declarations(core_machine *machine,
    const core_machine_timing_declaration *declarations, lib_size count)
{
    lib_size index;

    if (!core_machine_configuration_is_open(machine) ||
        machine->timing_declarations_copied) return LIB_STATUS_INVALID_STATE;
    if (core_machine_validate_timing_declarations(declarations, count) !=
            LIB_STATUS_OK) return LIB_STATUS_INVALID_ARGUMENT;
    for (index = 0u; index < count; ++index)
        machine->timing_declarations[declarations[index].capability] = declarations[index];
    machine->timing_declarations_copied = LIB_TRUE;
    return LIB_STATUS_OK;
}

lib_status core_machine_get_timing_declaration(const core_machine *machine,
    core_machine_timing_capability capability,
    core_machine_timing_declaration *out_declaration)
{
    if (machine == LIB_NULL || out_declaration == LIB_NULL ||
        !machine->timing_declarations_copied ||
        !core_machine_timing_capability_is_valid(capability)) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    *out_declaration = machine->timing_declarations[capability];
    return LIB_STATUS_OK;
}

static lib_status core_machine_cold_reset(core_machine *machine)
{
    lib_status status;
    core_machine_cpu_state_reset(machine->executor_cpu_execution);
    x86_fpu_reset(machine->fpu);
    core_machine_port_reset(&machine->executor_port);
    core_machine_memory_reset(&machine->executor_memory);
    if (machine->attachment.reset_devices != LIB_NULL) {
        machine->attachment.reset_devices(machine->attachment.context);
    }

    lib_atomic_i32_store_explicit(&machine->stop_requested, 0, LIB_MEMORY_ORDER_RELEASE);
    machine->fault_detail = 0u;
    machine->elapsed_ticks = 0u;
    machine->dma_cycle_wait_remaining = 0u;
    machine->dma_cycle_bus_ready = LIB_TRUE;
    machine->cpu_cycle_bus_ready = LIB_TRUE;
    machine->external_cycle_page_tag = 0u;
    machine->external_cycle_round_ticks = 0u;
    machine->cpu_retirement_wait_ticks = 0u;
    machine->cpu_retirement_completion_ticks = 0u;
    machine->cpu_retirement_source_ticks = 0u;
    machine->external_cycle_page_valid = LIB_FALSE;
    machine->external_cycle_pending_valid = LIB_FALSE;
    machine->external_cycle_pending_space = CORE_MACHINE_CPU_EXTERNAL_CYCLE_SPACE_MEMORY;
    machine->external_cycle_pending_physical = 0u;
    machine->external_cycle_pending_bytes = 0u;
    machine->external_cycle_pending_write = LIB_FALSE;
    machine->external_cycle_pending_provenance =
        CORE_MACHINE_CPU_MEMORY_ACCESS_DATA;
    machine->external_cycle_overlap_valid = LIB_FALSE;
    machine->external_cycle_overlap_next_physical = 0u;
    machine->external_cycle_round_overflow = LIB_FALSE;
    machine->cpu_retirement_wait_pending = LIB_FALSE;
    machine->retirement_eligibility_key_valid = LIB_FALSE;
    core_machine_transaction_reset(&machine->transaction);
    core_machine_timeline_reset(&machine->timeline);
    if (machine->attachment.reset_clocks != LIB_NULL) {
        machine->attachment.reset_clocks(machine->attachment.context);
    }
    core_machine_clock_domain_reset(&machine->provider_clock);
    machine->entry_plan_applied = LIB_FALSE;
    core_machine_cpu_diagnostic_reset(machine);
    core_machine_retirement_observation_reset(machine);
    if (machine->execution_provider != LIB_NULL &&
        machine->execution_provider->reset != LIB_NULL) {
        machine->execution_provider->reset(machine->execution_provider_context);
    }
    if (machine->firmware_provider != LIB_NULL) {
        status = core_machine_firmware_invoke(machine, 0, 1,
            machine->firmware_provider->reset);
        if (status != LIB_STATUS_OK) {
            machine->lifecycle = CORE_MACHINE_INITIALIZED;
            return status;
        }
    }
    machine->lifecycle = CORE_MACHINE_STOPPED;
    core_machine_trace_record(machine, CORE_MACHINE_TRACE_RESET, 0u, 0u, 0u);
    return LIB_STATUS_OK;
}

/* An 8042 reset pulse resets the processor only.  Preserve RAM, board state
 * and scheduled device work; discard only incomplete CPU execution state. */
static void core_machine_processor_reset(core_machine *machine)
{
    if (machine == LIB_NULL) return;
    core_machine_cpu_state_reset(machine->executor_cpu_execution);
    machine->cpu_retirement_wait_pending = LIB_FALSE;
    machine->cpu_retirement_wait_ticks = 0u;
    machine->cpu_retirement_completion_ticks = 0u;
    machine->cpu_retirement_source_ticks = 0u;
    machine->external_cycle_page_valid = LIB_FALSE;
    machine->external_cycle_pending_valid = LIB_FALSE;
    machine->external_cycle_round_ticks = 0u;
    machine->external_cycle_round_overflow = LIB_FALSE;
    machine->external_cycle_overlap_valid = LIB_FALSE;
    machine->retirement_eligibility_key_valid = LIB_FALSE;
}

lib_status core_machine_reconfigure_memory(core_machine *machine,
    lib_size memory_bytes)
{
    lib_uptr index;
    lib_status status;

    if (machine == LIB_NULL || !core_machine_mutable_operation_is_allowed(machine) ||
        !machine->execution_provider_frozen ||
        machine->lifecycle != CORE_MACHINE_STOPPED ||
        memory_bytes < CORE_MACHINE_MINIMUM_MEMORY_BYTES ||
        memory_bytes > CORE_MACHINE_MAXIMUM_MEMORY_BYTES) {
        return LIB_STATUS_INVALID_STATE;
    }
    if (machine->attachment.memory_admission != LIB_NULL) {
        status = machine->attachment.memory_admission(machine->attachment.context,
            memory_bytes);
        if (status != LIB_STATUS_OK) return status;
    }
    for (index = 0u; index < machine->executor_memory.connect.mapping_count;
            ++index) {
        const core_machine_memory_mapping *mapping =
            &machine->executor_memory.connect.mappings[index];
        if (mapping->backing_start > memory_bytes ||
            mapping->bytes > memory_bytes - mapping->backing_start) {
            return LIB_STATUS_INVALID_ARGUMENT;
        }
    }
    if (core_machine_memory_allocate_for(&machine->executor_memory,
            memory_bytes) != LIB_STATUS_OK) {
        return LIB_STATUS_NO_MEMORY;
    }
    return core_machine_cold_reset(machine);
}

lib_status core_machine_reset(core_machine *machine)
{
    if (machine == LIB_NULL) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }

    if (!core_machine_mutable_operation_is_allowed(machine) ||
        !machine->execution_provider_frozen ||
        machine->lifecycle == CORE_MACHINE_RUNNING) {
        return LIB_STATUS_INVALID_STATE;
    }

    return core_machine_cold_reset(machine);
}

lib_status core_machine_get_lifecycle(
    const core_machine *machine,
    core_machine_lifecycle *out_lifecycle)
{
    if (machine == LIB_NULL || out_lifecycle == LIB_NULL) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }

    *out_lifecycle = machine->lifecycle;
    return LIB_STATUS_OK;
}

static lib_status core_machine_complete_run_boundary(core_machine *machine,
    core_machine_run_result *result)
{
    if (machine->firmware_provider != LIB_NULL &&
        machine->firmware_provider->after_run != LIB_NULL &&
        core_machine_firmware_invoke(machine, 0, 0,
            machine->firmware_provider->after_run) != LIB_STATUS_OK) {
        (void)core_machine_report_fault(machine, 0x46575245u);
        result->reason = CORE_MACHINE_STOP_FAULT;
        result->detail = machine->fault_detail;
        return LIB_STATUS_INTERNAL_ERROR;
    }
    if (lib_atomic_i32_load_explicit(&machine->stop_requested, LIB_MEMORY_ORDER_ACQUIRE)) {
        result->reason = CORE_MACHINE_STOP_REQUESTED;
    }
    core_machine_trace_record(machine, CORE_MACHINE_TRACE_RUN_BOUNDARY,
        result->linear_pc, (lib_u32)result->executed,
        (lib_u32)result->reason);
    return LIB_STATUS_OK;
}

lib_status core_machine_run(
    core_machine *machine,
    core_machine_run_budget budget,
    core_machine_run_result *result)
{
    if (machine == LIB_NULL || result == LIB_NULL ||
        !core_machine_mutable_operation_is_allowed(machine)) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }

    result->reason = CORE_MACHINE_STOP_NONE;
    result->executed = 0u;
    result->ticks = 0u;
    result->elapsed_ticks = machine->elapsed_ticks;
    result->linear_pc = core_machine_linear_pc(machine);
    result->detail = 0u;

    if (machine->lifecycle == CORE_MACHINE_FAULTED) {
        result->reason = CORE_MACHINE_STOP_FAULT;
        result->detail = machine->fault_detail;
        return LIB_STATUS_INTERNAL_ERROR;
    }

    if (machine->lifecycle != CORE_MACHINE_STOPPED &&
        machine->lifecycle != CORE_MACHINE_PAUSED) {
        return LIB_STATUS_INVALID_STATE;
    }

    if (budget.instructions == 0u && budget.ticks == 0u) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }

    if (lib_atomic_i32_load_explicit(&machine->stop_requested, LIB_MEMORY_ORDER_ACQUIRE)) {
        lib_status status = core_machine_cold_reset(machine);
        if (status != LIB_STATUS_OK) return status;
        result->reason = CORE_MACHINE_STOP_REQUESTED;
        result->linear_pc = core_machine_linear_pc(machine);
        core_machine_trace_record(machine, CORE_MACHINE_TRACE_STOP, 0u, 0u,
                               (lib_u32)result->reason);
        return LIB_STATUS_OK;
    }

    machine->lifecycle = CORE_MACHINE_RUNNING;
    {
        while ((budget.instructions == 0u ||
                result->executed < budget.instructions) &&
               (budget.ticks == 0u || result->ticks < budget.ticks)) {
            if (machine->lifecycle == CORE_MACHINE_FAULTED) {
                result->reason = CORE_MACHINE_STOP_FAULT;
                result->linear_pc = core_machine_linear_pc(machine);
                result->detail = machine->fault_detail;
                return LIB_STATUS_INTERNAL_ERROR;
            }
            /* Only an explicit board binding converts shutdown to CPU reset. */
            if (machine->attachment.shutdown_reset != LIB_NULL &&
                machine->attachment.shutdown_reset(machine->attachment.context) &&
                core_machine_cpu_execution_consume_shutdown_request(
                    machine->executor_cpu_execution)) {
                machine->lifecycle = CORE_MACHINE_PAUSED;
                core_machine_processor_reset(machine);
                machine->lifecycle = CORE_MACHINE_STOPPED;
                result->reason = CORE_MACHINE_STOP_RESET_REQUESTED;
                result->linear_pc = core_machine_linear_pc(machine);
                return LIB_STATUS_OK;
            }
            if (core_machine_cpu_execution_consume_debug_pause_request(
                    machine->executor_cpu_execution)) {
                machine->lifecycle = CORE_MACHINE_PAUSED;
                result->reason = CORE_MACHINE_STOP_PAUSED;
                result->linear_pc = core_machine_linear_pc(machine);
                return core_machine_complete_run_boundary(machine, result);
            }
            if (lib_atomic_i32_load_explicit(&machine->stop_requested, LIB_MEMORY_ORDER_ACQUIRE) ||
                core_machine_cpu_execution_consume_stop_request(
                    machine->executor_cpu_execution)) {
                machine->lifecycle = CORE_MACHINE_PAUSED;
                {
                    lib_status status = core_machine_cold_reset(machine);
                    if (status != LIB_STATUS_OK) return status;
                }
                result->reason = CORE_MACHINE_STOP_REQUESTED;
                result->linear_pc = core_machine_linear_pc(machine);
                core_machine_trace_record(machine, CORE_MACHINE_TRACE_STOP, 0u,
                    0u, (lib_u32)result->reason);
                return LIB_STATUS_OK;
            }
            if (core_machine_cpu_execution_consume_reset_request(
                    machine->executor_cpu_execution)) {
                machine->lifecycle = CORE_MACHINE_PAUSED;
                core_machine_processor_reset(machine);
                machine->lifecycle = CORE_MACHINE_STOPPED;
                result->reason = CORE_MACHINE_STOP_RESET_REQUESTED;
                result->linear_pc = core_machine_linear_pc(machine);
                return LIB_STATUS_OK;
            }
            if (machine->cpu_retirement_wait_pending) {
                if (machine->transaction_contract.cpu_cycle_bus_ready_gate_enabled &&
                    !machine->cpu_cycle_bus_ready) {
                    if (result->ticks == LIB_UINT64_MAX || machine->elapsed_ticks == LIB_UINT64_MAX) {
                        (void)core_machine_report_fault(machine, 0x54494d45u);
                        result->reason = CORE_MACHINE_STOP_FAULT;
                        result->linear_pc = core_machine_linear_pc(machine);
                        result->detail = machine->fault_detail;
                        return LIB_STATUS_INTERNAL_ERROR;
                    }
                    ++result->ticks;
                    if (core_machine_publish_elapsed_ticks(machine, 1u,
                            CORE_MACHINE_TIME_PUBLICATION_EXTERNAL_WAIT) !=
                        LIB_STATUS_OK) return LIB_STATUS_INTERNAL_ERROR;
                    result->elapsed_ticks = machine->elapsed_ticks;
                    continue;
                }
                if (machine->cpu_retirement_wait_ticks != 0u) {
                    if (result->ticks == LIB_UINT64_MAX ||
                        machine->elapsed_ticks == LIB_UINT64_MAX) {
                        (void)core_machine_report_fault(machine, 0x54494d45u);
                        result->reason = CORE_MACHINE_STOP_FAULT;
                        result->linear_pc = core_machine_linear_pc(machine);
                        result->detail = machine->fault_detail;
                        result->elapsed_ticks = machine->elapsed_ticks;
                        return LIB_STATUS_INTERNAL_ERROR;
                    }
                    ++result->ticks;
                    --machine->cpu_retirement_wait_ticks;
                    if (core_machine_publish_elapsed_ticks(machine, 1u,
                            CORE_MACHINE_TIME_PUBLICATION_EXTERNAL_WAIT) !=
                        LIB_STATUS_OK) return LIB_STATUS_INTERNAL_ERROR;
                    result->elapsed_ticks = machine->elapsed_ticks;
                if (core_machine_cpu_is_halted(machine->executor_cpu_execution)) {
                    machine->lifecycle = CORE_MACHINE_PAUSED;
                    result->reason = CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT;
                    result->linear_pc = core_machine_linear_pc(machine);
                    return core_machine_complete_run_boundary(machine, result);
                }
                    continue;
                }
                if (budget.ticks != 0u && machine->cpu_retirement_completion_ticks >
                    budget.ticks - result->ticks) {
                    machine->lifecycle = CORE_MACHINE_PAUSED;
                    result->reason = CORE_MACHINE_STOP_BUDGET;
                    result->linear_pc = core_machine_linear_pc(machine);
                    result->elapsed_ticks = machine->elapsed_ticks;
                    return LIB_STATUS_OK;
                }
                if (LIB_UINT64_MAX - result->ticks < machine->cpu_retirement_completion_ticks ||
                    LIB_UINT64_MAX - machine->elapsed_ticks <
                        machine->cpu_retirement_completion_ticks) {
                    (void)core_machine_report_fault(machine, 0x54494d45u);
                    result->reason = CORE_MACHINE_STOP_FAULT;
                    result->linear_pc = core_machine_linear_pc(machine);
                    result->detail = machine->fault_detail;
                    result->elapsed_ticks = machine->elapsed_ticks;
                    return LIB_STATUS_INTERNAL_ERROR;
                }
                if (!core_machine_publish_successful_retirement(machine)) {
                    (void)core_machine_report_fault(machine, 0x54494d55u);
                    result->reason = CORE_MACHINE_STOP_FAULT;
                    result->linear_pc = core_machine_linear_pc(machine);
                    result->detail = machine->fault_detail;
                    result->elapsed_ticks = machine->elapsed_ticks;
                    return LIB_STATUS_INTERNAL_ERROR;
                }
                ++result->executed;
                result->ticks += machine->cpu_retirement_completion_ticks;
                if (core_machine_publish_elapsed_ticks(machine,
                        machine->cpu_retirement_completion_ticks,
                        CORE_MACHINE_TIME_PUBLICATION_CPU_RETIREMENT) !=
                    LIB_STATUS_OK) return LIB_STATUS_INTERNAL_ERROR;
                machine->cpu_retirement_wait_pending = LIB_FALSE;
                machine->cpu_retirement_completion_ticks = 0u;
                machine->cpu_retirement_source_ticks = 0u;
                result->elapsed_ticks = machine->elapsed_ticks;
                continue;
            }
            if (budget.ticks != 0u && machine->maximum_instruction_ticks >
                budget.ticks - result->ticks) {
                machine->lifecycle = CORE_MACHINE_PAUSED;
                result->reason = CORE_MACHINE_STOP_BUDGET;
                result->linear_pc = core_machine_linear_pc(machine);
                result->elapsed_ticks = machine->elapsed_ticks;
                return core_machine_complete_run_boundary(machine, result);
            }
            {
                lib_bool was_halted = core_machine_cpu_is_halted(
                    machine->executor_cpu_execution);

                machine->external_cycle_round_ticks = 0u;
                /* A completed instruction round cannot inherit an undeclared
                 * external-cycle overlap into the next CPU refresh. */
                core_machine_external_cycle_invalidate(machine);
                core_machine_cpu_execution_refresh(machine->executor_cpu_execution);
                if (machine->lifecycle == CORE_MACHINE_FAULTED) {
                    result->reason = CORE_MACHINE_STOP_FAULT;
                    result->linear_pc = core_machine_linear_pc(machine);
                    result->detail = machine->fault_detail;
                    result->elapsed_ticks = machine->elapsed_ticks;
                    return LIB_STATUS_INTERNAL_ERROR;
                }
                if (core_machine_cpu_is_shutdown(machine->executor_cpu_execution)) {
                    /* Return through the loop once so an explicit board reset
                     * can consume the notification before generic waiting. */
                    if (machine->attachment.shutdown_reset != LIB_NULL &&
                        machine->attachment.shutdown_reset(machine->attachment.context))
                        continue;
                    machine->lifecycle = CORE_MACHINE_PAUSED;
                    result->reason = CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT;
                    result->detail = VCPUINS_EXCEPT_SHUTDOWN;
                    result->linear_pc = core_machine_linear_pc(machine);
                    result->elapsed_ticks = machine->elapsed_ticks;
                    return core_machine_complete_run_boundary(machine, result);
                }
                if (core_machine_cpu_execution_consume_instruction_fault_delivery(
                        machine->executor_cpu_execution)) {
                    /* The synchronous exception frame and vector are committed, but
                     * the faulting instruction did not retire.  The handler starts
                     * at the next public execution round, without publishing CPU
                     * or device time for this faulting round. */
                    machine->lifecycle = CORE_MACHINE_PAUSED;
                    result->reason = CORE_MACHINE_STOP_BUDGET;
                    result->linear_pc = core_machine_linear_pc(machine);
                    result->elapsed_ticks = machine->elapsed_ticks;
                    return LIB_STATUS_OK;
                }
                if (core_machine_cpu_execution_consume_wait_stall(
                        machine->executor_cpu_execution)) {
                    /* An automatic 80286 ESC wait has restored its unretired
                     * instruction. The existing FPU deadline owns elapsed
                     * guest time; no synthetic retirement occurs here. */
                    machine->lifecycle = CORE_MACHINE_PAUSED;
                    result->reason = CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT;
                    result->linear_pc = core_machine_linear_pc(machine);
                    result->elapsed_ticks = machine->elapsed_ticks;
                    return core_machine_complete_run_boundary(machine, result);
                }
                if (was_halted && core_machine_cpu_is_halted(
                        machine->executor_cpu_execution)) {
                    machine->lifecycle = CORE_MACHINE_PAUSED;
                    result->reason = CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT;
                    result->linear_pc = core_machine_linear_pc(machine);
                    result->elapsed_ticks = machine->elapsed_ticks;
                    return core_machine_complete_run_boundary(machine, result);
                }
                if (machine->transaction_contract.cpu_prefetch_reservation_enabled) {
                    core_machine_cpu_execution_reserve_prefetch(
                        machine->executor_cpu_execution);
                }
            }
            {
                core_machine_cpu_timing_result timing_result;
                lib_u64 instruction_ticks;

                if (!core_machine_cpu_timing_select(
                        machine->executor_cpu_execution, &timing_result) ||
                    machine->external_cycle_round_overflow) {
                    (void)core_machine_report_fault(machine, 0x54494d45u);
                    result->reason = CORE_MACHINE_STOP_FAULT;
                    result->linear_pc = core_machine_linear_pc(machine);
                    result->detail = machine->fault_detail;
                    result->elapsed_ticks = machine->elapsed_ticks;
                    return LIB_STATUS_INTERNAL_ERROR;
                }
                instruction_ticks = timing_result.ticks;
                if (!core_machine_timing_add_ticks(&instruction_ticks,
                        machine->external_cycle_round_ticks) ||
                    LIB_UINT64_MAX - result->ticks < instruction_ticks ||
                    LIB_UINT64_MAX - machine->elapsed_ticks < instruction_ticks) {
                    (void)core_machine_report_fault(machine, 0x54494d45u);
                    result->reason = CORE_MACHINE_STOP_FAULT;
                    result->linear_pc = core_machine_linear_pc(machine);
                    result->detail = machine->fault_detail;
                    result->elapsed_ticks = machine->elapsed_ticks;
                    return LIB_STATUS_INTERNAL_ERROR;
                }
                core_machine_retirement_observation_capture_eligibility_key(machine);
                if (machine->external_cycle_round_ticks != 0u) {
                    machine->cpu_retirement_wait_pending = LIB_TRUE;
                    machine->cpu_retirement_wait_ticks = machine->external_cycle_round_ticks;
                    machine->cpu_retirement_completion_ticks = instruction_ticks - machine->external_cycle_round_ticks;
                    machine->cpu_retirement_source_ticks = instruction_ticks;
                    continue;
                }
                machine->cpu_retirement_source_ticks = instruction_ticks;
                if (!core_machine_publish_successful_retirement(machine)) {
                    (void)core_machine_report_fault(machine, 0x54494d55u);
                    result->reason = CORE_MACHINE_STOP_FAULT;
                    result->linear_pc = core_machine_linear_pc(machine);
                    result->detail = machine->fault_detail;
                    result->elapsed_ticks = machine->elapsed_ticks;
                    return LIB_STATUS_INTERNAL_ERROR;
                }
                ++result->executed;
                result->ticks += instruction_ticks;
                if (core_machine_publish_elapsed_ticks(machine,
                        instruction_ticks,
                        CORE_MACHINE_TIME_PUBLICATION_CPU_RETIREMENT) !=
                    LIB_STATUS_OK) {
                    (void)core_machine_report_fault(machine, 0x54494d45u);
                    result->reason = CORE_MACHINE_STOP_FAULT;
                    result->linear_pc = core_machine_linear_pc(machine);
                    result->detail = machine->fault_detail;
                    result->elapsed_ticks = machine->elapsed_ticks;
                    return LIB_STATUS_INTERNAL_ERROR;
                }
                machine->cpu_retirement_source_ticks = 0u;
                result->elapsed_ticks = machine->elapsed_ticks;
            }
            if (core_machine_cpu_execution_consume_debug_pause_request(
                    machine->executor_cpu_execution)) {
                machine->lifecycle = CORE_MACHINE_PAUSED;
                result->reason = CORE_MACHINE_STOP_PAUSED;
                result->linear_pc = core_machine_linear_pc(machine);
                return core_machine_complete_run_boundary(machine, result);
            }
            core_machine_cpu_execution_poll_wait(machine->executor_cpu_execution);
            if (core_machine_cpu_is_halted(machine->executor_cpu_execution)) {
                machine->lifecycle = CORE_MACHINE_PAUSED;
                result->reason = CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT;
                result->linear_pc = core_machine_linear_pc(machine);
                return core_machine_complete_run_boundary(machine, result);
            }
        }
        machine->lifecycle = CORE_MACHINE_PAUSED;
        result->reason = CORE_MACHINE_STOP_BUDGET;
        result->linear_pc = core_machine_linear_pc(machine);
        return core_machine_complete_run_boundary(machine, result);
    }
}

lib_status core_machine_advance_time(core_machine *machine,
    lib_u64 source_ticks)
{
    if (machine == LIB_NULL || machine->retirement_time_contract ==
        CORE_MACHINE_RETIREMENT_TIME_PHYSICAL ||
        !core_machine_mutable_operation_is_allowed(machine) ||
        (machine->lifecycle != CORE_MACHINE_STOPPED &&
        machine->lifecycle != CORE_MACHINE_PAUSED)) {
        return LIB_STATUS_INVALID_STATE;
    }
    return core_machine_publish_elapsed_ticks(machine, source_ticks,
        CORE_MACHINE_TIME_PUBLICATION_DETERMINISTIC_ADVANCE);
}

lib_status core_machine_advance_to_next_deadline(core_machine *machine,
    lib_u8 *out_advanced)
{
    core_machine_time_observation observation;
    lib_status status;

    if (machine == LIB_NULL || out_advanced == LIB_NULL ||
        !core_machine_mutable_operation_is_allowed(machine) ||
        (machine->lifecycle != CORE_MACHINE_STOPPED &&
        machine->lifecycle != CORE_MACHINE_PAUSED)) {
        return LIB_STATUS_INVALID_STATE;
    }
    *out_advanced = LIB_FALSE;
    core_machine_capture_time_observation_private(machine, &observation);
    if (!observation.next_deadline_valid ||
        observation.next_deadline_tick <= observation.elapsed_ticks) {
        return LIB_STATUS_OK;
    }
    status = core_machine_publish_elapsed_ticks(machine,
        observation.next_deadline_tick - observation.elapsed_ticks,
        CORE_MACHINE_TIME_PUBLICATION_DEADLINE);
    if (status != LIB_STATUS_OK) return status;
    *out_advanced = LIB_TRUE;
    return LIB_STATUS_OK;
}

lib_status core_machine_advance_l1_compatibility(core_machine *machine,
    lib_u8 *out_advanced)
{
    core_machine_time_observation observation;
    lib_u8 step;

    if (machine == LIB_NULL || out_advanced == LIB_NULL ||
        !core_machine_mutable_operation_is_allowed(machine) ||
        (machine->lifecycle != CORE_MACHINE_STOPPED &&
        machine->lifecycle != CORE_MACHINE_PAUSED)) {
        return LIB_STATUS_INVALID_STATE;
    }
    *out_advanced = LIB_FALSE;
    if (machine->l1_compatibility_policy !=
            CORE_MACHINE_L1_COMPATIBILITY_BOUNDED_PROGRESS ||
        machine->retirement_time_contract == CORE_MACHINE_RETIREMENT_TIME_PHYSICAL) {
        return LIB_STATUS_OK;
    }
    /* This is a host-control bound, not an emulated duration. Each iteration
     * follows the normal Core scheduler and reconsiders every known event. */
    for (step = 0u; step < CORE_MACHINE_L1_COMPATIBILITY_MAXIMUM_STEPS; ++step) {
        core_machine_capture_time_observation_private(machine, &observation);
        if (observation.progress_disposition !=
            CORE_MACHINE_TIME_PROGRESS_L1_COMPATIBILITY) {
            return LIB_STATUS_OK;
        }
        if (core_machine_publish_elapsed_ticks(machine, 1u,
                CORE_MACHINE_TIME_PUBLICATION_L1_COMPATIBILITY) != LIB_STATUS_OK) {
            return LIB_STATUS_INVALID_STATE;
        }
        *out_advanced = LIB_TRUE;
    }
    return LIB_STATUS_OK;
}

lib_status core_machine_request_stop(core_machine *machine)
{
    if (machine == LIB_NULL) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    if (!core_machine_mutable_operation_is_allowed(machine)) return LIB_STATUS_INVALID_STATE;

    lib_atomic_i32_store_explicit(&machine->stop_requested, 1, LIB_MEMORY_ORDER_RELEASE);
    return LIB_STATUS_OK;
}

lib_bool core_machine_signal_nmi(core_machine *machine)
{
    return machine != LIB_NULL &&
        core_machine_cpu_request_nmi(machine->executor_cpu_execution);
}

void core_machine_signal_processor_reset(core_machine *machine)
{
    if (machine != LIB_NULL)
        core_machine_cpu_execution_request_reset(machine->executor_cpu_execution);
}

lib_status core_machine_set_nmi_mask(core_machine *machine, lib_i32 masked)
{
    if (machine == LIB_NULL || !core_machine_mutable_operation_is_allowed(machine)) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    core_machine_cpu_set_nmi_mask(machine->executor_cpu_execution,
        masked ? LIB_TRUE : LIB_FALSE);
    if (!masked && machine->attachment.refresh_nmi != LIB_NULL) {
        machine->attachment.refresh_nmi(machine->attachment.context);
    }
    return LIB_STATUS_OK;
}

lib_status core_machine_get_nmi_mask(const core_machine *machine,
    lib_i32 *out_masked)
{
    if (machine == LIB_NULL || out_masked == LIB_NULL) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    *out_masked = core_machine_cpu_nmi_is_masked(machine->executor_cpu_execution);
    return LIB_STATUS_OK;
}

lib_status core_machine_report_fault(
    core_machine *machine,
    lib_u32 detail)
{
    if (machine == LIB_NULL || !core_machine_mutable_operation_is_allowed(machine)) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }

    if (machine->lifecycle != CORE_MACHINE_STOPPED &&
        machine->lifecycle != CORE_MACHINE_PAUSED &&
        machine->lifecycle != CORE_MACHINE_RUNNING) {
        return LIB_STATUS_INVALID_STATE;
    }

    machine->fault_detail = detail;
    machine->lifecycle = CORE_MACHINE_FAULTED;
    core_machine_trace_record(machine, CORE_MACHINE_TRACE_FAULT, 0u, 0u, detail);
    return LIB_STATUS_OK;
}

void core_machine_destroy(core_machine *machine)
{
    if (machine != LIB_NULL) {
        machine->firmware_context.active = 0;
        machine->firmware_context.machine = LIB_NULL;
        machine->firmware_provider = LIB_NULL;
        machine->firmware_provider_context = LIB_NULL;
        if (machine->attachment.finalize_devices != LIB_NULL) {
            machine->attachment.finalize_devices(machine->attachment.context);
        }
        core_machine_cpu_destroy(machine->executor_cpu_execution);
        x86_fpu_destroy(machine->fpu);
        core_machine_port_finalize(&machine->executor_port);
        core_machine_rollback_immutable_rom_mappings(machine, 0u);
        core_machine_memory_finalize(&machine->executor_memory);
    }
    core_machine_trace_finalize(machine);
    lib_release(machine);
}
