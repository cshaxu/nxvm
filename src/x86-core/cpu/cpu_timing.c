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

/* NEC October 1986 section 12 nominal rows. Operand/EA work is already included;
 * do not add the Intel EA recipe or count each byte callback as another cost.
 * Fetch starvation, prefixes and extra waits still need their own allocation. */
static void x86_core_cpu_timing_select_v30(
    const x86_core_cpu_execution_context *context, x86_core_cpu_timing_result *result)
{
    const t_cpuins_data *data = &context->instructions->data;
    const lib_u8 opcode = data->opcodes[0];
    const lib_bool scalar = opcode < 0x40u && (opcode & 7u) < 4u;
    const lib_bool shift = opcode == 0xc0u || opcode == 0xc1u ||
        (opcode >= 0xd0u && opcode <= 0xd3u);
    const lib_bool group = opcode == 0x80u || opcode == 0x81u || opcode == 0x83u || opcode == 0x8fu ||
        opcode == 0xf6u || opcode == 0xf7u || opcode == 0xfeu || opcode == 0xffu || shift;
    const lib_bool modrm = scalar || group || (opcode >= 0x84u && opcode <= 0x8eu) ||
        (opcode >= 0xc4u && opcode <= 0xc7u);
    lib_u64 ticks;
    lib_u8 odd_penalty = 0u;
    lib_u32 stack_penalty = 0u;
    lib_bool odd_word = LIB_FALSE;
    lib_bool control = LIB_FALSE, taken = LIB_TRUE;

    *result = (x86_core_cpu_timing_result){
        .source_timing_unallocated = LIB_TRUE,
        .key_id = X86_CORE_CPU_RETIREMENT_SOURCE_FORM_UNATTRIBUTED,
        .form_id = X86_CORE_CPU_RETIREMENT_SOURCE_FORM_UNATTRIBUTED
    };
    if (data->oplen == 0u || !(data->oldcpu.data.eflags & X86_CPU_NEC_PSW_MD)) return;
    if (scalar) {
        const lib_bool read_only = (opcode & 0x38u) == 0x38u || (opcode & 2u);
        ticks = !data->flagMem ? 2u : read_only ? 11u : 16u;
        odd_penalty = !(opcode & 1u) ? 0u : read_only ? 4u : 8u;
    } else if (opcode < 0x40u && (opcode & 7u) < 6u) ticks = 4u;
    else if (opcode >= 0x40u && opcode <= 0x4fu) ticks = 2u;
    else if (opcode >= 0x50u && opcode <= 0x5fu) { ticks = 8u; stack_penalty = 4u; }
    else if ((opcode >= 0x70u && opcode <= 0x7fu) || (opcode >= 0xe0u && opcode <= 0xe3u)) {
        if (!x86_core_cpu_timing_branch_taken(context, 0u, &taken)) return;
        control = LIB_TRUE;
        ticks = taken ? (opcode >= 0xe2u ? 13u : 14u) : opcode < 0xe0u ? 4u : 5u;
    } else if (opcode >= 0xb0u && opcode <= 0xbfu) ticks = 4u;
    else if (opcode >= 0x90u && opcode <= 0x97u) ticks = 3u;
    else if (shift) {
        if (((data->opcodes[1] >> 3u) & 7u) == 6u) return;
        const lib_bool single = opcode == 0xd0u || opcode == 0xd1u;
        const lib_u8 count = single ? 1u : opcode >= 0xd2u ?
            data->oldcpu.data.cl : (lib_u8)data->cimm;
        ticks = single ? (data->flagMem ? 16u : 2u) :
            (data->flagMem ? 19u : 7u) + count;
        odd_penalty = (opcode & 1u) ? 8u : 0u;
        if (!single) result->formula_inputs = X86_CORE_CPU_TIMING_INPUT_SIZE;
    }
    else switch (opcode) {
    case 0x37u: ticks = 3u; break;
    case 0x3fu: ticks = 7u; break;
    case 0x98u: ticks = 2u; break;
    case 0xd4u: case 0xd5u:
        /* Only the encoded radix-ten forms have documented nominal rows. */
        if (data->opcodes[1] != 10u) return;
        ticks = opcode == 0xd4u ? 15u : 7u;
        break;
    case 0x80u: case 0x81u: case 0x83u: {
        const lib_bool compare = ((data->opcodes[1] >> 3u) & 7u) == 7u;
        ticks = !data->flagMem ? 4u : compare ? 13u : 18u;
        odd_penalty = !(opcode & 1u) ? 0u : compare ? 4u : 8u;
        break;
    }
    case 0x84u: case 0x85u:
        ticks = data->flagMem ? 10u : 2u;
        odd_penalty = (opcode & 1u) ? 4u : 0u;
        break;
    case 0xa8u: case 0xa9u: ticks = 4u; break;
    case 0xf6u: case 0xf7u: {
        const lib_u8 operation = (data->opcodes[1] >> 3u) & 7u;
        /* TEST, NOT and NEG have fixed rows; multiply/divide and the
         * undocumented TEST alias cannot inherit their nominal costs. */
        if (operation != 0u && operation != 2u && operation != 3u) return;
        ticks = !data->flagMem ? (operation ? 2u : 4u) : operation ? 16u : 11u;
        odd_penalty = !(opcode & 1u) ? 0u : operation ? 8u : 4u;
        break;
    }
    case 0xfeu: case 0xffu: {
        const lib_u8 operation = (data->opcodes[1] >> 3u) & 7u;
        if (opcode == 0xffu && operation >= 2u && operation <= 5u) {
            if ((operation == 3u || operation == 5u) && !data->flagMem) return;
            if (operation <= 3u) {
                if (data->flagMem && ((data->mrm.offset ^ data->oldcpu.data.sp) & 1u)) return;
                ticks = !data->flagMem ? 14u : operation == 2u ? 23u : 31u;
                stack_penalty = !data->flagMem ? 4u : operation == 2u ? 8u : 16u;
            } else {
                ticks = !data->flagMem ? 11u : operation == 4u ? 20u : 27u;
                odd_penalty = !data->flagMem ? 0u : operation == 4u ? 4u : 8u;
            }
            control = LIB_TRUE;
            break;
        }
        if (opcode == 0xffu && operation == 6u) {
            /* Only all-even/all-odd source rows are documented for both addresses. */
            if (!data->flagMem || ((data->mrm.offset ^ data->oldcpu.data.sp) & 1u)) return;
            ticks = 18u;
            stack_penalty = 8u;
        } else {
            if (operation > 1u) return;
            ticks = data->flagMem ? 16u : 2u;
            odd_penalty = (opcode & 1u) ? 8u : 0u;
        }
        break;
    }
    case 0x06u: case 0x0eu: case 0x16u: case 0x1eu:
    case 0x07u: case 0x17u: case 0x1fu: case 0x68u: case 0x9cu: case 0x9du:
        ticks = 8u; stack_penalty = 4u; break;
    case 0x6au: ticks = 7u; stack_penalty = 4u; break;
    case 0x60u: ticks = 35u; stack_penalty = 32u; break;
    /* POPR's table has a 32-clock delta despite its seven data reads. */
    case 0x61u: ticks = 43u; stack_penalty = 32u; break;
    case 0x8fu:
        if (!data->flagMem || ((data->mrm.offset ^ data->oldcpu.data.sp) & 1u)) return;
        ticks = 17u;
        stack_penalty = 8u;
        break;
    case 0xc8u: {
        const lib_u8 level = data->opcodes[3];
        /* The source omits level one and mixed stack/frame alignment. */
        if (level == 1u || (level && ((data->oldcpu.data.sp ^ data->oldcpu.data.bp) & 1u))) return;
        ticks = level ? 19u + 8u * (level - 1u) : 12u;
        stack_penalty = level ? 4u + 8u * (level - 1u) : 4u;
        result->formula_inputs = X86_CORE_CPU_TIMING_INPUT_SIZE;
        break;
    }
    case 0xc9u: ticks = 6u; stack_penalty = 4u; break;
    case 0xe8u: ticks = 16u; stack_penalty = 4u; control = LIB_TRUE; break;
    case 0x9au: ticks = 21u; stack_penalty = 8u; control = LIB_TRUE; break;
    case 0xc3u: ticks = 15u; stack_penalty = 4u; control = LIB_TRUE; break;
    case 0xc2u: ticks = 20u; stack_penalty = 4u; control = LIB_TRUE; break;
    case 0xcbu: ticks = 21u; stack_penalty = 8u; control = LIB_TRUE; break;
    case 0xcau: ticks = 24u; stack_penalty = 8u; control = LIB_TRUE; break;
    case 0xe9u: case 0xebu: ticks = 12u; control = LIB_TRUE; break;
    case 0xeau: ticks = 15u; control = LIB_TRUE; break;
    case 0xe4u: case 0xe5u: case 0xe6u: case 0xe7u:
    case 0xecu: case 0xedu: case 0xeeu: case 0xefu: {
        const lib_u16 port = opcode < 0xecu ? data->opcodes[1] : data->oldcpu.data.dx;
        ticks = opcode == 0xe4u || opcode == 0xe5u ? 9u : 8u;
        if ((opcode & 1u) && (port & 1u)) { ticks += 4u; odd_word = LIB_TRUE; }
        break;
    }
    case 0xf5u: case 0xf8u: case 0xf9u: case 0xfau: case 0xfbu: case 0xfcu: case 0xfdu:
        ticks = 2u;
        break;
    case 0x88u: case 0x89u: case 0x8au: case 0x8bu:
        ticks = !data->flagMem ? 2u : opcode & 2u ? 11u : 9u;
        odd_penalty = (opcode & 1u) ? 4u : 0u;
        break;
    case 0x8cu: case 0x8eu:
        ticks = !data->flagMem ? 2u : opcode == 0x8cu ? 10u : 11u;
        odd_penalty = 4u;
        break;
    case 0xc6u: case 0xc7u:
        /* The source describes only memory destinations for this encoding. */
        if (!data->flagMem) return;
        ticks = 11u;
        odd_penalty = (opcode & 1u) ? 4u : 0u;
        break;
    case 0xa0u: case 0xa1u: case 0xa2u: case 0xa3u:
        ticks = opcode & 2u ? 9u : 10u;
        if ((opcode & 1u) && (data->opcodes[1] & 1u)) {
            ticks += 4u;
            odd_word = LIB_TRUE;
        }
        break;
    case 0xc4u: case 0xc5u:
        if (!data->flagMem) return;
        ticks = 18u;
        odd_penalty = 8u;
        break;
    case 0x8du:
        if (!data->flagMem) return;
        ticks = 4u;
        break;
    case 0x86u: case 0x87u:
        ticks = data->flagMem ? 16u : 3u;
        odd_penalty = (opcode & 1u) ? 8u : 0u;
        break;
    case 0x9eu: ticks = 3u; break;
    case 0x9fu: ticks = 2u; break;
    case 0xd7u: ticks = 9u; break;
    default: return;
    }
    if (stack_penalty) {
        result->formula_inputs |= X86_CORE_CPU_TIMING_INPUT_MEMORY;
        const lib_u16 stack = opcode == 0xc9u ? data->oldcpu.data.bp : data->oldcpu.data.sp;
        if (stack & 1u) { ticks += stack_penalty; odd_word = LIB_TRUE; }
    }
    if (modrm && data->flagMem) {
        result->formula_inputs |= X86_CORE_CPU_TIMING_INPUT_MODRM |
            X86_CORE_CPU_TIMING_INPUT_EFFECTIVE_ADDRESS;
        if (odd_penalty && (data->mrm.offset & 1u)) {
            ticks += odd_penalty;
            odd_word = LIB_TRUE;
        }
    }
    if (odd_word) result->formula_inputs |= X86_CORE_CPU_TIMING_INPUT_ODD_WORD;
    if (control) result->formula_inputs |= X86_CORE_CPU_TIMING_INPUT_CONTROL |
        X86_CORE_CPU_TIMING_INPUT_BRANCH | (taken ? X86_CORE_CPU_TIMING_INPUT_BRANCH_TAKEN : 0u);
    result->ticks = ticks;
    result->retirement_origin = X86_CORE_CPU_RETIREMENT_TIMING_ORIGIN_V30_NOMINAL_SOURCE;
    result->form_id = opcode | (modrm && data->flagMem ? 0x100u : 0u);
    if (group) result->form_id |= ((lib_u32)(data->opcodes[1] >> 3u) & 7u) << 9u;
    result->key_id = result->form_id;
}

lib_i32 x86_core_cpu_timing_select(x86_core_cpu_execution_context *context,
    x86_core_cpu_timing_result *out_result)
{
    x86_core_cpu_timing_result result;

    if (context == LIB_NULL || out_result == LIB_NULL) return 0;
    if (context->cpu_profile == X86_CORE_CPU_PROFILE_V30) {
        x86_core_cpu_timing_select_v30(context, out_result);
        return 1;
    }
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
