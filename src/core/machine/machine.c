#include "type.h"

#include "core/machine/machine.h"

static C_VOID core_machine_cpu_diagnostic_copy_point(
    core_machine_cpu_execution_point *point, const t_cpu *cpu,
    const t_cpuins *instructions)
{
    if (point == STD_NULL || cpu == STD_NULL || instructions == STD_NULL) return;
    point->cs = cpu->data.cs.selector;
    point->cs_base = cpu->data.cs.base;
    point->eip = cpu->data.eip;
    point->linear_pc = instructions->data.linear;
    point->byte_count = (uint8_t)instructions->data.oplen;
    STD_MEMCPY(point->bytes, instructions->data.opcodes, sizeof(point->bytes));
}

static C_VOID core_machine_cpu_diagnostic_record_instruction(C_VOID *opaque,
    const C_VOID *opaque_cpu, const t_cpuins *instructions)
{
    core_machine *machine = (core_machine *)opaque;
    const t_cpu *cpu = (const t_cpu *)opaque_cpu;
    core_machine_cpu_diagnostic_state *state;

    if (machine == STD_NULL) return;
    state = &machine->cpu_diagnostic;
    core_machine_cpu_diagnostic_copy_point(
        &state->snapshot.recent[state->next_index], cpu, instructions);
    state->next_index = (state->next_index + 1u) % CORE_MACHINE_CPU_DIAGNOSTIC_WINDOW_CAPACITY;
    if (state->snapshot.recent_count < CORE_MACHINE_CPU_DIAGNOSTIC_WINDOW_CAPACITY) {
        ++state->snapshot.recent_count;
    }
}

static C_VOID core_machine_cpu_diagnostic_record_fault(C_VOID *opaque,
    const C_VOID *opaque_cpu, const t_cpuins *instructions)
{
    core_machine *machine = (core_machine *)opaque;
    const t_cpu *cpu = (const t_cpu *)opaque_cpu;
    core_machine_cpu_fault_snapshot *fault;

    if (machine == STD_NULL || cpu == STD_NULL || instructions == STD_NULL) return;
    fault = &machine->cpu_diagnostic.snapshot.first_fault;
    if (fault->valid) return;
    STD_MEMSET(fault, 0, sizeof(*fault));
    fault->valid = 1;
    fault->exception_mask = instructions->data.except;
    fault->exception_code = instructions->data.excode;
    core_machine_cpu_diagnostic_copy_point(&fault->point, cpu, instructions);
    fault->eax = cpu->data.eax;
    fault->ebx = cpu->data.ebx;
    fault->ecx = cpu->data.ecx;
    fault->edx = cpu->data.edx;
    fault->cr2 = cpu->data.cr2;
    fault->esp = cpu->data.esp;
    fault->ebp = cpu->data.ebp;
    fault->esi = cpu->data.esi;
    fault->edi = cpu->data.edi;
    fault->eflags = cpu->data.eflags;
    (C_VOID)core_machine_report_fault(machine, fault->exception_mask);
}

static C_VOID core_machine_cpu_diagnostic_record_delivered_exception(
    C_VOID *opaque, const C_VOID *opaque_cpu, const t_cpuins *instructions)
{
    core_machine *machine = (core_machine *)opaque;
    const t_cpu *cpu = (const t_cpu *)opaque_cpu;
    core_machine_cpu_fault_snapshot *exception;

    if (machine == STD_NULL || cpu == STD_NULL || instructions == STD_NULL) return;
    exception = &machine->cpu_diagnostic.snapshot.last_delivered_exception;
    STD_MEMSET(exception, 0, sizeof(*exception));
    exception->valid = 1;
    exception->exception_mask = instructions->data.except;
    exception->exception_code = instructions->data.excode;
    core_machine_cpu_diagnostic_copy_point(&exception->point, cpu, instructions);
    exception->eax = cpu->data.eax;
    exception->ebx = cpu->data.ebx;
    exception->ecx = cpu->data.ecx;
    exception->edx = cpu->data.edx;
    exception->cr2 = cpu->data.cr2;
    exception->esp = cpu->data.esp;
    exception->ebp = cpu->data.ebp;
    exception->esi = cpu->data.esi;
    exception->edi = cpu->data.edi;
    exception->eflags = cpu->data.eflags;
    machine->cpu_diagnostic.snapshot.delivered_exception_count++;
}

static const core_machine_cpu_execution_diagnostic_provider
    core_machine_cpu_diagnostic_provider = {
        core_machine_cpu_diagnostic_record_instruction,
        core_machine_cpu_diagnostic_record_delivered_exception,
        core_machine_cpu_diagnostic_record_fault
    };

