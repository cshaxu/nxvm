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

static type_unsigned_8 core_machine_retirement_observation_prefix_count(
    const t_cpuins_data *data)
{
    type_unsigned_8 count = 0u;

    if (data == STD_NULL) return 0u;
    while (count < data->oplen) {
        switch (data->opcodes[count]) {
        case 0x26u: case 0x2eu: case 0x36u: case 0x3eu: case 0x64u: case 0x65u:
        case 0x66u: case 0x67u: case 0xf0u: case 0xf2u: case 0xf3u:
            ++count;
            break;
        default:
            return count;
        }
    }
    return count;
}

static C_INT core_machine_retirement_observation_modrm_index(
    const t_cpuins_data *data, type_unsigned_8 opcode_index,
    type_unsigned_8 *out_index)
{
    type_unsigned_8 opcode;

    if (data == STD_NULL || out_index == STD_NULL || opcode_index >= data->oplen) {
        return 0;
    }
    opcode = data->opcodes[opcode_index];
    if (opcode == 0x0fu) {
        if (opcode_index + 2u >= data->oplen) return 0;
        switch (data->opcodes[opcode_index + 1u]) {
        case 0x01u: case 0x20u: case 0x22u:
            *out_index = (type_unsigned_8)(opcode_index + 2u);
            return 1;
        default:
            return 0;
        }
    }
    if ((opcode <= 0x3bu && (opcode & 7u) <= 3u) ||
        (opcode >= 0x80u && opcode <= 0x83u) ||
        (opcode >= 0x84u && opcode <= 0x8fu) || opcode == 0xc4u ||
        opcode == 0xc5u || opcode == 0xc6u || opcode == 0xc7u ||
        (opcode >= 0xd0u && opcode <= 0xd3u) || opcode == 0xf6u ||
        opcode == 0xf7u || opcode == 0xfeu || opcode == 0xffu) {
        if (opcode_index + 1u >= data->oplen) return 0;
        *out_index = (type_unsigned_8)(opcode_index + 1u);
        return 1;
    }
    return 0;
}

static C_VOID core_machine_retirement_observation_capture_context(
    core_machine *machine, const t_cpu *cpu, const t_cpuins *instructions,
    core_machine_retirement_observation *observation)
{
    const t_cpuins_data *data;
    core_machine_cpu_instruction_lexeme instruction_lexeme;
    core_machine_cpu_instruction_lexeme next_lexeme;
    type_unsigned_8 opcode_index;
    type_unsigned_8 opcode;
    type_unsigned_8 modrm_index;
    type_unsigned_32 fallthrough;
    if (machine == STD_NULL || cpu == STD_NULL || instructions == STD_NULL ||
        observation == STD_NULL) return;
    data = &instructions->data;
    observation->modrm_form = CORE_MACHINE_RETIREMENT_MODRM_UNAVAILABLE;
    observation->modrm_extension = CORE_MACHINE_RETIREMENT_CONTEXT_UNAVAILABLE;
    observation->control_outcome = CORE_MACHINE_RETIREMENT_CONTROL_NONE;
    observation->next_lexeme_components = CORE_MACHINE_RETIREMENT_CONTEXT_UNAVAILABLE;
    observation->repeat_phase = CORE_MACHINE_RETIREMENT_REPEAT_NONE;
    opcode_index = core_machine_retirement_observation_prefix_count(data);
    if (opcode_index >= data->oplen) return;
    opcode = data->opcodes[opcode_index];
    if (core_machine_retirement_observation_modrm_index(data, opcode_index,
            &modrm_index)) {
        observation->modrm_form = (data->opcodes[modrm_index] & 0xc0u) == 0xc0u ?
            CORE_MACHINE_RETIREMENT_MODRM_REGISTER :
            CORE_MACHINE_RETIREMENT_MODRM_MEMORY;
        observation->modrm_extension = (type_unsigned_8)(
            (data->opcodes[modrm_index] >> 3u) & 7u);
    }
    switch (opcode) {
    case 0x70u: case 0x71u: case 0x72u: case 0x73u: case 0x74u: case 0x75u:
    case 0x76u: case 0x77u: case 0x78u: case 0x79u: case 0x7au: case 0x7bu:
    case 0x7cu: case 0x7du: case 0x7eu: case 0x7fu:
    case 0xe0u: case 0xe1u: case 0xe2u: case 0xe3u:
        if (core_machine_cpu_instruction_lexeme_scan(data->opcodes + opcode_index,
                (type_unsigned_8)(sizeof(data->opcodes) - opcode_index), machine->cpu_profile,
                data->oldcpu.data.cs.seg.exec.defsize, &instruction_lexeme)) {
            fallthrough = data->oldcpu.data.eip + opcode_index +
                instruction_lexeme.byte_count;
            if (!data->oldcpu.data.cs.seg.exec.defsize) fallthrough &= 0xffffu;
            observation->control_outcome = cpu->data.eip == fallthrough ?
                CORE_MACHINE_RETIREMENT_CONTROL_FALLTHROUGH :
                CORE_MACHINE_RETIREMENT_CONTROL_TAKEN;
        }
        break;
    case 0xe9u: case 0xeau: case 0xebu: case 0xffu:
        observation->control_outcome = CORE_MACHINE_RETIREMENT_CONTROL_TAKEN;
        break;
    default:
        break;
    }
    if (observation->control_outcome == CORE_MACHINE_RETIREMENT_CONTROL_TAKEN &&
        core_machine_cpu_execution_preview_lexeme(&machine->executor_cpu_execution,
            &next_lexeme) && next_lexeme.available) {
        observation->next_lexeme_components = next_lexeme.component_count;
    }
}

