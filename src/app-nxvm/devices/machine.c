#include "lib/types/types_interface.h"

#include "app-nxvm/devices/machine.h"
#include "app-nxvm/devices/cpu_timing.h"

#define CORE_MACHINE_L1_COMPATIBILITY_MAXIMUM_STEPS 16u

_Static_assert(CORE_MACHINE_TIMING_CAPABILITY_PRODUCT_DEBUG + 1u ==
    CORE_MACHINE_TIMING_CAPABILITY_COUNT,
    "timing capability count must match the frozen T433 universe");

lib_u32 core_machine_linear_pc(const core_machine *machine)
{
    return machine->executor_cpu.data.cs.base + machine->executor_cpu.data.eip;
}

static lib_i32 core_machine_retirement_qualification_contains(
    const core_machine *machine);

static lib_u8 core_machine_xt_ppi_request_nmi(void *owner)
{
    core_machine *machine = (core_machine *)owner;

    if (machine == LIB_NULL || machine->executor_cpu.data.flagMaskNMI) return LIB_FALSE;
    machine->executor_cpu.data.flagNMI = LIB_TRUE;
    return LIB_TRUE;
}

static void core_machine_xt_ppi_update_speaker(void *owner,
    lib_u8 timer_gate, lib_u8 data_enabled)
{
    core_machine_board_set_xt_ppi_speaker((core_machine *)owner, timer_gate,
        data_enabled);
}

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
         !machine->source_timing_unallocated &&
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

static lib_i32 core_machine_valid_fpu_profile(core_machine_fpu_profile profile)
{
    return profile >= CORE_MACHINE_FPU_PROFILE_NONE &&
        profile <= CORE_MACHINE_FPU_PROFILE_80387;
}

static lib_u32 core_machine_cpu_reset_rom_alias(
    core_machine_cpu_profile profile)
{
    switch (profile) {
    case CORE_MACHINE_CPU_PROFILE_80286:
        return 0x00ff0000u;
    case CORE_MACHINE_CPU_PROFILE_80386:
        return 0xffff0000u;
    case CORE_MACHINE_CPU_PROFILE_8086:
    case CORE_MACHINE_CPU_PROFILE_8088:
    case CORE_MACHINE_CPU_PROFILE_80186:
    case CORE_MACHINE_CPU_PROFILE_DEFAULT:
        return 0u;
    }
    return 0u;
}

static lib_i32 core_machine_cpu_reset_rom_is_present(const core_machine *machine)
{
    lib_size index;

    if (machine == LIB_NULL) return 0;
    for (index = 0u; index < machine->immutable_rom_mapping_count; ++index) {
        const core_machine_immutable_rom_mapping *mapping =
            &machine->immutable_rom_mappings[index];

        /* A reset alias is meaningful only when the actual reset prefetch
         * window comes from F0000h ROM.  A short unrelated F0000h alias must
         * not turn on a high-ROM provider which cannot serve the reset CPU. */
        if (0x000ffff0u >= mapping->physical_start &&
            (lib_u64)0x000ffff0u - mapping->physical_start + 15u <=
                mapping->bytes) return 1;
    }
    return 0;
}

static lib_i32 core_machine_cpu_reset_rom_alias_is_present(const core_machine *machine,
    lib_u32 reset_alias)
{
    lib_size index;

    if (machine == LIB_NULL) return 0;
    for (index = 0u; index < machine->immutable_rom_mapping_count; ++index) {
        const core_machine_immutable_rom_mapping *mapping =
            &machine->immutable_rom_mappings[index];

        if (reset_alias <= UINT32_MAX - 0xfff0u &&
            reset_alias + 0xfff0u >= mapping->physical_start &&
            (lib_u64)(reset_alias + 0xfff0u) - mapping->physical_start + 16u <=
                mapping->bytes) return 1;
    }
    return 0;
}

lib_i32 core_machine_configuration_is_open(const core_machine *machine)
{
    return machine != LIB_NULL &&
        machine->lifecycle == CORE_MACHINE_INITIALIZED &&
        !machine->execution_provider_frozen && !machine->firmware_operation_active;
}

