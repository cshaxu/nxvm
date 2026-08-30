#include "type.h"

#include "core/machine/machine.h"
#include "core/machine/cpu_timing.h"

static const char *const core_machine_cpu_timing_manifest_keys[] = {
#include "cpu_timing_manifest_catalog.inc"
};

static C_INT core_machine_cpu_timing_string_odd_word(
    const t_cpuins_data *data, type_unsigned_32 opcode_index)
{
    type_unsigned_8 opcode;
    C_INT source_transfer;
    C_INT destination_transfer;

    if (data == STD_NULL || opcode_index >= data->oplen) return 0;
    opcode = data->opcodes[opcode_index];
    if (opcode != 0xa5u && opcode != 0xa7u && opcode != 0xabu &&
        opcode != 0xadu && opcode != 0xafu && opcode != 0x6du &&
        opcode != 0x6fu) return 0;
    source_transfer = opcode == 0xa5u || opcode == 0xa7u || opcode == 0xadu ||
        opcode == 0x6fu;
    destination_transfer = opcode == 0xa5u || opcode == 0xa7u ||
        opcode == 0xabu || opcode == 0xafu || opcode == 0x6du;
    return (source_transfer && (data->oldcpu.data.si & 1u) != 0u) ||
        (destination_transfer && (data->oldcpu.data.di & 1u) != 0u);
}

_Static_assert(sizeof(core_machine_cpu_timing_manifest_keys) /
    sizeof(core_machine_cpu_timing_manifest_keys[0]) == 4906u,
    "CPU timing canonical manifest count drifted");

STD_SIZE_T core_machine_cpu_timing_manifest_key_count(C_VOID)
{
    return sizeof(core_machine_cpu_timing_manifest_keys) /
        sizeof(core_machine_cpu_timing_manifest_keys[0]);
}

static type_unsigned_32 core_machine_cpu_timing_formula_inputs(
    const core_machine *machine)
{
    const t_cpuins_data *data;
    type_unsigned_32 prefix;
    type_unsigned_32 opcode_index;
    type_unsigned_32 inputs = 0u;

    if (machine == STD_NULL) return 0u;
    data = &machine->executor_cpu_instructions.data;
    prefix = 0u;
    while (prefix < data->oplen) {
        switch (data->opcodes[prefix]) {
        case 0x26u: case 0x2eu: case 0x36u: case 0x3eu:
            inputs |= CORE_MACHINE_CPU_TIMING_INPUT_SEGMENT_OVERRIDE;
            ++prefix;
            break;
        case 0xf0u:
            inputs |= CORE_MACHINE_CPU_TIMING_INPUT_LOCK;
            ++prefix;
            break;
        case 0xf2u: case 0xf3u:
        case 0x64u: case 0x65u: case 0x66u: case 0x67u:
            ++prefix;
            break;
        default:
            prefix = data->oplen;
            break;
        }
    }
    opcode_index = 0u;
    while (opcode_index < data->oplen) {
        type_unsigned_8 byte = data->opcodes[opcode_index];

        if (byte != 0x26u && byte != 0x2eu && byte != 0x36u && byte != 0x3eu &&
            byte != 0x64u && byte != 0x65u && byte != 0x66u && byte != 0x67u &&
            byte != 0xf0u && byte != 0xf2u && byte != 0xf3u) break;
        ++opcode_index;
    }
    if (data->flagMem) {
        inputs |= CORE_MACHINE_CPU_TIMING_INPUT_MODRM |
            CORE_MACHINE_CPU_TIMING_INPUT_EFFECTIVE_ADDRESS;
        if (data->mrm.rsreg != STD_NULL &&
            ((data->mrm.rsreg->base + data->mrm.offset) & 1u) != 0u) {
            inputs |= CORE_MACHINE_CPU_TIMING_INPUT_ODD_WORD;
        }
    }
    if (data->flagLock) inputs |= CORE_MACHINE_CPU_TIMING_INPUT_LOCK;
    if (data->prefix_rep != PREFIX_REP_NONE) {
        inputs |= CORE_MACHINE_CPU_TIMING_INPUT_REPEAT;
        if (machine->source_timing_repeat_phase !=
            CORE_MACHINE_RETIREMENT_REPEAT_NONE) {
            inputs |= CORE_MACHINE_CPU_TIMING_INPUT_REPEAT_PHASE;
        }
    }
    if ((data->oldcpu.data.cr0 & VCPU_CR0_PE) != 0u ||
        (data->oldcpu.data.eflags & VCPU_EFLAGS_VM) != 0u) {
        inputs |= CORE_MACHINE_CPU_TIMING_INPUT_MODE;
    }
    if (data->prefix_oprsize || data->prefix_addrsize) {
        inputs |= CORE_MACHINE_CPU_TIMING_INPUT_SIZE;
    }
    if (opcode_index + 1u < data->oplen &&
        (data->opcodes[opcode_index] == 0xf6u ||
         data->opcodes[opcode_index] == 0xf7u) &&
        ((data->opcodes[opcode_index + 1u] >> 3u) & 7u) >= 4u) {
        inputs |= CORE_MACHINE_CPU_TIMING_INPUT_GROUP3_OPERAND;
    }
    if ((machine->cpu_profile == CORE_MACHINE_CPU_PROFILE_8086 ||
         machine->cpu_profile == CORE_MACHINE_CPU_PROFILE_80186 ||
         machine->cpu_profile == CORE_MACHINE_CPU_PROFILE_80286) &&
        core_machine_cpu_timing_string_odd_word(data, opcode_index)) {
        inputs |= CORE_MACHINE_CPU_TIMING_INPUT_ODD_WORD;
    }
    if (opcode_index < data->oplen && data->opcodes[opcode_index] == 0x9bu &&
        core_machine_fpu_last_wait_iterations(&machine->fpu) != 0u) {
        inputs |= CORE_MACHINE_CPU_TIMING_INPUT_WAIT_ITERATIONS;
    }
    return inputs;
}

