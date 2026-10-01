#include "lib/types/types_interface.h"

#include "app-nxvm/devices/machine.h"
#include "x86/devices/cpu/cpu_interface.h"

void core_machine_retirement_observation_initialize(core_machine *machine)
{
    if (machine != LIB_NULL) {
        lib_memory_set(&machine->retirement_observation, 0,
            sizeof(machine->retirement_observation));
    }
}

void core_machine_retirement_observation_reset(core_machine *machine)
{
    if (machine != LIB_NULL) {
        machine->retirement_observation.pending = LIB_FALSE;
    }
}

lib_status core_machine_set_retirement_observation_provider(
    core_machine *machine,
    const core_machine_retirement_observation_provider *provider)
{
    if (machine == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    if (machine->lifecycle != CORE_MACHINE_STOPPED &&
        machine->lifecycle != CORE_MACHINE_PAUSED) return LIB_STATUS_INVALID_STATE;
    if (provider != LIB_NULL && provider->callback == LIB_NULL) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    lib_memory_set(&machine->retirement_observation.provider, 0,
        sizeof(machine->retirement_observation.provider));
    if (provider != LIB_NULL) machine->retirement_observation.provider = *provider;
    machine->retirement_observation.pending = LIB_FALSE;
    core_machine_cpu_execution_context_bind_diagnostic_provider(
        machine->executor_cpu_execution,
        (provider != LIB_NULL || machine->retirement_time_contract ==
            CORE_MACHINE_RETIREMENT_TIME_PHYSICAL) ?
            &core_machine_cpu_diagnostic_provider :
            &core_machine_cpu_fault_diagnostic_provider,
        machine);
    return LIB_STATUS_OK;
}

static lib_u8 core_machine_retirement_observation_prefix_count(
    const core_machine_cpu_instruction_observation *data)
{
    lib_u8 count = 0u;

    if (data == LIB_NULL) return 0u;
    while (count < data->point.byte_count) {
        switch (data->point.bytes[count]) {
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

static lib_i32 core_machine_retirement_observation_modrm_index(
    const core_machine_cpu_instruction_observation *data, lib_u8 opcode_index,
    lib_u8 *out_index)
{
    lib_u8 opcode;

    if (data == LIB_NULL || out_index == LIB_NULL || opcode_index >= data->point.byte_count) {
        return 0;
    }
    opcode = data->point.bytes[opcode_index];
    if (opcode == 0x0fu) {
        if (opcode_index + 2u >= data->point.byte_count) return 0;
        switch (data->point.bytes[opcode_index + 1u]) {
        case 0x01u: case 0x20u: case 0x22u:
            *out_index = (lib_u8)(opcode_index + 2u);
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
        if (opcode_index + 1u >= data->point.byte_count) return 0;
        *out_index = (lib_u8)(opcode_index + 1u);
        return 1;
    }
    return 0;
}

static void core_machine_retirement_observation_capture_context(
    core_machine *machine, const core_machine_cpu_instruction_observation *data,
    core_machine_retirement_observation *observation)
{
    core_machine_cpu_instruction_lexeme instruction_lexeme;
    core_machine_cpu_instruction_lexeme next_lexeme;
    lib_u8 opcode_index;
    lib_u8 opcode;
    lib_u8 modrm_index;
    lib_u32 fallthrough;
    if (machine == LIB_NULL || data == LIB_NULL ||
        observation == LIB_NULL) return;
    observation->modrm_form = CORE_MACHINE_RETIREMENT_MODRM_UNAVAILABLE;
    observation->modrm_extension = CORE_MACHINE_RETIREMENT_CONTEXT_UNAVAILABLE;
    observation->control_outcome = CORE_MACHINE_RETIREMENT_CONTROL_NONE;
    observation->next_lexeme_components = CORE_MACHINE_RETIREMENT_CONTEXT_UNAVAILABLE;
    observation->repeat_phase = CORE_MACHINE_RETIREMENT_REPEAT_NONE;
    opcode_index = core_machine_retirement_observation_prefix_count(data);
    if (opcode_index >= data->point.byte_count) return;
    opcode = data->point.bytes[opcode_index];
    if (core_machine_retirement_observation_modrm_index(data, opcode_index,
            &modrm_index)) {
        observation->modrm_form = (data->point.bytes[modrm_index] & 0xc0u) == 0xc0u ?
            CORE_MACHINE_RETIREMENT_MODRM_REGISTER :
            CORE_MACHINE_RETIREMENT_MODRM_MEMORY;
        observation->modrm_extension = (lib_u8)(
            (data->point.bytes[modrm_index] >> 3u) & 7u);
    }
    switch (opcode) {
    case 0x70u: case 0x71u: case 0x72u: case 0x73u: case 0x74u: case 0x75u:
    case 0x76u: case 0x77u: case 0x78u: case 0x79u: case 0x7au: case 0x7bu:
    case 0x7cu: case 0x7du: case 0x7eu: case 0x7fu:
    case 0xe0u: case 0xe1u: case 0xe2u: case 0xe3u:
        if (core_machine_cpu_instruction_lexeme_scan(data->point.bytes + opcode_index,
                (lib_u8)(sizeof(data->point.bytes) - opcode_index), machine->cpu_profile,
                data->old_default_size_32, &instruction_lexeme)) {
            fallthrough = data->old_eip + opcode_index +
                instruction_lexeme.byte_count;
            if (!data->old_default_size_32) fallthrough &= 0xffffu;
            observation->control_outcome = data->point.eip == fallthrough ?
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
        core_machine_cpu_execution_preview_lexeme(machine->executor_cpu_execution,
            &next_lexeme) && next_lexeme.available) {
        observation->next_lexeme_components = next_lexeme.component_count;
    }
}

static void core_machine_retirement_observation_capture_io(
    core_machine_retirement_observation *observation,
    const core_machine_cpu_instruction_observation *data)
{
    lib_u8 opcode_index;
    lib_u8 opcode;

    if (observation == LIB_NULL || data == LIB_NULL) return;
    observation->io_direction = CORE_MACHINE_RETIREMENT_IO_NONE;
    observation->io_port = 0u;
    observation->io_bytes = 0u;
    observation->io_value = 0u;
    opcode_index = core_machine_retirement_observation_prefix_count(data);
    if (opcode_index >= data->point.byte_count) return;
    opcode = data->point.bytes[opcode_index];
    switch (opcode) {
    case 0xe4u: case 0xe5u: case 0xe6u: case 0xe7u:
        if (opcode_index + 1u >= data->point.byte_count) return;
        observation->io_port = data->point.bytes[opcode_index + 1u];
        break;
    case 0xecu: case 0xedu: case 0xeeu: case 0xefu:
        observation->io_port = data->dx;
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
        observation->io_value = observation->io_bytes == 1u ? (data->eax & 0xffu) :
            observation->io_bytes == 2u ? (data->eax & 0xffffu) : data->eax;
    }
}

void core_machine_retirement_observation_capture_instruction(core_machine *machine,
    const core_machine_cpu_instruction_observation *data)
{
    core_machine_retirement_observation *observation;

    if (machine == LIB_NULL || data == LIB_NULL) return;
    observation = &machine->retirement_observation.pending_observation;
    /* Register copies are filled at publication only. Keep their work out of
     * the per-instruction eligibility path when no observer is installed. */
    lib_memory_set(observation, 0,
        lib_offsetof(core_machine_retirement_observation, instruction_entry_cpu));
    observation->point = data->point;
    observation->cpu_profile = machine->cpu_profile;
    observation->cpl = data->cpl;
    observation->protected_mode = data->protected_mode;
    observation->virtual_8086_mode = data->virtual_8086_mode;
    observation->operand_size_32 = data->operand_size_32;
    observation->address_size_32 = data->address_size_32;
    observation->lock_prefix = data->lock_prefix;
    observation->repeat_prefix = data->repeat_prefix;
    core_machine_retirement_observation_capture_context(machine, data, observation);
    core_machine_retirement_observation_capture_io(observation, data);
    machine->retirement_observation.pending =
        machine->retirement_observation.provider.callback != LIB_NULL;
}

void core_machine_retirement_observation_capture_eligibility_key(
    core_machine *machine)
{
    core_machine_retirement_observation *observation;
    core_machine_cpu_instruction_observation data;
    core_machine_cpu_timing_result timing;
    lib_u8 opcode_index;
    lib_u8 opcode = 0xffu;
    lib_u8 escape_opcode = 0xffu;

    if (machine == LIB_NULL) return;
    timing = core_machine_cpu_capture_timing(machine->executor_cpu_execution);
    observation = &machine->retirement_observation.pending_observation;
    core_machine_cpu_execution_copy_observation(machine->executor_cpu_execution, &data);
    if (observation->io_direction == CORE_MACHINE_RETIREMENT_IO_READ) {
        observation->io_value = observation->io_bytes == 1u ?
            (data.eax & 0xffu) : observation->io_bytes == 2u ?
            (data.eax & 0xffffu) : data.eax;
    }
    core_machine_retirement_observation_capture_context(machine,
        &data, observation);
    observation->repeat_phase = timing.repeat_phase;
    opcode_index = core_machine_retirement_observation_prefix_count(
        &data);
    if (opcode_index < data.point.byte_count) {
        opcode = data.point.bytes[opcode_index];
        if (opcode == 0x0fu && opcode_index + 1u <
            data.point.byte_count) {
            escape_opcode = data.point.bytes[
                opcode_index + 1u];
        }
    }
    observation->eligibility_key = (core_machine_retirement_eligibility_key) {
        observation->cpu_profile, timing.retirement_origin,
        timing.form_id, opcode, escape_opcode,
        observation->modrm_form, observation->modrm_extension,
        observation->control_outcome, observation->next_lexeme_components,
        observation->repeat_phase, observation->cpl, observation->protected_mode,
        observation->virtual_8086_mode, observation->operand_size_32,
        observation->address_size_32, observation->lock_prefix,
        observation->repeat_prefix };
    machine->retirement_eligibility_key = observation->eligibility_key;
    machine->retirement_eligibility_key_valid = LIB_TRUE;
}
void core_machine_retirement_observation_publish(core_machine *machine,
    lib_u64 source_ticks)
{
    core_machine_retirement_observation_state *state;
    core_machine_cpu_instruction_observation data;
    core_machine_cpu_timing_result timing;
    core_machine_retirement_observation *observation;

    if (machine == LIB_NULL) return;
    timing = core_machine_cpu_capture_timing(machine->executor_cpu_execution);
    state = &machine->retirement_observation;
    if (state->provider.callback == LIB_NULL || !state->pending) return;
    observation = &state->pending_observation;
    observation->sequence = state->next_sequence++;
    observation->elapsed_ticks = machine->elapsed_ticks;
    observation->timeline_ticks = machine->timeline.now;
    observation->source_ticks = source_ticks;
    observation->timing_disposition = timing.source_timing_unallocated ?
        CORE_MACHINE_RETIREMENT_TIMING_SOURCE_UNALLOCATED :
        CORE_MACHINE_RETIREMENT_TIMING_CLASSIFIED;
    observation->timing_origin = timing.retirement_origin;
    observation->source_timing_form_id = timing.form_id;
    observation->timing_key_id = timing.key_id;
    observation->formula_inputs = timing.formula_inputs;
    core_machine_cpu_execution_copy_observation(machine->executor_cpu_execution, &data);
    core_machine_retirement_observation_capture_context(machine, &data, observation);
    observation->repeat_phase = timing.repeat_phase;
    (void)core_machine_cpu_debug_capture_snapshot(machine->executor_cpu_execution,
        CORE_MACHINE_CPU_SNAPSHOT_INSTRUCTION_ENTRY, &observation->instruction_entry_cpu);
    (void)core_machine_cpu_debug_capture_snapshot(machine->executor_cpu_execution,
        CORE_MACHINE_CPU_SNAPSHOT_CURRENT, &observation->current_cpu);
    state->provider.callback(state->provider.context, observation);
    state->pending = LIB_FALSE;
}
