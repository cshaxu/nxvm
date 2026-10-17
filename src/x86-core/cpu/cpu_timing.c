#include "lib/types/types_interface.h"

#include "x86-core/cpu/cpu_instructions.h"
#include "x86-core/cpu/cpu_timing.h"

static lib_i32 x86_core_cpu_timing_string_odd_word(
    const t_cpuins_data *data, lib_u32 opcode_index)
{
    lib_u8 opcode;
    lib_i32 source_transfer;
    lib_i32 destination_transfer;

    if (data == LIB_NULL || opcode_index >= data->oplen) return 0;
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

static lib_u32 x86_core_cpu_timing_capture_formula_inputs(
    x86_core_cpu_execution_context *context)
{
    const t_cpuins_data *data;
    lib_u32 prefix;
    lib_u32 opcode_index;
    lib_u32 inputs = 0u;

    if (context == LIB_NULL) return 0u;
    data = &context->instructions->data;
    prefix = 0u;
    while (prefix < data->oplen) {
        switch (data->opcodes[prefix]) {
        case 0x26u: case 0x2eu: case 0x36u: case 0x3eu:
            inputs |= X86_CORE_CPU_TIMING_INPUT_SEGMENT_OVERRIDE;
            ++prefix;
            break;
        case 0xf0u:
            inputs |= X86_CORE_CPU_TIMING_INPUT_LOCK;
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
        lib_u8 byte = data->opcodes[opcode_index];

        if (byte != 0x26u && byte != 0x2eu && byte != 0x36u && byte != 0x3eu &&
            byte != 0x64u && byte != 0x65u && byte != 0x66u && byte != 0x67u &&
            byte != 0xf0u && byte != 0xf2u && byte != 0xf3u) break;
        ++opcode_index;
    }
    if (data->flagMem) {
        inputs |= X86_CORE_CPU_TIMING_INPUT_MODRM |
            X86_CORE_CPU_TIMING_INPUT_EFFECTIVE_ADDRESS;
        if (data->mrm.rsreg != LIB_NULL &&
            ((data->mrm.rsreg->base + data->mrm.offset) & 1u) != 0u) {
            inputs |= X86_CORE_CPU_TIMING_INPUT_ODD_WORD;
        }
    }
    if (data->flagLock) inputs |= X86_CORE_CPU_TIMING_INPUT_LOCK;
    if (data->prefix_rep != PREFIX_REP_NONE) {
        inputs |= X86_CORE_CPU_TIMING_INPUT_REPEAT;
        if (context->timing_result.repeat_phase !=
            X86_CORE_CPU_RETIREMENT_REPEAT_NONE) {
            inputs |= X86_CORE_CPU_TIMING_INPUT_REPEAT_PHASE;
        }
    }
    if ((data->oldcpu.data.cr0 & VCPU_CR0_PE) != 0u ||
        (data->oldcpu.data.eflags & VCPU_EFLAGS_VM) != 0u) {
        inputs |= X86_CORE_CPU_TIMING_INPUT_MODE;
    }
    if (data->prefix_oprsize || data->prefix_addrsize) {
        inputs |= X86_CORE_CPU_TIMING_INPUT_SIZE;
    }
    if (opcode_index + 1u < data->oplen &&
        (data->opcodes[opcode_index] == 0xf6u ||
         data->opcodes[opcode_index] == 0xf7u) &&
        ((data->opcodes[opcode_index + 1u] >> 3u) & 7u) >= 4u) {
        inputs |= X86_CORE_CPU_TIMING_INPUT_GROUP3_OPERAND;
    }
    if ((context->cpu_profile == X86_CORE_CPU_PROFILE_8086 ||
         context->cpu_profile == X86_CORE_CPU_PROFILE_80186 ||
         context->cpu_profile == X86_CORE_CPU_PROFILE_80286) &&
        x86_core_cpu_timing_string_odd_word(data, opcode_index)) {
        inputs |= X86_CORE_CPU_TIMING_INPUT_ODD_WORD;
    }
    {
        lib_bool taken;

        if (x86_core_cpu_timing_branch_taken(context, opcode_index, &taken)) {
            x86_core_cpu_instruction_lexeme lexeme;

            inputs |= X86_CORE_CPU_TIMING_INPUT_CONTROL | X86_CORE_CPU_TIMING_INPUT_BRANCH;
            if (taken) inputs |= X86_CORE_CPU_TIMING_INPUT_BRANCH_TAKEN;
            /* Reuse the timing evaluator's next-term observation when present.
             * Its L2 fallback already proved the target unavailable. */
            if (taken && context->completed_observation.next_lexeme_components == 0u &&
                context->timing_result.retirement_origin != X86_CORE_CPU_RETIREMENT_TIMING_ORIGIN_L2_CONTROL_MODEL &&
                x86_core_cpu_execution_preview_lexeme(context, &lexeme) && lexeme.available)
                context->completed_observation.next_lexeme_components = lexeme.component_count;
        }
    }
    return inputs;
}

static lib_i32 x86_core_cpu_timing_try(x86_core_cpu_execution_context *context,
    x86_core_cpu_timing_result *result,
    x86_core_cpu_retirement_timing_origin origin,
    lib_i32 (*evaluate)(x86_core_cpu_execution_context *, lib_u64 *))
{
    lib_u64 ticks;

    context->timing_result.retirement_origin =
        X86_CORE_CPU_RETIREMENT_TIMING_ORIGIN_UNATTRIBUTED;
    if (evaluate == LIB_NULL || !evaluate(context, &ticks)) return 0;
    if (context->timing_result.retirement_origin ==
            X86_CORE_CPU_RETIREMENT_TIMING_ORIGIN_L2_CONTROL_MODEL ||
        context->timing_result.retirement_origin ==
            X86_CORE_CPU_RETIREMENT_TIMING_ORIGIN_L2_80486_MODEL) {
        origin = context->timing_result.retirement_origin;
    }
    result->ticks = ticks;
    result->retirement_origin = origin;
    context->timing_result.retirement_origin = origin;
    return 1;
}

/* The 80186 is the first profile being migrated from the historical shared
 * selector chain.  Candidate evaluators still live at their existing owner
 * during the incremental replacement, but only this profile-private branch
 * may select one for a successful 80186 retirement. */
static lib_i32 x86_core_cpu_timing_select_80186(x86_core_cpu_execution_context *context,
    x86_core_cpu_timing_result *result)
{
    return x86_core_cpu_timing_try(context, result,
        X86_CORE_CPU_RETIREMENT_TIMING_ORIGIN_STRING_IO,
        x86_core_cpu_string_io_source_instruction_cost) ||
        x86_core_cpu_timing_try(context, result,
            X86_CORE_CPU_RETIREMENT_TIMING_ORIGIN_L2_DYNAMIC_ARITHMETIC,
            x86_core_cpu_l2_dynamic_arithmetic_model_cost) ||
        x86_core_cpu_timing_try(context, result,
            X86_CORE_CPU_RETIREMENT_TIMING_ORIGIN_PRIMARY,
            x86_core_cpu_primary_source_instruction_cost) ||
        x86_core_cpu_timing_try(context, result,
            X86_CORE_CPU_RETIREMENT_TIMING_ORIGIN_CONTROL_STACK,
            x86_core_cpu_control_stack_source_instruction_cost) ||
        x86_core_cpu_timing_try(context, result,
            X86_CORE_CPU_RETIREMENT_TIMING_ORIGIN_80186_FALLBACK,
            x86_core_cpu_80186_source_instruction_cost);
}

/* The 80286 has a complete Appendix-B timing ledger.  Keep its candidate
 * selection profile-private: 80386-only candidates and the compatibility
 * endpoint cannot silently supply a successful 80286 retirement. */
static lib_i32 x86_core_cpu_timing_select_80286(x86_core_cpu_execution_context *context,
    x86_core_cpu_timing_result *result)
{
    return x86_core_cpu_timing_try(context, result,
        X86_CORE_CPU_RETIREMENT_TIMING_ORIGIN_STRING_IO,
        x86_core_cpu_string_io_source_instruction_cost) ||
        x86_core_cpu_timing_try(context, result,
            X86_CORE_CPU_RETIREMENT_TIMING_ORIGIN_L2_DYNAMIC_ARITHMETIC,
            x86_core_cpu_l2_dynamic_arithmetic_model_cost) ||
        x86_core_cpu_timing_try(context, result,
            X86_CORE_CPU_RETIREMENT_TIMING_ORIGIN_PRIMARY,
            x86_core_cpu_primary_source_instruction_cost) ||
        x86_core_cpu_timing_try(context, result,
            X86_CORE_CPU_RETIREMENT_TIMING_ORIGIN_CONTROL_STACK,
            x86_core_cpu_control_stack_source_instruction_cost) ||
        x86_core_cpu_timing_try(context, result,
            X86_CORE_CPU_RETIREMENT_TIMING_ORIGIN_80286_FALLBACK,
            x86_core_cpu_80286_source_instruction_cost);
}

static lib_i32 x86_core_cpu_timing_has_8086_lock_prefix(
    const t_cpuins_data *data)
{
    lib_u32 index = 0u;

    if (data == LIB_NULL) return 0;
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
static lib_i32 x86_core_cpu_timing_apply_8086_lock(x86_core_cpu_execution_context *context,
    x86_core_cpu_timing_result *result)
{
    const t_cpuins_data *data;

    if (context == LIB_NULL || result == LIB_NULL) return 0;
    if (context->cpu_profile != X86_CORE_CPU_PROFILE_8086 &&
        context->cpu_profile != X86_CORE_CPU_PROFILE_8088 &&
        context->cpu_profile != X86_CORE_CPU_PROFILE_80186) return 1;
    data = &context->instructions->data;
    if (!x86_core_cpu_timing_has_8086_lock_prefix(data) ||
        result->source_timing_unallocated) return 1;
    if (result->ticks > LIB_UINT64_MAX - 2u) return 0;
    result->ticks += 2u;
    return 1;
}

lib_i32 x86_core_cpu_timing_select(x86_core_cpu_execution_context *context,
    x86_core_cpu_timing_result *out_result)
{
    x86_core_cpu_timing_result result;

    if (context == LIB_NULL || out_result == LIB_NULL) return 0;
    context->timing_result.source_timing_unallocated = LIB_FALSE;
    context->timing_result.retirement_origin =
        X86_CORE_CPU_RETIREMENT_TIMING_ORIGIN_UNATTRIBUTED;
    context->timing_result.form_id = X86_CORE_CPU_RETIREMENT_SOURCE_FORM_UNATTRIBUTED;
    context->timing_result.key_id = X86_CORE_CPU_RETIREMENT_SOURCE_FORM_UNATTRIBUTED;
    context->timing_result.formula_inputs = 0u;
    context->timing_result.repeat_phase = X86_CORE_CPU_RETIREMENT_REPEAT_NONE;
    lib_memory_set(&result, 0, sizeof(result));
    result.event_ticks = context->timing_result.event_ticks;

    if (context->cpu_profile == X86_CORE_CPU_PROFILE_8088) {
        if (!x86_core_cpu_timing_try(context, &result,
                X86_CORE_CPU_RETIREMENT_TIMING_ORIGIN_STRING_IO,
                x86_core_cpu_string_io_source_instruction_cost) &&
            !x86_core_cpu_timing_try(context, &result,
                X86_CORE_CPU_RETIREMENT_TIMING_ORIGIN_L2_DYNAMIC_ARITHMETIC,
                x86_core_cpu_l2_dynamic_arithmetic_model_cost) &&
            !x86_core_cpu_timing_try(context, &result,
                X86_CORE_CPU_RETIREMENT_TIMING_ORIGIN_PRIMARY,
                x86_core_cpu_primary_source_instruction_cost) &&
            !x86_core_cpu_timing_try(context, &result,
                X86_CORE_CPU_RETIREMENT_TIMING_ORIGIN_CONTROL_STACK,
                x86_core_cpu_control_stack_source_instruction_cost) &&
            !x86_core_cpu_timing_try(context, &result,
                X86_CORE_CPU_RETIREMENT_TIMING_ORIGIN_COMPATIBILITY,
                x86_core_cpu_compatibility_instruction_cost)) return 0;
    } else if (context->cpu_profile == X86_CORE_CPU_PROFILE_80186) {
        if (!x86_core_cpu_timing_select_80186(context, &result)) return 0;
    } else if (context->cpu_profile == X86_CORE_CPU_PROFILE_80286) {
        if (!x86_core_cpu_timing_select_80286(context, &result)) return 0;
    } else if (context->cpu_profile == X86_CORE_CPU_PROFILE_80486SX) {
        if (!x86_core_cpu_timing_try(context, &result,
                X86_CORE_CPU_RETIREMENT_TIMING_ORIGIN_80486_SOURCE,
                x86_core_cpu_80486_source_instruction_cost)) return 0;
    } else if (!x86_core_cpu_timing_try(context, &result,
            X86_CORE_CPU_RETIREMENT_TIMING_ORIGIN_STRING_IO,
            x86_core_cpu_string_io_source_instruction_cost) &&
        !x86_core_cpu_timing_try(context, &result,
            X86_CORE_CPU_RETIREMENT_TIMING_ORIGIN_80386_DYNAMIC_MULTIPLY,
            x86_core_cpu_80386_dynamic_multiply_cost) &&
        !x86_core_cpu_timing_try(context, &result,
            X86_CORE_CPU_RETIREMENT_TIMING_ORIGIN_L2_DYNAMIC_ARITHMETIC,
            x86_core_cpu_l2_dynamic_arithmetic_model_cost) &&
        !x86_core_cpu_timing_try(context, &result,
            X86_CORE_CPU_RETIREMENT_TIMING_ORIGIN_80386_SECONDARY,
            x86_core_cpu_80386_secondary_source_instruction_cost) &&
        !x86_core_cpu_timing_try(context, &result,
            X86_CORE_CPU_RETIREMENT_TIMING_ORIGIN_80386_PRIVILEGED,
            x86_core_cpu_80386_privileged_source_instruction_cost) &&
        !x86_core_cpu_timing_try(context, &result,
            X86_CORE_CPU_RETIREMENT_TIMING_ORIGIN_PRIMARY,
            x86_core_cpu_primary_source_instruction_cost) &&
        !x86_core_cpu_timing_try(context, &result,
            X86_CORE_CPU_RETIREMENT_TIMING_ORIGIN_CONTROL_STACK,
            x86_core_cpu_control_stack_source_instruction_cost) &&
        !(context->cpu_profile == X86_CORE_CPU_PROFILE_8086 &&
          x86_core_cpu_timing_try(context, &result,
              X86_CORE_CPU_RETIREMENT_TIMING_ORIGIN_PRIMARY,
              x86_core_cpu_8086_source_instruction_cost)) &&
        !(context->cpu_profile == X86_CORE_CPU_PROFILE_80386 &&
          x86_core_cpu_timing_try(context, &result,
              X86_CORE_CPU_RETIREMENT_TIMING_ORIGIN_80386_FALLBACK,
              x86_core_cpu_80386_source_instruction_cost)) &&
        !x86_core_cpu_timing_try(context, &result,
            X86_CORE_CPU_RETIREMENT_TIMING_ORIGIN_COMPATIBILITY,
            x86_core_cpu_compatibility_instruction_cost)) {
        return 0;
    }
    if (!x86_core_cpu_timing_apply_8086_lock(context, &result)) return 0;
    result.key_id = context->timing_result.form_id;
    result.formula_inputs = context->timing_result.formula_inputs |
        x86_core_cpu_timing_capture_formula_inputs(context);
    if (result.retirement_origin ==
            X86_CORE_CPU_RETIREMENT_TIMING_ORIGIN_CONTROL_STACK ||
        result.retirement_origin ==
            X86_CORE_CPU_RETIREMENT_TIMING_ORIGIN_L2_CONTROL_MODEL) {
        result.formula_inputs |= X86_CORE_CPU_TIMING_INPUT_CONTROL;
    }
    result.source_timing_unallocated = context->timing_result.source_timing_unallocated;
    result.form_id = context->timing_result.form_id;
    result.repeat_phase = context->timing_result.repeat_phase;
    context->timing_result = result;
    *out_result = result;
    return 1;
}

x86_core_cpu_timing_result x86_core_cpu_capture_timing(
    const x86_core_cpu_execution_context *context)
{
    return context->timing_result;
}