static C_VOID core_machine_undefined_instruction_dispatch(C_VOID *opaque,
    const core_machine_undefined_instruction_input *input,
    core_machine_undefined_instruction_response *out_response)
{
    core_machine *machine = (core_machine *)opaque;
    STD_SIZE_T index;

    if (machine == STD_NULL || input == STD_NULL || out_response == STD_NULL) return;
    out_response->outcome = CORE_MACHINE_UNDEFINED_INSTRUCTION_UNHANDLED;
    for (index = 0u; index < machine->undefined_instruction_registry.count; ++index) {
        const core_machine_undefined_instruction_transition *entry =
            &machine->undefined_instruction_registry.entries[index];
        if (input->byte_count < entry->length ||
            STD_MEMCMP(input->bytes, entry->pattern, entry->length) != 0) {
            continue;
        }
        entry->consumer(entry->owner, input, out_response);
        out_response->consumed_bytes = entry->length;
        if (out_response->outcome == CORE_MACHINE_UNDEFINED_INSTRUCTION_FAULT &&
            out_response->fault_detail != 0u) {
            (C_VOID)core_machine_report_fault(machine, out_response->fault_detail);
        } else if (out_response->outcome > CORE_MACHINE_UNDEFINED_INSTRUCTION_FAULT ||
                   (out_response->outcome == CORE_MACHINE_UNDEFINED_INSTRUCTION_FAULT &&
                    out_response->fault_detail == 0u)) {
            out_response->outcome = CORE_MACHINE_UNDEFINED_INSTRUCTION_UNHANDLED;
        }
        return;
    }
}

type_status core_machine_register_undefined_instruction_transition(
    core_machine *machine, const uint8_t *pattern, uint8_t length,
    core_machine_undefined_instruction_consumer consumer, C_VOID *owner)
{
    core_machine_undefined_instruction_registry *registry;
    STD_SIZE_T index;

    if (machine == STD_NULL || pattern == STD_NULL || length == 0u ||
        length > CORE_MACHINE_UNDEFINED_INSTRUCTION_MAX_BYTES ||
        consumer == STD_NULL || owner == STD_NULL) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    if (!core_machine_configuration_is_open(machine)) return TYPE_STATUS_INVALID_STATE;
    registry = &machine->undefined_instruction_registry;
    if (registry->frozen) return TYPE_STATUS_INVALID_STATE;
    if (registry->count >= CORE_MACHINE_UNDEFINED_INSTRUCTION_TRANSITION_CAPACITY) {
        return TYPE_STATUS_NO_MEMORY;
    }
    for (index = 0u; index < registry->count; ++index) {
        const core_machine_undefined_instruction_transition *existing =
            &registry->entries[index];
        uint8_t common = length < existing->length ? length : existing->length;

        if (STD_MEMCMP(pattern, existing->pattern, common) == 0) {
            return TYPE_STATUS_INVALID_ARGUMENT;
        }
    }
    STD_MEMCPY(registry->entries[registry->count].pattern, pattern, length);
    registry->entries[registry->count].length = length;
    registry->entries[registry->count].consumer = consumer;
    registry->entries[registry->count].owner = owner;
    ++registry->count;
    return TYPE_STATUS_OK;
}

static C_VOID core_machine_cpu_diagnostic_ordered_copy(
    const core_machine_cpu_diagnostic_state *state,
    core_machine_cpu_diagnostic *out_diagnostic)
{
    STD_SIZE_T index;
    STD_SIZE_T first;

    *out_diagnostic = state->snapshot;
    if (state->snapshot.recent_count < CORE_MACHINE_CPU_DIAGNOSTIC_WINDOW_CAPACITY ||
        state->next_index == 0u) return;
    first = state->next_index;
    for (index = 0u; index < CORE_MACHINE_CPU_DIAGNOSTIC_WINDOW_CAPACITY; ++index) {
        out_diagnostic->recent[index] = state->snapshot.recent[
            (first + index) % CORE_MACHINE_CPU_DIAGNOSTIC_WINDOW_CAPACITY];
    }
}

C_VOID core_machine_cpu_diagnostic_initialize(core_machine *machine)
{
    if (machine != STD_NULL) {
        STD_MEMSET(&machine->cpu_diagnostic, 0, sizeof(machine->cpu_diagnostic));
    }
}

C_VOID core_machine_cpu_diagnostic_reset(core_machine *machine)
{
    core_machine_cpu_diagnostic_initialize(machine);
}



static uint32_t core_machine_linear_pc(const core_machine *machine)
{
    return machine->executor_cpu.data.cs.base + machine->executor_cpu.data.eip;
}

static core_machine_cpu_profile core_machine_resolve_cpu_profile(
    core_machine_cpu_profile profile)
{
    return profile == CORE_MACHINE_CPU_PROFILE_DEFAULT ?
        CORE_MACHINE_CPU_PROFILE_80386 : profile;
}

static uint32_t core_machine_resolve_ticks_per_instruction(uint32_t ticks)
{
    return ticks == 0u ? 1u : ticks;
}

static C_INT core_machine_clock_plan_is_valid(
    const core_machine_clock_plan *plan)
{
    return plan != STD_NULL &&
        core_machine_clock_ratio_is_valid(&plan->dma) &&
        core_machine_clock_ratio_is_valid(&plan->pit) &&
        core_machine_clock_ratio_is_valid(&plan->vadp) &&
        core_machine_clock_ratio_is_valid(&plan->kbc) &&
        core_machine_clock_ratio_is_valid(&plan->provider);
}