lib_status core_machine_register_reset_rom_alias(core_machine *machine)
{
    lib_u32 reset_alias;
    lib_size index;
    lib_i32 copied = 0;

    if (machine == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    reset_alias = core_machine_cpu_reset_rom_alias(machine->cpu_profile);
    if (reset_alias == 0u || !core_machine_cpu_reset_rom_is_present(machine)) {
        return LIB_STATUS_OK;
    }
    if (core_machine_cpu_reset_rom_alias_is_present(machine, reset_alias)) {
        return LIB_STATUS_OK;
    }
    for (index = 0u; index < machine->immutable_rom_mapping_count; ++index) {
        const core_machine_immutable_rom_mapping *mapping =
            &machine->immutable_rom_mappings[index];
        lib_u32 source_start;
        lib_u64 source_end = (lib_u64)mapping->physical_start +
            mapping->bytes;
        lib_u64 copy_end;
        lib_status status;

        if (source_end <= 0x000f0000u || mapping->physical_start >= 0x00100000u) {
            continue;
        }
        source_start = mapping->physical_start < 0x000f0000u ?
            0x000f0000u : mapping->physical_start;
        copy_end = source_end < 0x00100000u ? source_end : 0x00100000u;
        status = core_machine_register_immutable_rom_mapping_reset_alias(machine,
            source_start, reset_alias + (source_start - 0x000f0000u),
            (lib_size)(copy_end - source_start));
        if (status != LIB_STATUS_OK) return status;
        copied = 1;
    }
    return copied ? LIB_STATUS_OK : LIB_STATUS_INVALID_ARGUMENT;
}

lib_i32 core_machine_mutable_operation_is_allowed(const core_machine *machine)
{
    return machine != LIB_NULL && !machine->firmware_operation_active;
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
    out_state->cs = machine->executor_cpu.data.cs.selector;
    out_state->cs_base = machine->executor_cpu.data.cs.base;
    out_state->eip = machine->executor_cpu.data.eip;
    out_state->eflags = machine->executor_cpu.data.eflags;
    out_state->halted = machine->executor_cpu.data.flagHalt;
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
    const core_machine *machine, core_machine_fpu_profile *out_profile)
{
    if (machine == LIB_NULL || out_profile == LIB_NULL) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    *out_profile = machine->fpu.profile;
    return LIB_STATUS_OK;
}

lib_status core_machine_get_fpu_state(
    const core_machine *machine, core_machine_fpu_state *out_state)
{
    if (machine == LIB_NULL || out_state == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    core_machine_fpu_get_state(&machine->fpu, out_state);
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

static lib_status core_machine_create_internal(
    const core_machine_config *config,
    core_machine **out_machine,
    core_machine_memory_test_allocation *test_allocation,
    core_machine_port_test_allocation *port_test_allocation)
{
    core_machine *machine;
    core_machine_port_provider_entry *port_checkpoint;
    lib_size memory_bytes;
    lib_u8 dma_controller_count;
    if (out_machine == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    *out_machine = LIB_NULL;
    if (config == LIB_NULL ||
        !core_machine_valid_cpu_profile(
            core_machine_resolve_cpu_profile(config->cpu_profile)) ||
        !core_machine_valid_fpu_profile(config->fpu_profile) ||
        (config->a20_wrap_policy != CORE_MACHINE_A20_WRAP_GLOBAL_MASK &&
        config->a20_wrap_policy != CORE_MACHINE_A20_WRAP_FIRST_TO_SECOND_MIB) ||
        !core_machine_clock_plan_is_valid(&config->clock_plan) ||
        !core_machine_retirement_time_contract_is_valid(
            config->retirement_time_contract) ||
        !core_machine_transaction_contract_is_valid(
            &config->transaction_contract) ||
        (config->shared_pit_personality != CORE_MACHINE_PIT_PERSONALITY_8254 &&
        config->shared_pit_personality != CORE_MACHINE_PIT_PERSONALITY_8253) ||
        (config->auxiliary_pit_present != LIB_FALSE &&
        config->auxiliary_pit_present != LIB_TRUE) ||
        (config->pic_topology != CORE_MACHINE_PIC_TOPOLOGY_CASCADED &&
        config->pic_topology != CORE_MACHINE_PIC_TOPOLOGY_SINGLE) ||
        (config->dma_controller_count > CORE_MACHINE_DMA_CONTROLLER_COUNT) ||
        (config->keyboard_topology != CORE_MACHINE_KEYBOARD_TOPOLOGY_8042 &&
        config->keyboard_topology != CORE_MACHINE_KEYBOARD_TOPOLOGY_XT_PPI) ||
        (config->keyboard_topology == CORE_MACHINE_KEYBOARD_TOPOLOGY_XT_PPI &&
        !core_machine_xt_ppi_keyboard_config_is_valid(&config->xt_ppi_keyboard)) ||
        (config->auxiliary_pit_present && config->auxiliary_pit_base_port > 0xfffcu)) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    if ((config->time_axis.kind != CORE_MACHINE_TIME_AXIS_UNQUALIFIED &&
        config->time_axis.kind != CORE_MACHINE_TIME_AXIS_MACRO_PROPORTIONAL &&
        config->time_axis.kind != CORE_MACHINE_TIME_AXIS_VERIFIED_PHYSICAL) ||
        (config->l1_compatibility_policy != CORE_MACHINE_L1_COMPATIBILITY_DISABLED &&
        config->l1_compatibility_policy !=
            CORE_MACHINE_L1_COMPATIBILITY_BOUNDED_PROGRESS) ||
        (config->time_axis.kind == CORE_MACHINE_TIME_AXIS_UNQUALIFIED &&
        config->time_axis.ticks_per_second != 0u) ||
        (config->time_axis.kind != CORE_MACHINE_TIME_AXIS_UNQUALIFIED &&
        config->time_axis.ticks_per_second == 0u) ||
        (config->retirement_time_contract == CORE_MACHINE_RETIREMENT_TIME_PHYSICAL &&
        config->time_axis.kind != CORE_MACHINE_TIME_AXIS_VERIFIED_PHYSICAL)) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }

    memory_bytes = config->memory_bytes == 0u ?
        CORE_MACHINE_DEFAULT_MEMORY_BYTES : config->memory_bytes;
    dma_controller_count = config->dma_controller_count == 0u ?
        CORE_MACHINE_DMA_CONTROLLER_COUNT : config->dma_controller_count;

    machine = (core_machine *)lib_allocate_zero(1u, sizeof(*machine));
    if (machine == LIB_NULL) {
        return LIB_STATUS_NO_MEMORY;
    }

    machine->lifecycle = CORE_MACHINE_INITIALIZED;
    machine->cpu_profile = core_machine_resolve_cpu_profile(config->cpu_profile);
    machine->keyboard_topology = config->keyboard_topology;
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
    core_machine_resolve_instruction_timing(&machine->instruction_timing,
        &config->instruction_timing, config->ticks_per_instruction);
    machine->maximum_instruction_ticks = core_machine_cpu_timing_maximum_ticks(
        machine->cpu_profile, &machine->instruction_timing);
    if (core_machine_clock_domain_initialize(&machine->dma_clock,
            &config->clock_plan.dma) != LIB_STATUS_OK ||
        core_machine_clock_domain_initialize(&machine->pit_clock,
            &config->clock_plan.pit) != LIB_STATUS_OK ||
        core_machine_clock_domain_initialize(&machine->auxiliary_pit_clock,
            &config->clock_plan.auxiliary_pit) != LIB_STATUS_OK ||
        core_machine_clock_domain_initialize(&machine->rtc_clock,
            &config->clock_plan.rtc) != LIB_STATUS_OK ||
        core_machine_clock_domain_initialize(&machine->vadp_clock,
            &config->clock_plan.vadp) != LIB_STATUS_OK ||
        core_machine_clock_domain_initialize(&machine->kbc_clock,
            &config->clock_plan.kbc) != LIB_STATUS_OK ||
        core_machine_clock_domain_initialize(&machine->provider_clock,
            &config->clock_plan.provider) != LIB_STATUS_OK) {
        lib_release(machine);
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    /* Zero is an explicit profile choice: without a calibrated guest-time
     * mapping, core-generated keyboard repeat must remain disabled. */
    machine->kbc_typematic_initial_ticks = config->kbc_typematic_initial_ticks;
    machine->kbc_typematic_repeat_ticks = config->kbc_typematic_repeat_ticks;
    machine->kbc_command_response_ticks = config->kbc_command_response_ticks;
    machine->kbc_command_response_status_polls =
        config->kbc_command_response_status_polls;
    machine->kbc_serial_delivery_ticks = config->kbc_serial_delivery_ticks;
    machine->kbc_input_port_configured = config->kbc_input_port_configured;
    machine->kbc_input_port = config->kbc_input_port;
    core_machine_fpu_initialize(&machine->fpu, config->fpu_profile);
    lib_atomic_i32_initialize(&machine->stop_requested, 0);
    core_machine_trace_initialize(machine);
    core_machine_transaction_initialize(&machine->transaction);
    core_machine_transaction_bind_trace(&machine->transaction,
        core_machine_transaction_trace, machine);
    core_machine_cpu_diagnostic_initialize(machine);
    core_machine_retirement_observation_initialize(machine);

    core_machine_cpu_execution_context_initialize(&machine->executor_cpu_execution,
        &machine->executor_cpu, &machine->executor_cpu_instructions,
        &machine->executor_memory, &machine->executor_port);
    core_machine_cpu_execution_context_bind_profiles(
        &machine->executor_cpu_execution, machine->cpu_profile,
        machine->fpu.profile, machine->cpu_80386_cr_mov_ignores_mod);
    core_machine_cpu_execution_context_bind_fpu(
        &machine->executor_cpu_execution, &machine->fpu);
    core_machine_cpu_execution_context_bind_external_cycle_provider(
        &machine->executor_cpu_execution, core_machine_cpu_external_cycle_trace,
        machine);
    core_machine_cpu_execution_context_bind_firmware_interrupt_provider(
        &machine->executor_cpu_execution,
        core_machine_firmware_handle_software_interrupt, machine);
    core_machine_cpu_execution_context_bind_transaction(
        &machine->executor_cpu_execution, &machine->transaction);
    core_machine_cpu_execution_context_bind_diagnostic_provider(
        &machine->executor_cpu_execution,
        machine->retirement_time_contract == CORE_MACHINE_RETIREMENT_TIME_PHYSICAL ?
            &core_machine_cpu_diagnostic_provider :
            &core_machine_cpu_fault_diagnostic_provider,
        machine);
    core_machine_cpu_state_initialize(&machine->executor_cpu_execution);
    core_machine_port_initialize(&machine->executor_port);
    core_machine_port_set_test_allocation(&machine->executor_port,
        port_test_allocation);
    if (core_machine_bus_initialize(machine) != LIB_STATUS_OK) {
        core_machine_destroy(machine);
        return LIB_STATUS_NO_MEMORY;
    }
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
    /* A firmware-less Core fixture may deliberately supply reset bytes through
     * ordinary backing RAM.  Firmware-backed machines take the reset-only ROM
     * provider route in the CPU owner before this fallback is consulted. */
    if (memory_bytes >= 0x00100000u &&
        (machine->cpu_profile == CORE_MACHINE_CPU_PROFILE_80286 ||
         machine->cpu_profile == CORE_MACHINE_CPU_PROFILE_80386) &&
        core_machine_memory_register_mapping(&machine->executor_memory,
            machine->cpu_profile == CORE_MACHINE_CPU_PROFILE_80286 ?
                0x00ff0000u : 0xffff0000u,
            0x000f0000u, 0x00010000u, LIB_FALSE) != LIB_STATUS_OK) {
        core_machine_destroy(machine);
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    port_checkpoint = core_machine_port_registration_begin(&machine->executor_port);
    core_machine_memory_register_ports(&machine->executor_memory,
        &machine->executor_port);
    core_machine_vadp_initialize(&machine->shared_vadp, &machine->executor_port);
    if (config->keyboard_topology == CORE_MACHINE_KEYBOARD_TOPOLOGY_XT_PPI) {
        if (core_machine_xt_ppi_keyboard_initialize(&machine->xt_ppi_keyboard,
                &config->xt_ppi_keyboard, &machine->executor_port) != LIB_STATUS_OK) {
            core_machine_destroy(machine);
            return LIB_STATUS_INVALID_ARGUMENT;
        }
        if (core_machine_xt_keyboard_initialize(&machine->xt_keyboard,
                &machine->xt_ppi_keyboard, config->time_axis.ticks_per_second) !=
            LIB_STATUS_OK) {
            core_machine_destroy(machine);
            return LIB_STATUS_INVALID_ARGUMENT;
        }
    } else {
        core_machine_kbc_initialize(&machine->shared_kbc, &machine->executor_port);
    }
    core_machine_dma_initialize(&machine->shared_dma_latch,
        &machine->shared_dma_primary, &machine->shared_dma_secondary,
        &machine->executor_port, dma_controller_count);
    core_machine_pic_initialize(&machine->shared_pic_master,
        &machine->shared_pic_slave, &machine->executor_port, config->pic_topology);
    core_machine_pic_set_irq_timing(&machine->shared_pic_master,
        &machine->shared_pic_slave, &config->pic_irq_timing);
    core_machine_cpu_execution_context_bind_pic(&machine->executor_cpu_execution,
        &machine->shared_pic_master, &machine->shared_pic_slave);
    core_machine_pic_irq_source_bind(&machine->shared_pit_irq0_source,
        &machine->shared_pic_master, &machine->shared_pic_slave, 0u);
    core_machine_pit_initialize_as(&machine->shared_pit, &machine->executor_port,
        config->shared_pit_personality);
    if (config->auxiliary_pit_present) {
        core_machine_pit_initialize_at(&machine->auxiliary_pit,
            &machine->executor_port, config->auxiliary_pit_base_port);
        machine->auxiliary_pit_configured = LIB_TRUE;
    }
    core_machine_pit_set_output(&machine->shared_pit, 0,
        core_machine_pic_timer_output, &machine->shared_pit_irq0_source);
    if (config->keyboard_topology == CORE_MACHINE_KEYBOARD_TOPOLOGY_XT_PPI) {
        core_machine_board_configure_xt_ppi_speaker(machine);
        core_machine_xt_ppi_keyboard_bind_pic(&machine->xt_ppi_keyboard,
            &machine->shared_pic_master, &machine->shared_pic_slave);
        core_machine_xt_ppi_keyboard_bind_nmi(&machine->xt_ppi_keyboard,
            core_machine_xt_ppi_request_nmi, machine);
        core_machine_xt_ppi_keyboard_bind_speaker(&machine->xt_ppi_keyboard,
            core_machine_xt_ppi_update_speaker, machine);
        core_machine_xt_ppi_keyboard_bind_keyboard_observer(&machine->xt_ppi_keyboard,
            core_machine_xt_keyboard_observe_ppi_lines, &machine->xt_keyboard,
            core_machine_xt_keyboard_notify_ppi_byte_released);
    } else {
        core_machine_kbc_bind_core_services(&machine->shared_kbc,
            &machine->shared_pic_master, &machine->shared_pic_slave,
            &machine->executor_memory, &machine->executor_cpu_execution,
            !config->kbc_aux_absent);
        if (config->kbc_reset_output_port_configured) {
            core_machine_kbc_set_reset_output_port(&machine->shared_kbc,
                config->kbc_reset_output_port);
        }
        core_machine_kbc_set_typematic_timing(&machine->shared_kbc,
            machine->kbc_typematic_initial_ticks,
            machine->kbc_typematic_repeat_ticks);
        core_machine_kbc_set_command_response_timing(&machine->shared_kbc,
            machine->kbc_command_response_ticks);
        core_machine_kbc_set_command_response_status_polls(&machine->shared_kbc,
            machine->kbc_command_response_status_polls);
        core_machine_kbc_set_serial_delivery_timing(&machine->shared_kbc,
            machine->kbc_serial_delivery_ticks);
    }
    core_machine_pit_set_output(&machine->shared_pit, 1, LIB_NULL, LIB_NULL);
    {
        lib_status status = core_machine_port_registration_status(
            &machine->executor_port);

        if (status != LIB_STATUS_OK) {
            core_machine_port_rollback_registration(&machine->executor_port,
                port_checkpoint);
            core_machine_destroy(machine);
            return status;
        }
    }

    *out_machine = machine;

    return LIB_STATUS_OK;
}

lib_status core_machine_create(const core_machine_config *config,
    core_machine **out_machine)
{
    return core_machine_create_internal(config, out_machine, LIB_NULL, LIB_NULL);
}

lib_status core_machine_create_from_plan(const core_machine_plan *plan,
    core_machine **out_machine)
{
    lib_status status;

    if (out_machine == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    *out_machine = LIB_NULL;
    if (core_machine_plan_validate(plan) != LIB_STATUS_OK) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    status = core_machine_create_internal(&plan->configuration, out_machine,
        LIB_NULL, LIB_NULL);
    if (status != LIB_STATUS_OK) return status;
    status = core_machine_plan_apply_topology(*out_machine, plan);
    if (status != LIB_STATUS_OK) {
        core_machine_destroy(*out_machine);
        *out_machine = LIB_NULL;
        return status;
    }
    (*out_machine)->timing_plan = *plan;
    /* Configuration-owned retirement qualification is already copied by
     * create_internal; the plan copy must retain no caller-owned pointer. */
    (*out_machine)->timing_plan.configuration.retirement_qualification = LIB_NULL;
    (*out_machine)->timing_plan_copied = LIB_TRUE;
    return LIB_STATUS_OK;
}

lib_status core_machine_get_timing_disposition(const core_machine *machine,
    core_machine_timing_capability capability,
    core_machine_timing_disposition *out_disposition)
{
    const core_machine_timing_declaration *declaration;

    if (machine == LIB_NULL || out_disposition == LIB_NULL ||
        !machine->timing_plan_copied ||
        !core_machine_timing_capability_is_valid(capability)) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    declaration = core_machine_plan_declaration_find(&machine->timing_plan,
        capability);
    if (declaration == LIB_NULL) return LIB_STATUS_INVALID_STATE;
    *out_disposition = declaration->disposition;
    return LIB_STATUS_OK;
}

lib_status core_machine_get_timing_declaration(const core_machine *machine,
    core_machine_timing_capability capability,
    core_machine_timing_declaration *out_declaration)
{
    if (machine == LIB_NULL || out_declaration == LIB_NULL ||
        !machine->timing_plan_copied ||
        !core_machine_timing_capability_is_valid(capability)) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    {
        const core_machine_timing_declaration *declaration =
            core_machine_plan_declaration_find(&machine->timing_plan, capability);

        if (declaration == LIB_NULL) return LIB_STATUS_INVALID_STATE;
        *out_declaration = *declaration;
    }
    return LIB_STATUS_OK;
}

lib_status core_machine_create_with_test_memory_allocation(
    const core_machine_config *config, core_machine **out_machine,
    core_machine_memory_test_allocation *test_allocation)
{
    return core_machine_create_internal(config, out_machine, test_allocation,
        LIB_NULL);
}

lib_status core_machine_create_with_test_port_allocation(
    const core_machine_config *config, core_machine **out_machine,
    core_machine_port_test_allocation *test_allocation)
{
    return core_machine_create_internal(config, out_machine, LIB_NULL,
        test_allocation);
}

static lib_status core_machine_cold_reset(core_machine *machine)
{
    lib_status status;
    core_machine_cpu_state_reset(&machine->executor_cpu_execution);
    core_machine_fpu_reset(&machine->fpu);
    core_machine_port_reset(&machine->executor_port);
    core_machine_memory_reset(&machine->executor_memory);
    core_machine_d4_memory_reset(machine);
    if (machine->keyboard_topology ==
            CORE_MACHINE_KEYBOARD_TOPOLOGY_XT_PPI) {
        core_machine_xt_ppi_keyboard_reset(&machine->xt_ppi_keyboard);
        core_machine_xt_keyboard_reset(&machine->xt_keyboard);
    } else {
        core_machine_kbc_reset(&machine->shared_kbc);
        if (machine->kbc_input_port_configured) {
            core_machine_kbc_set_input_port(&machine->shared_kbc,
                machine->kbc_input_port);
        }
    }
    core_machine_dma_reset(&machine->shared_dma_latch,
        &machine->shared_dma_primary, &machine->shared_dma_secondary);
    if (machine->rtc_cmos_configured) core_machine_rtc_reset(&machine->shared_rtc);
    core_machine_board_cold_reset(machine);
    core_machine_fdc_reset(&machine->fdc);
    core_machine_hdc_reset(&machine->hdc);
    core_machine_pic_reset(&machine->shared_pic_master,
        &machine->shared_pic_slave);
    core_machine_pit_reset(&machine->shared_pit);
    if (machine->auxiliary_pit_configured) {
        core_machine_pit_reset(&machine->auxiliary_pit);
    }
    core_machine_board_after_pit_reset(machine);
    machine->d4_refresh_hold_pending = LIB_FALSE;
    machine->d4_refresh_pulse_active = LIB_FALSE;
    machine->d4_refresh_address = 0u;
    core_machine_vadp_reset(&machine->shared_vadp);

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
    machine->source_repeat_active = LIB_FALSE;
    machine->source_repeat_cs = 0u;
    machine->source_repeat_eip = 0u;
    machine->source_repeat_opcode = 0u;
    machine->source_repeat_prefix = 0u;
    machine->source_repeat_operand_size = LIB_FALSE;
    machine->source_repeat_address_size = LIB_FALSE;
    core_machine_transaction_reset(&machine->transaction);
    core_machine_timeline_reset(&machine->timeline);
    core_machine_clock_domain_reset(&machine->dma_clock);
    core_machine_clock_domain_reset(&machine->pit_clock);
    core_machine_clock_domain_reset(&machine->auxiliary_pit_clock);
    core_machine_clock_domain_reset(&machine->rtc_clock);
    core_machine_clock_domain_reset(&machine->vadp_clock);
    core_machine_clock_domain_reset(&machine->kbc_clock);
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
    core_machine_cpu_state_reset(&machine->executor_cpu_execution);
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
    machine->source_repeat_active = LIB_FALSE;
}

lib_status core_machine_reconfigure_memory(core_machine *machine,
    lib_size memory_bytes)
{
    lib_uptr index;

    if (machine == LIB_NULL || !core_machine_mutable_operation_is_allowed(machine) ||
        !machine->execution_provider_frozen ||
        machine->lifecycle != CORE_MACHINE_STOPPED ||
        (machine->planar_parity_configured &&
         machine->planar_parity_config.memory_bytes != 0u) ||
        memory_bytes < CORE_MACHINE_MINIMUM_MEMORY_BYTES ||
        memory_bytes > CORE_MACHINE_MAXIMUM_MEMORY_BYTES) {
        return LIB_STATUS_INVALID_STATE;
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
            /* DeskPro D3PE consumes processor shutdown as a CPU-reset pulse.
             * It must win over the legacy stop marker carried with that CPU
             * event, or the generic stop path would incorrectly cold-reset
             * the board before D4 can consume the event. */
            if (machine->d4_platform_configured &&
                core_machine_cpu_execution_consume_shutdown_request(
                    &machine->executor_cpu_execution)) {
                machine->lifecycle = CORE_MACHINE_PAUSED;
                core_machine_processor_reset(machine);
                machine->lifecycle = CORE_MACHINE_STOPPED;
                result->reason = CORE_MACHINE_STOP_RESET_REQUESTED;
                result->linear_pc = core_machine_linear_pc(machine);
                return LIB_STATUS_OK;
            }
            if (core_machine_cpu_execution_consume_debug_pause_request(
                    &machine->executor_cpu_execution)) {
                machine->lifecycle = CORE_MACHINE_PAUSED;
                result->reason = CORE_MACHINE_STOP_PAUSED;
                result->linear_pc = core_machine_linear_pc(machine);
                return core_machine_complete_run_boundary(machine, result);
            }
            if (lib_atomic_i32_load_explicit(&machine->stop_requested, LIB_MEMORY_ORDER_ACQUIRE) ||
                core_machine_cpu_execution_consume_stop_request(
                    &machine->executor_cpu_execution)) {
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
                    &machine->executor_cpu_execution)) {
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
                    if (result->ticks == UINT64_MAX || machine->elapsed_ticks == UINT64_MAX) {
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
                    if (result->ticks == UINT64_MAX ||
                        machine->elapsed_ticks == UINT64_MAX) {
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
                if (machine->executor_cpu.data.flagHalt) {
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
                if (UINT64_MAX - result->ticks < machine->cpu_retirement_completion_ticks ||
                    UINT64_MAX - machine->elapsed_ticks <
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
                lib_u8 was_halted = machine->executor_cpu.data.flagHalt;

                machine->external_cycle_round_ticks = 0u;
                /* A completed instruction round cannot inherit an undeclared
                 * external-cycle overlap into the next CPU refresh. */
                core_machine_external_cycle_invalidate(machine);
                core_machine_cpu_execution_refresh(&machine->executor_cpu_execution);
                if (machine->lifecycle == CORE_MACHINE_FAULTED) {
                    result->reason = CORE_MACHINE_STOP_FAULT;
                    result->linear_pc = core_machine_linear_pc(machine);
                    result->detail = machine->fault_detail;
                    result->elapsed_ticks = machine->elapsed_ticks;
                    return LIB_STATUS_INTERNAL_ERROR;
                }
                if (core_machine_cpu_execution_consume_instruction_fault_delivery(
                        &machine->executor_cpu_execution)) {
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
                if (was_halted && machine->executor_cpu.data.flagHalt) {
                    machine->lifecycle = CORE_MACHINE_PAUSED;
                    result->reason = CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT;
                    result->linear_pc = core_machine_linear_pc(machine);
                    result->elapsed_ticks = machine->elapsed_ticks;
                    return core_machine_complete_run_boundary(machine, result);
                }
                if (machine->transaction_contract.cpu_prefetch_reservation_enabled) {
                    core_machine_cpu_execution_reserve_prefetch(
                        &machine->executor_cpu_execution);
                }
            }
            {
                core_machine_cpu_timing_result timing_result;
                lib_u64 instruction_ticks;

                if (!core_machine_cpu_timing_select(machine, &timing_result) ||
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
                    UINT64_MAX - result->ticks < instruction_ticks ||
                    UINT64_MAX - machine->elapsed_ticks < instruction_ticks) {
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
                    &machine->executor_cpu_execution)) {
                machine->lifecycle = CORE_MACHINE_PAUSED;
                result->reason = CORE_MACHINE_STOP_PAUSED;
                result->linear_pc = core_machine_linear_pc(machine);
                return core_machine_complete_run_boundary(machine, result);
            }
            if (machine->executor_cpu.data.flagHalt) {
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

lib_status core_machine_set_nmi_mask(core_machine *machine, lib_i32 masked)
{
    if (machine == LIB_NULL || !core_machine_mutable_operation_is_allowed(machine)) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    machine->executor_cpu.data.flagMaskNMI = masked ? LIB_TRUE : LIB_FALSE;
    if (!masked) {
        core_machine_board_refresh_nmi(machine);
    }
    return LIB_STATUS_OK;
}

lib_status core_machine_get_nmi_mask(const core_machine *machine,
    lib_i32 *out_masked)
{
    if (machine == LIB_NULL || out_masked == LIB_NULL) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    *out_masked = machine->executor_cpu.data.flagMaskNMI ? LIB_TRUE : LIB_FALSE;
    return LIB_STATUS_OK;
}

lib_status core_machine_keyboard_receive_native_byte(core_machine *machine,
    lib_u8 native_byte)
{
    if (machine == LIB_NULL || !core_machine_mutable_operation_is_allowed(machine) ||
        machine->lifecycle == CORE_MACHINE_INITIALIZED ||
        machine->lifecycle == CORE_MACHINE_FAULTED) {
        return LIB_STATUS_INVALID_STATE;
    }
    if (machine->keyboard_topology ==
            CORE_MACHINE_KEYBOARD_TOPOLOGY_XT_PPI) {
        return core_machine_xt_keyboard_receive_native_bytes(&machine->xt_keyboard,
            &native_byte, 1u);
    }
    return core_machine_kbc_submit_native_byte(&machine->shared_kbc, native_byte);
}

lib_status core_machine_keyboard_get_native_scan_set(const core_machine *machine,
    lib_u8 *out_scan_set)
{
    if (machine == LIB_NULL || out_scan_set == LIB_NULL ||
        machine->lifecycle == CORE_MACHINE_INITIALIZED) {
        return LIB_STATUS_INVALID_STATE;
    }
    *out_scan_set = machine->keyboard_topology ==
        CORE_MACHINE_KEYBOARD_TOPOLOGY_XT_PPI ? CORE_MACHINE_KEYBOARD_SCAN_SET_1 :
        machine->shared_kbc.data.scan_set;
    return LIB_STATUS_OK;
}

lib_status core_machine_keyboard_receive_native_bytes(core_machine *machine,
    const lib_u8 *native_bytes, lib_size count)
{
    if (machine == LIB_NULL || !core_machine_mutable_operation_is_allowed(machine) ||
        machine->lifecycle == CORE_MACHINE_INITIALIZED ||
        machine->lifecycle == CORE_MACHINE_FAULTED) {
        return LIB_STATUS_INVALID_STATE;
    }
    if (machine->keyboard_topology ==
            CORE_MACHINE_KEYBOARD_TOPOLOGY_XT_PPI) {
        return core_machine_xt_keyboard_receive_native_bytes(&machine->xt_keyboard,
            native_bytes, count);
    }
    return core_machine_kbc_submit_native_bytes(&machine->shared_kbc, native_bytes, count);
}

lib_status core_machine_set_xt_ppi_fault_input(core_machine *machine,
    core_machine_xt_ppi_fault_input input, lib_i32 asserted)
{
    if (machine == LIB_NULL || !core_machine_mutable_operation_is_allowed(machine) ||
        machine->keyboard_topology != CORE_MACHINE_KEYBOARD_TOPOLOGY_XT_PPI) {
        return LIB_STATUS_INVALID_STATE;
    }
    return core_machine_xt_ppi_keyboard_set_fault_input(&machine->xt_ppi_keyboard,
        input, asserted);
}

lib_status core_machine_mouse_receive_relative(core_machine *machine,
    lib_i16 delta_x, lib_i16 delta_y, lib_u8 buttons)
{
    if (machine == LIB_NULL || !core_machine_mutable_operation_is_allowed(machine) ||
        (machine->lifecycle != CORE_MACHINE_RUNNING &&
        machine->lifecycle != CORE_MACHINE_PAUSED &&
        machine->lifecycle != CORE_MACHINE_STOPPED)) {
        return LIB_STATUS_INVALID_STATE;
    }
    if (machine->keyboard_topology ==
            CORE_MACHINE_KEYBOARD_TOPOLOGY_XT_PPI) return LIB_STATUS_UNSUPPORTED;
    return core_machine_kbc_submit_aux_report(&machine->shared_kbc, delta_x, delta_y, buttons);
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
        core_machine_hdc_finalize(&machine->hdc);
        core_machine_fdc_finalize(&machine->fdc);
        core_machine_dma_finalize(&machine->shared_dma_latch,
            &machine->shared_dma_primary, &machine->shared_dma_secondary);
        core_machine_rtc_finalize(&machine->shared_rtc);
        if (machine->keyboard_topology ==
                CORE_MACHINE_KEYBOARD_TOPOLOGY_XT_PPI) {
            core_machine_xt_keyboard_finalize(&machine->xt_keyboard);
            core_machine_xt_ppi_keyboard_finalize(&machine->xt_ppi_keyboard);
        } else core_machine_kbc_finalize(&machine->shared_kbc);
        core_machine_pic_finalize(&machine->shared_pic_master,
            &machine->shared_pic_slave);
        core_machine_pit_finalize(&machine->shared_pit);
        if (machine->auxiliary_pit_configured) {
            core_machine_pit_finalize(&machine->auxiliary_pit);
        }
        core_machine_vadp_finalize(&machine->shared_vadp);
        core_machine_cpu_execution_finalize(&machine->executor_cpu_execution);
        core_machine_port_finalize(&machine->executor_port);
        core_machine_memory_finalize(&machine->executor_memory);
        for (lib_size index = 0u; index < machine->immutable_rom_mapping_count;
                ++index) {
            if (machine->immutable_rom_mappings[index].owns_image) {
                lib_release(machine->immutable_rom_mappings[index].image);
            }
        }
    }
    core_machine_trace_finalize(machine);
    core_machine_bus_finalize(machine);
    lib_release(machine);
}