static C_VOID core_machine_retirement_observation_capture_io(
    core_machine_retirement_observation *observation, const t_cpu *cpu,
    const t_cpuins_data *data)
{
    type_unsigned_8 opcode_index;
    type_unsigned_8 opcode;

    if (observation == STD_NULL || cpu == STD_NULL || data == STD_NULL) return;
    observation->io_direction = CORE_MACHINE_RETIREMENT_IO_NONE;
    observation->io_port = 0u;
    observation->io_bytes = 0u;
    observation->io_value = 0u;
    opcode_index = core_machine_retirement_observation_prefix_count(data);
    if (opcode_index >= data->oplen) return;
    opcode = data->opcodes[opcode_index];
    switch (opcode) {
    case 0xe4u: case 0xe5u: case 0xe6u: case 0xe7u:
        if (opcode_index + 1u >= data->oplen) return;
        observation->io_port = data->opcodes[opcode_index + 1u];
        break;
    case 0xecu: case 0xedu: case 0xeeu: case 0xefu:
        observation->io_port = cpu->data.dx;
        break;
    default:
        return;
    }
    observation->io_direction = (opcode == 0xe4u || opcode == 0xe5u ||
        opcode == 0xecu || opcode == 0xedu) ? CORE_MACHINE_RETIREMENT_IO_READ :
        CORE_MACHINE_RETIREMENT_IO_WRITE;
    observation->io_bytes = (opcode == 0xe4u || opcode == 0xe6u ||
        opcode == 0xecu || opcode == 0xeeu) ? 1u :
        observation->operand_size_32 ? 4u : 2u;
    if (observation->io_direction == CORE_MACHINE_RETIREMENT_IO_WRITE) {
        observation->io_value = observation->io_bytes == 1u ? cpu->data.al :
            observation->io_bytes == 2u ? cpu->data.ax : cpu->data.eax;
    }
}

C_VOID core_machine_retirement_observation_capture_instruction(core_machine *machine,
    const t_cpu *cpu, const t_cpuins *instructions)
{
    core_machine_retirement_observation *observation;

    if (machine == STD_NULL || cpu == STD_NULL || instructions == STD_NULL) return;
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
    core_machine_retirement_observation_capture_context(machine, cpu,
        instructions, observation);
    core_machine_retirement_observation_capture_io(observation, cpu,
        &instructions->data);
    machine->retirement_observation.pending =
        machine->retirement_observation.provider.callback != STD_NULL;
}

C_VOID core_machine_retirement_observation_capture_eligibility_key(
    core_machine *machine)
{
    core_machine_retirement_observation *observation;
    type_unsigned_8 opcode_index;
    type_unsigned_8 opcode = 0xffu;
    type_unsigned_8 escape_opcode = 0xffu;

    if (machine == STD_NULL) return;
    observation = &machine->retirement_observation.pending_observation;
    if (observation->io_direction == CORE_MACHINE_RETIREMENT_IO_READ) {
        observation->io_value = observation->io_bytes == 1u ?
            machine->executor_cpu.data.al : observation->io_bytes == 2u ?
            machine->executor_cpu.data.ax : machine->executor_cpu.data.eax;
    }
    core_machine_retirement_observation_capture_context(machine,
        &machine->executor_cpu, &machine->executor_cpu_instructions, observation);
    observation->repeat_phase = machine->source_timing_repeat_phase;
    opcode_index = core_machine_retirement_observation_prefix_count(
        &machine->executor_cpu_instructions.data);
    if (opcode_index < machine->executor_cpu_instructions.data.oplen) {
        opcode = machine->executor_cpu_instructions.data.opcodes[opcode_index];
        if (opcode == 0x0fu && opcode_index + 1u <
            machine->executor_cpu_instructions.data.oplen) {
            escape_opcode = machine->executor_cpu_instructions.data.opcodes[
                opcode_index + 1u];
        }
    }
    observation->eligibility_key = (core_machine_retirement_eligibility_key) {
        observation->cpu_profile, machine->source_timing_origin,
        machine->source_timing_form_id, opcode, escape_opcode,
        observation->modrm_form, observation->modrm_extension,
        observation->control_outcome, observation->next_lexeme_components,
        observation->repeat_phase, observation->cpl, observation->protected_mode,
        observation->virtual_8086_mode, observation->operand_size_32,
        observation->address_size_32, observation->lock_prefix,
        observation->repeat_prefix };
    machine->retirement_eligibility_key = observation->eligibility_key;
    machine->retirement_eligibility_key_valid = TYPE_TRUE;
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
    observation->timing_key_id = machine->source_timing_key_id;
    observation->formula_inputs = machine->source_timing_formula_inputs;
    core_machine_retirement_observation_capture_context(machine, &machine->executor_cpu,
        &machine->executor_cpu_instructions, observation);
    observation->repeat_phase = machine->source_timing_repeat_phase;
    state->provider.callback(state->provider.context, observation);
    state->pending = TYPE_FALSE;
}