static C_VOID core_machine_advance_scheduler(core_machine *machine,
    uint64_t elapsed_ticks)
{
    uint64_t dma_ticks;
    uint64_t pit_ticks;
    uint64_t vadp_ticks;
    uint64_t kbc_ticks;
    uint64_t provider_ticks;

    dma_ticks = core_machine_clock_domain_advance(&machine->dma_clock,
        elapsed_ticks);
    pit_ticks = core_machine_clock_domain_advance(&machine->pit_clock,
        elapsed_ticks);
    vadp_ticks = core_machine_clock_domain_advance(&machine->vadp_clock,
        elapsed_ticks);
    kbc_ticks = core_machine_clock_domain_advance(&machine->kbc_clock,
        elapsed_ticks);
    provider_ticks = core_machine_clock_domain_advance(&machine->provider_clock,
        elapsed_ticks);
    core_machine_dma_advance(&machine->shared_dma_latch,
        &machine->shared_dma_primary, &machine->shared_dma_secondary,
        &machine->executor_memory, dma_ticks);
    core_machine_pit_advance(&machine->shared_pit, pit_ticks);
    core_machine_vadp_advance(&machine->shared_vadp, &machine->executor_memory,
        vadp_ticks);
    core_machine_kbc_advance(&machine->shared_kbc, kbc_ticks);
    if (machine->execution_provider != STD_NULL &&
        machine->execution_provider->advance_time != STD_NULL) {
        machine->execution_provider->advance_time(
            machine->execution_provider_context, provider_ticks);
    }
    core_machine_pic_refresh(&machine->shared_pic_master,
        &machine->shared_pic_slave);
}

static C_INT core_machine_valid_cpu_profile(core_machine_cpu_profile profile)
{
    return profile >= CORE_MACHINE_CPU_PROFILE_8086 &&
        profile <= CORE_MACHINE_CPU_PROFILE_80386;
}

static C_INT core_machine_valid_fpu_profile(core_machine_fpu_profile profile)
{
    return profile >= CORE_MACHINE_FPU_PROFILE_NONE &&
        profile <= CORE_MACHINE_FPU_PROFILE_80387;
}

C_INT core_machine_configuration_is_open(const core_machine *machine)
{
    return machine != STD_NULL &&
        machine->lifecycle == CORE_MACHINE_INITIALIZED &&
        !machine->execution_provider_frozen;
}

t_cpu *core_machine_configuration_cpu_borrow(core_machine *machine)
{ return core_machine_configuration_is_open(machine) ? &machine->executor_cpu : STD_NULL; }

t_cpuins *core_machine_configuration_cpu_instructions_borrow(core_machine *machine)
{ return core_machine_configuration_is_open(machine) ? &machine->executor_cpu_instructions : STD_NULL; }

core_machine_cpu_execution_context *core_machine_configuration_cpu_execution_borrow(
    core_machine *machine)
{ return core_machine_configuration_is_open(machine) ? &machine->executor_cpu_execution : STD_NULL; }

t_ram *core_machine_configuration_memory_borrow(core_machine *machine)
{ return core_machine_configuration_is_open(machine) ? &machine->executor_memory : STD_NULL; }

type_status core_machine_profile_binding_initialize(
    core_machine *machine, core_machine_profile_binding *binding)
{
    if (!core_machine_configuration_is_open(machine) || binding == STD_NULL) {
        return TYPE_STATUS_INVALID_STATE;
    }
    binding->machine = machine;
    return TYPE_STATUS_OK;
}

t_ram *core_machine_profile_binding_memory(
    const core_machine_profile_binding *binding)
{
    return binding == STD_NULL || binding->machine == STD_NULL ? STD_NULL :
        &binding->machine->executor_memory;
}

core_machine_cpu_execution_context *core_machine_profile_binding_execution(
    const core_machine_profile_binding *binding)
{
    return binding == STD_NULL || binding->machine == STD_NULL ? STD_NULL :
        &binding->machine->executor_cpu_execution;
}

type_status core_machine_profile_binding_configure_text_video(
    const core_machine_profile_binding *binding, uint8_t mode, uint16_t columns,
    uint16_t rows, C_INT color_enabled)
{
    return binding == STD_NULL || binding->machine == STD_NULL ?
        TYPE_STATUS_INVALID_ARGUMENT : core_machine_vadp_configure_text(
            &binding->machine->shared_vadp, mode, columns, rows, color_enabled);
}

type_status core_machine_profile_binding_configure_text_raster(
    const core_machine_profile_binding *binding,
    const core_machine_vadp_text_timing *timing)
{
    return binding == STD_NULL || binding->machine == STD_NULL ?
        TYPE_STATUS_INVALID_ARGUMENT : core_machine_vadp_configure_text_timing(
            &binding->machine->shared_vadp, timing);
}

type_status core_machine_profile_binding_configure_ega_sequencer(
    const core_machine_profile_binding *binding,
    const core_machine_vadp_ega_sequencer_config *config)
{
    return binding == STD_NULL || binding->machine == STD_NULL ?
        TYPE_STATUS_INVALID_ARGUMENT : core_machine_vadp_configure_ega_sequencer(
            &binding->machine->shared_vadp, &binding->machine->executor_memory,
            config);
}

