#include "type.h"

#include "core/machine/machine.h"
#include "core/machine/cpu_timing.h"

static const char *const core_machine_cpu_timing_manifest_keys[] = {
#include "cpu_timing_manifest_catalog.inc"
};

_Static_assert(sizeof(core_machine_cpu_timing_manifest_keys) /
    sizeof(core_machine_cpu_timing_manifest_keys[0]) == 3295u,
    "T435 S2 canonical manifest count drifted");

STD_SIZE_T core_machine_cpu_timing_manifest_key_count(C_VOID)
{
    return sizeof(core_machine_cpu_timing_manifest_keys) /
        sizeof(core_machine_cpu_timing_manifest_keys[0]);
}

static type_unsigned_32 core_machine_cpu_timing_formula_inputs(
    const core_machine *machine)
{
    const t_cpuins_data *data;
    type_unsigned_32 inputs = 0u;

    if (machine == STD_NULL) return 0u;
    data = &machine->executor_cpu_instructions.data;
    if (data->flagMem) inputs |= CORE_MACHINE_CPU_TIMING_INPUT_MODRM;
    if (data->flagLock) inputs |= CORE_MACHINE_CPU_TIMING_INPUT_LOCK;
    if (data->prefix_rep != PREFIX_REP_NONE) {
        inputs |= CORE_MACHINE_CPU_TIMING_INPUT_REPEAT;
    }
    if ((data->oldcpu.data.cr0 & VCPU_CR0_PE) != 0u ||
        (data->oldcpu.data.eflags & VCPU_EFLAGS_VM) != 0u) {
        inputs |= CORE_MACHINE_CPU_TIMING_INPUT_MODE;
    }
    if (data->prefix_oprsize || data->prefix_addrsize) {
        inputs |= CORE_MACHINE_CPU_TIMING_INPUT_SIZE;
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

    if (!core_machine_cpu_timing_try(machine, &result,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_STRING_IO,
            core_machine_string_io_source_instruction_cost) &&
        !core_machine_cpu_timing_try(machine, &result,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_80386_DYNAMIC_MULTIPLY,
            core_machine_80386_dynamic_multiply_cost) &&
        !core_machine_cpu_timing_try(machine, &result,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_LEGACY_DYNAMIC_ARITHMETIC,
            core_machine_legacy_dynamic_arithmetic_model_cost) &&
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
              CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_8086_FALLBACK,
              core_machine_8086_source_instruction_cost)) &&
        !(machine->cpu_profile == CORE_MACHINE_CPU_PROFILE_80186 &&
          core_machine_cpu_timing_try(machine, &result,
              CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_80186_FALLBACK,
              core_machine_80186_source_instruction_cost)) &&
        !(machine->cpu_profile == CORE_MACHINE_CPU_PROFILE_80286 &&
          core_machine_cpu_timing_try(machine, &result,
              CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_80286_FALLBACK,
              core_machine_80286_source_instruction_cost)) &&
        !(machine->cpu_profile == CORE_MACHINE_CPU_PROFILE_80386 &&
          core_machine_cpu_timing_try(machine, &result,
              CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_80386_FALLBACK,
              core_machine_80386_source_instruction_cost)) &&
        !core_machine_cpu_timing_try(machine, &result,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_COMPATIBILITY,
            core_machine_compatibility_instruction_cost)) {
        return 0;
    }
    result.key_id = machine->source_timing_form_id;
    result.formula_inputs = core_machine_cpu_timing_formula_inputs(machine);
    result.source_timing_unallocated = machine->source_timing_unallocated;
    machine->source_timing_key_id = result.key_id;
    machine->source_timing_formula_inputs = result.formula_inputs;
    *out_result = result;
    return 1;
}