static C_INT core_machine_cpu_timing_try(core_machine *machine,
    core_machine_cpu_timing_result *result,
    core_machine_retirement_timing_origin origin,
    C_INT (*evaluate)(core_machine *, type_unsigned_64 *))
{
    type_unsigned_64 ticks;

    if (evaluate == STD_NULL || !evaluate(machine, &ticks)) return 0;
    result->ticks = ticks;
    result->retirement_origin = origin;
    machine->source_timing_origin = origin;
    return 1;
}

/* The 80186 is the first profile being migrated from the historical shared
 * selector chain.  Candidate evaluators still live at their existing owner
 * during the incremental replacement, but only this profile-private branch
 * may select one for a successful 80186 retirement. */
static C_INT core_machine_cpu_timing_select_80186(core_machine *machine,
    core_machine_cpu_timing_result *result)
{
    return core_machine_cpu_timing_try(machine, result,
        CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_STRING_IO,
        core_machine_string_io_source_instruction_cost) ||
        core_machine_cpu_timing_try(machine, result,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_L2_DYNAMIC_ARITHMETIC,
            core_machine_l2_dynamic_arithmetic_model_cost) ||
        core_machine_cpu_timing_try(machine, result,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY,
            core_machine_primary_source_instruction_cost) ||
        core_machine_cpu_timing_try(machine, result,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_CONTROL_STACK,
            core_machine_control_stack_source_instruction_cost) ||
        core_machine_cpu_timing_try(machine, result,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_80186_FALLBACK,
            core_machine_80186_source_instruction_cost);
}

/* The 80286 has a complete Appendix-B timing ledger.  Keep its candidate
 * selection profile-private: 80386-only candidates and the compatibility
 * endpoint cannot silently supply a successful 80286 retirement. */
static C_INT core_machine_cpu_timing_select_80286(core_machine *machine,
    core_machine_cpu_timing_result *result)
{
    return core_machine_cpu_timing_try(machine, result,
        CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_STRING_IO,
        core_machine_string_io_source_instruction_cost) ||
        core_machine_cpu_timing_try(machine, result,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_L2_DYNAMIC_ARITHMETIC,
            core_machine_l2_dynamic_arithmetic_model_cost) ||
        core_machine_cpu_timing_try(machine, result,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY,
            core_machine_primary_source_instruction_cost) ||
        core_machine_cpu_timing_try(machine, result,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_CONTROL_STACK,
            core_machine_control_stack_source_instruction_cost) ||
        core_machine_cpu_timing_try(machine, result,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_80286_FALLBACK,
            core_machine_80286_source_instruction_cost);
}

static C_INT core_machine_cpu_timing_has_8086_lock_prefix(
    const t_cpuins_data *data)
{
    type_unsigned_32 index = 0u;

    if (data == STD_NULL) return 0;
    while (index < data->oplen) {
        switch (data->opcodes[index]) {
        case 0x26u: case 0x2eu: case 0x36u: case 0x3eu:
        case 0xf0u: case 0xf2u: case 0xf3u:
            if (data->opcodes[index] == 0xf0u) return 1;
            ++index;
            break;
        default:
            return 0;
        }
    }
    return 0;
}

/* Intel's 8086/8088/80186 timing tables assign LOCK a two-clock prefix term. Its
 * applicability is a decoder/semantic question; once a valid instruction
 * has retired and one source row owns its base cost, this sole selector owns
 * the additive clock term. */