type_status core_machine_profile_binding_configure_ega_controllers(
    const core_machine_profile_binding *binding,
    const core_machine_vadp_ega_controller_config *config)
{
    return binding == STD_NULL || binding->machine == STD_NULL ?
        TYPE_STATUS_INVALID_ARGUMENT : core_machine_vadp_configure_ega_controllers(
            &binding->machine->shared_vadp, config);
}

C_VOID core_machine_profile_binding_set_video_cursor_shape(
    const core_machine_profile_binding *binding, uint8_t top, uint8_t bottom)
{
    if (binding != STD_NULL && binding->machine != STD_NULL) {
        core_machine_vadp_set_cursor_shape(&binding->machine->shared_vadp, top, bottom);
    }
}

C_VOID core_machine_profile_binding_set_video_cursor_address(
    const core_machine_profile_binding *binding, uint16_t address)
{
    if (binding != STD_NULL && binding->machine != STD_NULL) {
        core_machine_vadp_set_cursor_address(&binding->machine->shared_vadp, address);
    }
}

C_VOID core_machine_profile_binding_set_video_display_start(
    const core_machine_profile_binding *binding, uint16_t address)
{
    if (binding != STD_NULL && binding->machine != STD_NULL) {
        core_machine_vadp_set_display_start(&binding->machine->shared_vadp, address);
    }
}

type_status core_machine_capture_display_snapshot(const core_machine *machine,
    core_machine_display_snapshot *out_snapshot)
{
    core_machine *mutable_machine = (core_machine *)machine;

    if (machine == STD_NULL || out_snapshot == STD_NULL) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    if (machine->lifecycle != CORE_MACHINE_STOPPED &&
        machine->lifecycle != CORE_MACHINE_PAUSED) {
        return TYPE_STATUS_INVALID_STATE;
    }
    return core_machine_vadp_capture_snapshot(&mutable_machine->shared_vadp,
        &mutable_machine->executor_memory, out_snapshot) ? TYPE_STATUS_OK :
        TYPE_STATUS_UNSUPPORTED;
}

t_port *core_machine_configuration_port_borrow(core_machine *machine)
{ return core_machine_configuration_is_open(machine) ? &machine->executor_port : STD_NULL; }

t_pic *core_machine_configuration_shared_pic_master_borrow(core_machine *machine)
{ return core_machine_configuration_is_open(machine) ? &machine->shared_pic_master : STD_NULL; }
t_pic *core_machine_configuration_shared_pic_slave_borrow(core_machine *machine)
{ return core_machine_configuration_is_open(machine) ? &machine->shared_pic_slave : STD_NULL; }
t_pit *core_machine_configuration_shared_pit_borrow(core_machine *machine)
{ return core_machine_configuration_is_open(machine) ? &machine->shared_pit : STD_NULL; }
t_latch *core_machine_configuration_shared_dma_latch_borrow(core_machine *machine)
{ return core_machine_configuration_is_open(machine) ? &machine->shared_dma_latch : STD_NULL; }
t_dma *core_machine_configuration_shared_dma_primary_borrow(core_machine *machine)
{ return core_machine_configuration_is_open(machine) ? &machine->shared_dma_primary : STD_NULL; }
t_dma *core_machine_configuration_shared_dma_secondary_borrow(core_machine *machine)
{ return core_machine_configuration_is_open(machine) ? &machine->shared_dma_secondary : STD_NULL; }
t_kbc *core_machine_configuration_shared_kbc_borrow(core_machine *machine)
{ return core_machine_configuration_is_open(machine) ? &machine->shared_kbc : STD_NULL; }
type_status core_machine_bind_execution_provider(core_machine *machine,
    const core_machine_execution_provider *provider, C_VOID *context)
{
    if (!core_machine_configuration_is_open(machine)) {
        return TYPE_STATUS_INVALID_STATE;
    }
    if (provider != STD_NULL && provider->reset == STD_NULL &&
        provider->refresh == STD_NULL && provider->advance_time == STD_NULL) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    machine->execution_provider = provider;
    machine->execution_provider_context = context;
    return TYPE_STATUS_OK;
}

type_status core_machine_freeze_execution_providers(core_machine *machine)
{
    if (!core_machine_configuration_is_open(machine)) {
        return TYPE_STATUS_INVALID_STATE;
    }
    machine->execution_provider_frozen = 1;
    machine->undefined_instruction_registry.frozen = TYPE_TRUE;
    core_machine_memory_freeze_mappings(&machine->executor_memory);
    return TYPE_STATUS_OK;
}

type_status core_machine_get_cpu_state(
    const core_machine *machine,
    core_machine_cpu_state *out_state)
{
    if (machine == STD_NULL || out_state == STD_NULL) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }

    if (machine->lifecycle == CORE_MACHINE_INITIALIZED ||
        machine->lifecycle == CORE_MACHINE_RUNNING) {
        return TYPE_STATUS_INVALID_STATE;
    }
    out_state->cs = machine->executor_cpu.data.cs.selector;
    out_state->cs_base = machine->executor_cpu.data.cs.base;
    out_state->eip = machine->executor_cpu.data.eip;
    out_state->eflags = machine->executor_cpu.data.eflags;
    out_state->halted = machine->executor_cpu.data.flagHalt;
    return TYPE_STATUS_OK;
}

