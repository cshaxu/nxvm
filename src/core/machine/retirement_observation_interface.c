#include "type.h"

#include "core/machine/machine.h"

static C_VOID core_machine_retirement_observation_copy_point(
    core_machine_cpu_execution_point *point, const t_cpu *cpu,
    const t_cpuins *instructions)
{
    if (point == STD_NULL || cpu == STD_NULL || instructions == STD_NULL) return;
    point->cs = cpu->data.cs.selector;
    point->cs_base = cpu->data.cs.base;
    point->eip = cpu->data.eip;
    point->linear_pc = instructions->data.linear;
    point->byte_count = (type_unsigned_8)instructions->data.oplen;
    STD_MEMCPY(point->bytes, instructions->data.opcodes, sizeof(point->bytes));
}

C_VOID core_machine_retirement_observation_initialize(core_machine *machine)
{
    if (machine != STD_NULL) {
        STD_MEMSET(&machine->retirement_observation, 0,
            sizeof(machine->retirement_observation));
    }
}

C_VOID core_machine_retirement_observation_reset(core_machine *machine)
{
    if (machine != STD_NULL) {
        machine->retirement_observation.pending = TYPE_FALSE;
    }
}

type_status core_machine_set_retirement_observation_provider(
    core_machine *machine,
    const core_machine_retirement_observation_provider *provider)
{
    if (machine == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    if (machine->lifecycle != CORE_MACHINE_STOPPED &&
        machine->lifecycle != CORE_MACHINE_PAUSED) return TYPE_STATUS_INVALID_STATE;
    if (provider != STD_NULL && provider->callback == STD_NULL) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    STD_MEMSET(&machine->retirement_observation.provider, 0,
        sizeof(machine->retirement_observation.provider));
    if (provider != STD_NULL) machine->retirement_observation.provider = *provider;
    machine->retirement_observation.pending = TYPE_FALSE;
    return TYPE_STATUS_OK;
}

C_VOID core_machine_retirement_observation_capture_instruction(core_machine *machine,
    const t_cpu *cpu, const t_cpuins *instructions)
{
    core_machine_retirement_observation *observation;

    if (machine == STD_NULL || cpu == STD_NULL || instructions == STD_NULL ||
        machine->retirement_observation.provider.callback == STD_NULL) return;
    observation = &machine->retirement_observation.pending_observation;
    STD_MEMSET(observation, 0, sizeof(*observation));
    core_machine_retirement_observation_copy_point(&observation->point, cpu,
        instructions);
    observation->cpu_profile = machine->cpu_profile;
    observation->cpl = (type_unsigned_8)cpu->data.cs.dpl;
    observation->protected_mode = (cpu->data.cr0 & VCPU_CR0_PE) != 0u;
    observation->virtual_8086_mode = (cpu->data.eflags & VCPU_EFLAGS_VM) != 0u;
    observation->operand_size_32 = cpu->data.cs.seg.exec.defsize ^
        instructions->data.prefix_oprsize;
    observation->address_size_32 = cpu->data.cs.seg.exec.defsize ^
        instructions->data.prefix_addrsize;
    observation->lock_prefix = instructions->data.flagLock;
    observation->repeat_prefix = (type_unsigned_8)instructions->data.prefix_rep;
    machine->retirement_observation.pending = TYPE_TRUE;
}

C_VOID core_machine_retirement_observation_publish(core_machine *machine,
    type_unsigned_64 source_ticks)
{
    core_machine_retirement_observation_state *state;
    core_machine_retirement_observation *observation;

    if (machine == STD_NULL) return;
    state = &machine->retirement_observation;
    if (state->provider.callback == STD_NULL || !state->pending) return;
    observation = &state->pending_observation;
    observation->sequence = state->next_sequence++;
    observation->elapsed_ticks = machine->elapsed_ticks;
    observation->timeline_ticks = machine->timeline.now;
    observation->source_ticks = source_ticks;
    observation->timing_disposition = machine->source_timing_unallocated ?
        CORE_MACHINE_RETIREMENT_TIMING_SOURCE_UNALLOCATED :
        CORE_MACHINE_RETIREMENT_TIMING_CLASSIFIED;
    observation->timing_origin = machine->source_timing_origin;
    observation->source_timing_form_id = machine->source_timing_form_id;
    state->provider.callback(state->provider.context, observation);
    state->pending = TYPE_FALSE;
}