static C_INT core_machine_cpu_timing_apply_8086_lock(core_machine *machine,
    core_machine_cpu_timing_result *result)
{
    const t_cpuins_data *data;

    if (machine == STD_NULL || result == STD_NULL) return 0;
    if (machine->cpu_profile != CORE_MACHINE_CPU_PROFILE_8086 &&
        machine->cpu_profile != CORE_MACHINE_CPU_PROFILE_8088 &&
        machine->cpu_profile != CORE_MACHINE_CPU_PROFILE_80186) return 1;
    data = &machine->executor_cpu_instructions.data;
    if (!core_machine_cpu_timing_has_8086_lock_prefix(data) ||
        result->source_timing_unallocated) return 1;
    if (result->ticks > UINT64_MAX - 2u) return 0;
    result->ticks += 2u;
    return 1;
}

C_INT core_machine_cpu_timing_select(core_machine *machine,
    core_machine_cpu_timing_result *out_result)
{
    core_machine_cpu_timing_result result;

    if (machine == STD_NULL || out_result == STD_NULL) return 0;
    machine->source_timing_unallocated = TYPE_FALSE;
    machine->source_timing_origin =
        CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_UNATTRIBUTED;
    machine->source_timing_form_id = CORE_MACHINE_RETIREMENT_SOURCE_FORM_UNATTRIBUTED;
    machine->source_timing_key_id = CORE_MACHINE_RETIREMENT_SOURCE_FORM_UNATTRIBUTED;
    machine->source_timing_formula_inputs = 0u;
    machine->source_timing_repeat_phase = CORE_MACHINE_RETIREMENT_REPEAT_NONE;
    STD_MEMSET(&result, 0, sizeof(result));

    if (machine->cpu_profile == CORE_MACHINE_CPU_PROFILE_8088) {
        if (!core_machine_cpu_timing_try(machine, &result,
                CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_STRING_IO,
                core_machine_string_io_source_instruction_cost) &&
            !core_machine_cpu_timing_try(machine, &result,
                CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_L2_DYNAMIC_ARITHMETIC,
                core_machine_l2_dynamic_arithmetic_model_cost) &&
            !core_machine_cpu_timing_try(machine, &result,
                CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY,
                core_machine_primary_source_instruction_cost) &&
            !core_machine_cpu_timing_try(machine, &result,
                CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_CONTROL_STACK,
                core_machine_control_stack_source_instruction_cost) &&
            !core_machine_cpu_timing_try(machine, &result,
                CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_COMPATIBILITY,
                core_machine_compatibility_instruction_cost)) return 0;
    } else if (machine->cpu_profile == CORE_MACHINE_CPU_PROFILE_80186) {
        if (!core_machine_cpu_timing_select_80186(machine, &result)) return 0;
    } else if (machine->cpu_profile == CORE_MACHINE_CPU_PROFILE_80286) {
        if (!core_machine_cpu_timing_select_80286(machine, &result)) return 0;
    } else if (!core_machine_cpu_timing_try(machine, &result,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_STRING_IO,
            core_machine_string_io_source_instruction_cost) &&
        !core_machine_cpu_timing_try(machine, &result,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_80386_DYNAMIC_MULTIPLY,
            core_machine_80386_dynamic_multiply_cost) &&
        !core_machine_cpu_timing_try(machine, &result,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_L2_DYNAMIC_ARITHMETIC,
            core_machine_l2_dynamic_arithmetic_model_cost) &&
        !core_machine_cpu_timing_try(machine, &result,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_80386_SECONDARY,
            core_machine_80386_secondary_source_instruction_cost) &&
        !core_machine_cpu_timing_try(machine, &result,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_80386_PRIVILEGED,
            core_machine_80386_privileged_source_instruction_cost) &&
        !core_machine_cpu_timing_try(machine, &result,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY,
            core_machine_primary_source_instruction_cost) &&
        !core_machine_cpu_timing_try(machine, &result,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_CONTROL_STACK,
            core_machine_control_stack_source_instruction_cost) &&
        !(machine->cpu_profile == CORE_MACHINE_CPU_PROFILE_8086 &&
          core_machine_cpu_timing_try(machine, &result,
              CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY,
              core_machine_8086_source_instruction_cost)) &&
        !(machine->cpu_profile == CORE_MACHINE_CPU_PROFILE_80386 &&
          core_machine_cpu_timing_try(machine, &result,
              CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_80386_FALLBACK,
              core_machine_80386_source_instruction_cost)) &&
        !core_machine_cpu_timing_try(machine, &result,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_COMPATIBILITY,
            core_machine_compatibility_instruction_cost)) {
        return 0;
    }
    if (!core_machine_cpu_timing_apply_8086_lock(machine, &result)) return 0;
    result.key_id = machine->source_timing_form_id;
    result.formula_inputs = core_machine_cpu_timing_formula_inputs(machine);
    if (result.retirement_origin ==
        CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_CONTROL_STACK) {
        result.formula_inputs |= CORE_MACHINE_CPU_TIMING_INPUT_CONTROL;
    }
    result.source_timing_unallocated = machine->source_timing_unallocated;
    machine->source_timing_key_id = result.key_id;
    machine->source_timing_formula_inputs = result.formula_inputs;
    *out_result = result;
    return 1;
}