type_status core_machine_get_cpu_profile(
    const core_machine *machine, core_machine_cpu_profile *out_profile)
{
    if (machine == STD_NULL || out_profile == STD_NULL) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    *out_profile = machine->cpu_profile;
    return TYPE_STATUS_OK;
}

type_status core_machine_get_fpu_profile(
    const core_machine *machine, core_machine_fpu_profile *out_profile)
{
    if (machine == STD_NULL || out_profile == STD_NULL) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    *out_profile = machine->fpu.profile;
    return TYPE_STATUS_OK;
}

type_status core_machine_get_fpu_state(
    const core_machine *machine, core_machine_fpu_state *out_state)
{
    if (machine == STD_NULL || out_state == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    core_machine_fpu_get_state(&machine->fpu, out_state);
    return TYPE_STATUS_OK;
}

type_status core_machine_get_memory_bytes(
    const core_machine *machine, STD_SIZE_T *out_memory_bytes)
{
    if (machine == STD_NULL || out_memory_bytes == STD_NULL) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    *out_memory_bytes = machine->executor_memory.connect.installed_bytes;
    return TYPE_STATUS_OK;
}

type_status core_machine_get_elapsed_ticks(
    const core_machine *machine, uint64_t *out_elapsed_ticks)
{
    if (machine == STD_NULL || out_elapsed_ticks == STD_NULL ||
        machine->lifecycle == CORE_MACHINE_INITIALIZED ||
        machine->lifecycle == CORE_MACHINE_RUNNING) {
        return TYPE_STATUS_INVALID_STATE;
    }
    *out_elapsed_ticks = machine->elapsed_ticks;
    return TYPE_STATUS_OK;
}

type_status core_machine_get_cpu_diagnostic(
    const core_machine *machine, core_machine_cpu_diagnostic *out_diagnostic)
{
    if (machine == STD_NULL || out_diagnostic == STD_NULL ||
        machine->lifecycle == CORE_MACHINE_INITIALIZED ||
        machine->lifecycle == CORE_MACHINE_RUNNING) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    core_machine_cpu_diagnostic_ordered_copy(&machine->cpu_diagnostic,
        out_diagnostic);
    return TYPE_STATUS_OK;
}

type_status core_machine_capture_observation(
    const core_machine *machine, core_machine_observation *out_observation)
{
    if (machine == STD_NULL || out_observation == STD_NULL ||
        machine->lifecycle == CORE_MACHINE_INITIALIZED ||
        machine->lifecycle == CORE_MACHINE_RUNNING) {
        return TYPE_STATUS_INVALID_STATE;
    }
    out_observation->lifecycle = machine->lifecycle;
    out_observation->elapsed_ticks = machine->elapsed_ticks;
    if (core_machine_get_cpu_state(machine, &out_observation->cpu) !=
            TYPE_STATUS_OK) {
        return TYPE_STATUS_INVALID_STATE;
    }
    core_machine_cpu_diagnostic_ordered_copy(&machine->cpu_diagnostic,
        &out_observation->diagnostic);
    return TYPE_STATUS_OK;
}

type_status core_machine_create(
    const core_machine_config *config,
    core_machine **out_machine)
{
    core_machine *machine;
    if (config == STD_NULL || out_machine == STD_NULL ||
        !core_machine_valid_cpu_profile(
            core_machine_resolve_cpu_profile(config->cpu_profile)) ||
        !core_machine_valid_fpu_profile(config->fpu_profile) ||
        !core_machine_clock_plan_is_valid(&config->clock_plan)) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }

    *out_machine = STD_NULL;

    machine = (core_machine *)STD_CALLOC(1u, sizeof(*machine));
    if (machine == STD_NULL) {
        return TYPE_STATUS_NO_MEMORY;
    }

    machine->lifecycle = CORE_MACHINE_INITIALIZED;
    machine->cpu_profile = core_machine_resolve_cpu_profile(config->cpu_profile);
    machine->ticks_per_instruction =
        core_machine_resolve_ticks_per_instruction(config->ticks_per_instruction);
    if (core_machine_clock_domain_initialize(&machine->dma_clock,
            &config->clock_plan.dma) != TYPE_STATUS_OK ||
        core_machine_clock_domain_initialize(&machine->pit_clock,
            &config->clock_plan.pit) != TYPE_STATUS_OK ||
        core_machine_clock_domain_initialize(&machine->vadp_clock,
            &config->clock_plan.vadp) != TYPE_STATUS_OK ||
        core_machine_clock_domain_initialize(&machine->kbc_clock,
            &config->clock_plan.kbc) != TYPE_STATUS_OK ||
        core_machine_clock_domain_initialize(&machine->provider_clock,
            &config->clock_plan.provider) != TYPE_STATUS_OK) {
        STD_FREE(machine);
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    /* Zero is an explicit profile choice: without a calibrated guest-time
     * mapping, core-generated keyboard repeat must remain disabled. */
    machine->kbc_typematic_initial_ticks = config->kbc_typematic_initial_ticks;
    machine->kbc_typematic_repeat_ticks = config->kbc_typematic_repeat_ticks;
    machine->kbc_command_response_ticks = config->kbc_command_response_ticks;
    core_machine_fpu_initialize(&machine->fpu, config->fpu_profile);
    STD_ATOMIC_INIT(&machine->stop_requested, 0);
    core_machine_trace_initialize(machine);
    core_machine_cpu_diagnostic_initialize(machine);

    core_machine_cpu_execution_context_initialize(&machine->executor_cpu_execution,
        &machine->executor_cpu, &machine->executor_cpu_instructions,
        &machine->executor_memory, &machine->executor_port);
    core_machine_cpu_execution_context_bind_profiles(
        &machine->executor_cpu_execution, machine->cpu_profile,
        machine->fpu.profile);
    core_machine_cpu_execution_context_bind_fpu(
        &machine->executor_cpu_execution, &machine->fpu);
    core_machine_cpu_execution_context_bind_diagnostic_provider(
        &machine->executor_cpu_execution, &core_machine_cpu_diagnostic_provider,
        machine);
    core_machine_cpu_execution_context_bind_undefined_instruction_dispatch(
        &machine->executor_cpu_execution, core_machine_undefined_instruction_dispatch,
        machine);
    core_machine_cpu_state_initialize(&machine->executor_cpu_execution);
    core_machine_port_initialize(&machine->executor_port);
    if (core_machine_bus_initialize(machine) != TYPE_STATUS_OK) {
        core_machine_destroy(machine);
        return TYPE_STATUS_NO_MEMORY;
    }
    core_machine_memory_initialize(&machine->executor_memory);
    if (config->memory_bytes != 0u) {
        if (core_machine_memory_allocate_for(&machine->executor_memory,
                config->memory_bytes) != TYPE_STATUS_OK) {
            core_machine_destroy(machine);
            return TYPE_STATUS_NO_MEMORY;
        }
    }
    core_machine_memory_register_ports(&machine->executor_memory,
        &machine->executor_port);
    core_machine_vadp_initialize(&machine->shared_vadp, &machine->executor_port);
    core_machine_kbc_initialize(&machine->shared_kbc, &machine->executor_port);
    core_machine_dma_initialize(&machine->shared_dma_latch,
        &machine->shared_dma_primary, &machine->shared_dma_secondary,
        &machine->executor_port);
    core_machine_pic_initialize(&machine->shared_pic_master,
        &machine->shared_pic_slave, &machine->executor_port);
    core_machine_pic_irq_source_bind(&machine->shared_pit_irq0_source,
        &machine->shared_pic_master, &machine->shared_pic_slave, 0u);
    core_machine_pit_initialize(&machine->shared_pit, &machine->executor_port);
    core_machine_pit_set_output(&machine->shared_pit, 0,
        core_machine_pic_timer_output, &machine->shared_pit_irq0_source);
    core_machine_kbc_bind_core_services(&machine->shared_kbc,
        &machine->shared_pic_master, &machine->shared_pic_slave,
        &machine->executor_memory, &machine->executor_cpu_execution);
    core_machine_kbc_set_typematic_timing(&machine->shared_kbc,
        machine->kbc_typematic_initial_ticks,
        machine->kbc_typematic_repeat_ticks);
    core_machine_kbc_set_command_response_timing(&machine->shared_kbc,
        machine->kbc_command_response_ticks);
    core_machine_pit_set_output(&machine->shared_pit, 1, STD_NULL, STD_NULL);

    *out_machine = machine;

    return TYPE_STATUS_OK;
}

static type_status core_machine_cold_reset(core_machine *machine)
{
    core_machine_cpu_state_reset(&machine->executor_cpu_execution);
    core_machine_fpu_reset(&machine->fpu);
    core_machine_port_reset(&machine->executor_port);
    core_machine_memory_reset(&machine->executor_memory);
    core_machine_kbc_reset(&machine->shared_kbc);
    core_machine_dma_reset(&machine->shared_dma_latch,
        &machine->shared_dma_primary, &machine->shared_dma_secondary);
    core_machine_pic_reset(&machine->shared_pic_master,
        &machine->shared_pic_slave);
    core_machine_pit_reset(&machine->shared_pit);
    core_machine_vadp_reset(&machine->shared_vadp);

    STD_ATOMIC_STORE(&machine->stop_requested, 0);
    machine->fault_detail = 0u;
    machine->elapsed_ticks = 0u;
    core_machine_clock_domain_reset(&machine->dma_clock);
    core_machine_clock_domain_reset(&machine->pit_clock);
    core_machine_clock_domain_reset(&machine->vadp_clock);
    core_machine_clock_domain_reset(&machine->kbc_clock);
    core_machine_clock_domain_reset(&machine->provider_clock);
    machine->entry_plan_applied = TYPE_FALSE;
    core_machine_cpu_diagnostic_reset(machine);
    if (machine->execution_provider != STD_NULL &&
        machine->execution_provider->reset != STD_NULL) {
        machine->execution_provider->reset(machine->execution_provider_context);
    }
    machine->lifecycle = CORE_MACHINE_STOPPED;
    core_machine_trace_record(machine, CORE_MACHINE_TRACE_RESET, 0u, 0u, 0u);
    return TYPE_STATUS_OK;
}

type_status core_machine_reconfigure_memory(core_machine *machine,
    STD_SIZE_T memory_bytes)
{
    type_native_unsigned index;

    if (machine == STD_NULL || !machine->execution_provider_frozen ||
        machine->lifecycle != CORE_MACHINE_STOPPED ||
        memory_bytes < CORE_MACHINE_MINIMUM_MEMORY_BYTES ||
        memory_bytes > CORE_MACHINE_MAXIMUM_MEMORY_BYTES) {
        return TYPE_STATUS_INVALID_STATE;
    }
    for (index = 0u; index < machine->executor_memory.connect.mapping_count;
            ++index) {
        const core_machine_memory_mapping *mapping =
            &machine->executor_memory.connect.mappings[index];
        if (mapping->backing_start > memory_bytes ||
            mapping->bytes > memory_bytes - mapping->backing_start) {
            return TYPE_STATUS_INVALID_ARGUMENT;
        }
    }
    if (core_machine_memory_allocate_for(&machine->executor_memory,
            memory_bytes) != TYPE_STATUS_OK) {
        return TYPE_STATUS_NO_MEMORY;
    }
    return core_machine_cold_reset(machine);
}

type_status core_machine_reset(core_machine *machine)
{
    if (machine == STD_NULL) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }

    if (!machine->execution_provider_frozen ||
        machine->lifecycle == CORE_MACHINE_RUNNING) {
        return TYPE_STATUS_INVALID_STATE;
    }

    return core_machine_cold_reset(machine);
}

type_status core_machine_get_lifecycle(
    const core_machine *machine,
    core_machine_lifecycle *out_lifecycle)
{
    if (machine == STD_NULL || out_lifecycle == STD_NULL) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }

    *out_lifecycle = machine->lifecycle;
    return TYPE_STATUS_OK;
}

type_status core_machine_run(
    core_machine *machine,
    core_machine_run_budget budget,
    core_machine_run_result *result)
{
    if (machine == STD_NULL || result == STD_NULL) {
        return TYPE_STATUS_INVALID_ARGUMENT;
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
        return TYPE_STATUS_FAULT;
    }

    if (machine->lifecycle != CORE_MACHINE_STOPPED &&
        machine->lifecycle != CORE_MACHINE_PAUSED) {
        return TYPE_STATUS_INVALID_STATE;
    }

    if (budget.instructions == 0u && budget.ticks == 0u) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }

    if (STD_ATOMIC_LOAD(&machine->stop_requested)) {
        if (core_machine_cold_reset(machine) != TYPE_STATUS_OK) {
            return TYPE_STATUS_FAULT;
        }
        result->reason = CORE_MACHINE_STOP_REQUESTED;
        result->linear_pc = core_machine_linear_pc(machine);
        core_machine_trace_record(machine, CORE_MACHINE_TRACE_STOP, 0u, 0u,
                               (uint32_t)result->reason);
        return TYPE_STATUS_OK;
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
                return TYPE_STATUS_FAULT;
            }
            if (STD_ATOMIC_LOAD(&machine->stop_requested) ||
                core_machine_cpu_execution_consume_stop_request(
                    &machine->executor_cpu_execution)) {
                machine->lifecycle = CORE_MACHINE_PAUSED;
                if (core_machine_cold_reset(machine) != TYPE_STATUS_OK) {
                    return TYPE_STATUS_FAULT;
                }
                result->reason = CORE_MACHINE_STOP_REQUESTED;
                result->linear_pc = core_machine_linear_pc(machine);
                core_machine_trace_record(machine, CORE_MACHINE_TRACE_STOP, 0u,
                    0u, (uint32_t)result->reason);
                return TYPE_STATUS_OK;
            }
            if (core_machine_cpu_execution_consume_reset_request(
                    &machine->executor_cpu_execution)) {
                machine->lifecycle = CORE_MACHINE_PAUSED;
                if (core_machine_cold_reset(machine) != TYPE_STATUS_OK) {
                    return TYPE_STATUS_FAULT;
                }
                result->reason = CORE_MACHINE_STOP_RESET_REQUESTED;
                result->linear_pc = core_machine_linear_pc(machine);
                return TYPE_STATUS_OK;
            }
            if (budget.ticks != 0u &&
                machine->ticks_per_instruction > budget.ticks - result->ticks) {
                machine->lifecycle = CORE_MACHINE_PAUSED;
                result->reason = CORE_MACHINE_STOP_BUDGET;
                result->linear_pc = core_machine_linear_pc(machine);
                result->elapsed_ticks = machine->elapsed_ticks;
                return TYPE_STATUS_OK;
            }
            if (machine->execution_provider != STD_NULL &&
                machine->execution_provider->refresh != STD_NULL) {
                machine->execution_provider->refresh(
                    machine->execution_provider_context);
            }
            core_machine_kbc_refresh(&machine->shared_kbc);
            core_machine_cpu_execution_refresh(&machine->executor_cpu_execution);
            if (machine->lifecycle == CORE_MACHINE_FAULTED) {
                result->reason = CORE_MACHINE_STOP_FAULT;
                result->linear_pc = core_machine_linear_pc(machine);
                result->detail = machine->fault_detail;
                result->elapsed_ticks = machine->elapsed_ticks;
                return TYPE_STATUS_FAULT;
            }
            ++result->executed;
            result->ticks += machine->ticks_per_instruction;
            machine->elapsed_ticks += machine->ticks_per_instruction;
            result->elapsed_ticks = machine->elapsed_ticks;
            core_machine_advance_scheduler(machine,
                machine->ticks_per_instruction);
            if (machine->executor_cpu.data.flagHalt) {
                machine->lifecycle = CORE_MACHINE_PAUSED;
                result->reason = CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT;
                result->linear_pc = core_machine_linear_pc(machine);
                return TYPE_STATUS_OK;
            }
        }
        machine->lifecycle = CORE_MACHINE_PAUSED;
        result->reason = CORE_MACHINE_STOP_BUDGET;
        result->linear_pc = core_machine_linear_pc(machine);
        core_machine_trace_record(machine, CORE_MACHINE_TRACE_RUN_BOUNDARY,
            result->linear_pc, (uint32_t)result->executed,
            (uint32_t)result->reason);
        return TYPE_STATUS_OK;
    }
}

type_status core_machine_request_stop(core_machine *machine)
{
    if (machine == STD_NULL) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }

    STD_ATOMIC_STORE(&machine->stop_requested, 1);
    return TYPE_STATUS_OK;
}

type_status core_machine_keyboard_submit_scan_code(core_machine *machine,
    uint8_t scan_code)
{
    if (machine == STD_NULL || machine->lifecycle == CORE_MACHINE_INITIALIZED ||
        machine->lifecycle == CORE_MACHINE_FAULTED) {
        return TYPE_STATUS_INVALID_STATE;
    }
    return core_machine_kbc_submit_scan_code(&machine->shared_kbc, scan_code);
}

type_status core_machine_keyboard_submit_scan_codes(core_machine *machine,
    const uint8_t *scan_codes, STD_SIZE_T count)
{
    if (machine == STD_NULL || machine->lifecycle == CORE_MACHINE_INITIALIZED ||
        machine->lifecycle == CORE_MACHINE_FAULTED) {
        return TYPE_STATUS_INVALID_STATE;
    }
    return core_machine_kbc_submit_scan_codes(&machine->shared_kbc, scan_codes,
        count);
}

type_status core_machine_mouse_submit_relative(core_machine *machine,
    int16_t delta_x, int16_t delta_y, uint8_t buttons)
{
    if (machine == STD_NULL || (machine->lifecycle != CORE_MACHINE_RUNNING &&
        machine->lifecycle != CORE_MACHINE_PAUSED &&
        machine->lifecycle != CORE_MACHINE_STOPPED)) {
        return TYPE_STATUS_INVALID_STATE;
    }
    return core_machine_kbc_submit_aux_report(&machine->shared_kbc, delta_x,
        delta_y, buttons);
}

type_status core_machine_report_fault(
    core_machine *machine,
    uint32_t detail)
{
    if (machine == STD_NULL) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }

    if (machine->lifecycle != CORE_MACHINE_STOPPED &&
        machine->lifecycle != CORE_MACHINE_PAUSED &&
        machine->lifecycle != CORE_MACHINE_RUNNING) {
        return TYPE_STATUS_INVALID_STATE;
    }

    machine->fault_detail = detail;
    machine->lifecycle = CORE_MACHINE_FAULTED;
    core_machine_trace_record(machine, CORE_MACHINE_TRACE_FAULT, 0u, 0u, detail);
    return TYPE_STATUS_OK;
}

C_VOID core_machine_destroy(core_machine *machine)
{
    if (machine != STD_NULL) {
        core_machine_dma_finalize(&machine->shared_dma_latch,
            &machine->shared_dma_primary, &machine->shared_dma_secondary);
        core_machine_kbc_finalize(&machine->shared_kbc);
        core_machine_pic_finalize(&machine->shared_pic_master,
            &machine->shared_pic_slave);
        core_machine_pit_finalize(&machine->shared_pit);
        core_machine_vadp_finalize(&machine->shared_vadp);
        core_machine_cpu_execution_finalize(&machine->executor_cpu_execution);
        core_machine_port_finalize(&machine->executor_port);
        core_machine_memory_finalize(&machine->executor_memory);
        for (STD_SIZE_T index = 0u; index < machine->immutable_rom_mapping_count;
                ++index) {
            STD_FREE(machine->immutable_rom_mappings[index].image);
        }
    }
    core_machine_trace_finalize(machine);
    core_machine_bus_finalize(machine);
    STD_FREE(machine);
}
