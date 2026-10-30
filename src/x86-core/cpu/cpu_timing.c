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
    lib_u32 opcode_index;
    lib_u32 inputs = 0u;

    if (context == LIB_NULL) return 0u;
    data = &context->instructions->data;
    opcode_index = 0u;
    while (opcode_index < data->oplen) {
        lib_u8 byte = data->opcodes[opcode_index];

        if (byte != 0x26u && byte != 0x2eu && byte != 0x36u && byte != 0x3eu &&
            byte != 0x64u && byte != 0x65u && byte != 0x66u && byte != 0x67u &&
            byte != 0xf0u && byte != 0xf2u && byte != 0xf3u) break;
        if (byte == 0x26u || byte == 0x2eu || byte == 0x36u || byte == 0x3eu)
            inputs |= X86_CORE_CPU_TIMING_INPUT_SEGMENT_OVERRIDE;
        else if (byte == 0xf0u)
            inputs |= X86_CORE_CPU_TIMING_INPUT_LOCK;
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
    if (!x86_core_cpu_instruction_has_lock_prefix(data,
            x86_core_cpu_instruction_prefix_count(context, data)) ||
        result->source_timing_unallocated) return 1;
    if (result->ticks > LIB_UINT64_MAX - 2u) return 0;
    result->ticks += 2u;
    return 1;
}

/* Owner-approved8080AF compatibility aggregate, not V30 physical timing.
 * The instruction decoder owns forms/conditions; this is their sole clock row. */
static void x86_core_cpu_timing_emulation_l2(const t_cpuins_data *data,
    x86_core_cpu_timing_result *result)
{
    const lib_u8 opcode=data->opcodes[0];
    const nec_emulation_form form=x86_core_cpu_emulation_form(opcode);
    const lib_u8 destination=(opcode >> 3u) & 7u, source=opcode & 7u;
    const lib_bool condition=x86_core_cpu_emulation_condition(data->oldcpu.data.eflags,destination);
    lib_bool memory=LIB_FALSE, control=LIB_FALSE, taken=LIB_FALSE;
    lib_u64 clocks;

    switch (form) {
    case NEC_EMULATION_MOV:
        memory=destination==6u || source==6u; clocks=memory ? 7u : 5u; break;
    case NEC_EMULATION_MVI: memory=destination==6u; clocks=memory ? 10u : 7u; break;
    case NEC_EMULATION_INR: case NEC_EMULATION_DCR:
        memory=destination==6u; clocks=memory ? 10u : 5u; break;
    case NEC_EMULATION_ALU: memory=source==6u; clocks=memory ? 7u : 4u; break;
    case NEC_EMULATION_ALUI: clocks=7u; break;
    case NEC_EMULATION_LXI: case NEC_EMULATION_DAD: clocks=10u; break;
    case NEC_EMULATION_INX: case NEC_EMULATION_DCX: case NEC_EMULATION_SPHL:
        clocks=5u; break;
    case NEC_EMULATION_LDAX: case NEC_EMULATION_STAX: memory=LIB_TRUE; clocks=7u; break;
    case NEC_EMULATION_LHLD: case NEC_EMULATION_SHLD: memory=LIB_TRUE; clocks=16u; break;
    case NEC_EMULATION_LDA: case NEC_EMULATION_STA: memory=LIB_TRUE; clocks=13u; break;
    case NEC_EMULATION_POP: memory=LIB_TRUE; clocks=10u; break;
    case NEC_EMULATION_PUSH: memory=LIB_TRUE; clocks=11u; break;
    case NEC_EMULATION_XTHL: memory=LIB_TRUE; clocks=18u; break;
    case NEC_EMULATION_CALL:
        control=LIB_TRUE; taken=opcode==0xcdu || condition; memory=taken;
        clocks=taken ? 17u : 11u; break;
    case NEC_EMULATION_RETURN:
        control=LIB_TRUE; taken=opcode==0xc9u || condition; memory=taken;
        clocks=opcode==0xc9u ? 10u : taken ? 11u : 5u; break;
    case NEC_EMULATION_JUMP:
        control=LIB_TRUE; taken=opcode==0xc3u || condition; clocks=10u; break;
    case NEC_EMULATION_PCHL: control=taken=LIB_TRUE; clocks=5u; break;
    case NEC_EMULATION_RST: memory=control=taken=LIB_TRUE; clocks=11u; break;
    case NEC_EMULATION_PORT: clocks=10u; break;
    case NEC_EMULATION_HLT: clocks=7u; break;
    case NEC_EMULATION_NOP: case NEC_EMULATION_ROTATE: case NEC_EMULATION_DAA:
    case NEC_EMULATION_CMA: case NEC_EMULATION_STC: case NEC_EMULATION_CMC:
    case NEC_EMULATION_XCHG: case NEC_EMULATION_EI: case NEC_EMULATION_DI:
        clocks=4u; break;
    default: return;
    }
    result->ticks=clocks;
    result->retirement_origin=X86_CORE_CPU_RETIREMENT_TIMING_ORIGIN_V30_EMULATION_L2;
    /* Ordinary emulation forms have a distinct opaque key from native bytes. */
    result->key_id=result->form_id=0x01000000u | opcode;
    result->formula_inputs=X86_CORE_CPU_TIMING_INPUT_MODE |
        (memory ? X86_CORE_CPU_TIMING_INPUT_MEMORY : 0u) |
        (control ? X86_CORE_CPU_TIMING_INPUT_CONTROL | X86_CORE_CPU_TIMING_INPUT_BRANCH : 0u) |
        (taken ? X86_CORE_CPU_TIMING_INPUT_BRANCH_TAKEN : 0u);
}

/* NEC October 1986 section 12 nominal rows. Operand/EA work is already included;
 * do not add the Intel EA recipe or count each byte callback as another cost.
 * The sole selector adds accepted fetch service; Core owns configured waits. */
static void x86_core_cpu_timing_select_v30(
    x86_core_cpu_execution_context *context, x86_core_cpu_timing_result *result)
{
    const t_cpuins_data *data = &context->instructions->data;
    const lib_bool native = (data->oldcpu.data.eflags & X86_CPU_NEC_PSW_MD) != 0u;
    const lib_u32 prefixes = native ? data->nec_prefix_bytes : 0u;
    const lib_u8 *code = data->opcodes;
    const lib_u8 opcode = native ? data->nec_opcode : code[0];
    const lib_bool string = (opcode >= 0xa4u && opcode <= 0xafu &&
        opcode != 0xa8u && opcode != 0xa9u) || (opcode >= 0x6cu && opcode <= 0x6fu);
    const lib_bool scalar = opcode < 0x40u && (opcode & 7u) < 4u;
    const lib_bool shift = opcode == 0xc0u || opcode == 0xc1u ||
        (opcode >= 0xd0u && opcode <= 0xd3u);
    const lib_bool escape = opcode == 0x66u || opcode == 0x67u ||
        (opcode >= 0xd8u && opcode <= 0xdfu);
    const lib_bool group = (opcode >= 0x80u && opcode <= 0x83u) || opcode == 0x8fu ||
        opcode == 0xf6u || opcode == 0xf7u || opcode == 0xfeu || opcode == 0xffu || shift;
    lib_bool modrm = scalar || group || escape || opcode == 0x62u || opcode == 0x69u || opcode == 0x6bu ||
        (opcode >= 0x84u && opcode <= 0x8eu) ||
        (opcode >= 0xc4u && opcode <= 0xc7u);
    x86_core_cpu_retirement_timing_origin origin = X86_CORE_CPU_RETIREMENT_TIMING_ORIGIN_V30_NOMINAL_SOURCE;
    lib_u8 form_extension = 0u;
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
    if (!data->oplen || (!native && opcode==0xedu &&
        (data->oplen<2u || (code[1]!=0xedu && code[1]!=0xfdu)))) {
        context->source_repeat_active = LIB_FALSE;
        return;
    }
    if (!native && opcode!=0xedu) {
        context->source_repeat_active=LIB_FALSE;
        x86_core_cpu_timing_emulation_l2(data,result);
        return;
    }
    if (!string) {
        context->source_repeat_active = LIB_FALSE;
    }
    if (!native) {
        form_extension = code[1];
        const lib_bool call = form_extension == 0xedu;
        /* The capture may include lookahead beyond the completed instruction. */
        if (data->oplen < (call ? 3u : 2u)) return;
        ticks = call ? 38u : 27u;
        /* CALLN's printed odd row differs from BRKEM despite the same frame. */
        stack_penalty = call ? 20u : 12u;
        control = LIB_TRUE;
    } else if (string) {
        const lib_bool repeat = data->prefix_rep != PREFIX_REP_NONE;
        const lib_bool comparison = (opcode & 0xfeu) == 0xa6u || (opcode & 0xfeu) == 0xaeu;
        form_extension = data->nec_repeat_opcode;
        lib_u64 setup = 7u, iteration;
        switch (opcode & 0xfeu) {
        case 0xa4u: ticks = 11u; setup = 11u; iteration = 8u; break;
        case 0xa6u: ticks = 13u; iteration = 14u; break;
        case 0xaau: ticks = 7u; iteration = 4u; break;
        case 0xacu: ticks = 7u; iteration = 9u; break;
        case 0xaeu: ticks = 7u; iteration = 10u; break;
        default: ticks = 10u; setup = 9u; iteration = 8u; break;
        }
        const lib_bool source = (opcode & 0xfeu) == 0xa4u || (opcode & 0xfeu) == 0xa6u ||
            (opcode & 0xfeu) == 0xacu || (opcode & 0xfeu) == 0x6eu;
        const lib_bool destination = (opcode & 0xfeu) == 0xa4u || (opcode & 0xfeu) == 0xa6u ||
            (opcode & 0xfeu) == 0xaau || (opcode & 0xfeu) == 0xaeu || (opcode & 0xfeu) == 0x6cu;
        const lib_u8 odd = !(opcode & 1u) ? (lib_u8)0u :
            (lib_u8)((source && (data->oldcpu.data.si & 1u)) +
            (destination && (data->oldcpu.data.di & 1u)) +
            (opcode < 0x70u && (data->oldcpu.data.dx & 1u)));
        x86_core_cpu_source_timing_record_repeat(context, data);
        result->repeat_phase = context->timing_result.repeat_phase;
        if (repeat) {
            const lib_bool zero = result->repeat_phase == X86_CORE_CPU_RETIREMENT_REPEAT_ZERO_COUNT;
            ticks = result->repeat_phase == X86_CORE_CPU_RETIREMENT_REPEAT_CONTINUATION ? 0u : setup;
            /* Use the printed aggregate once, not another prefix setup per iteration. */
            if (!zero) ticks += iteration + ((opcode & 0xfeu) == 0xa4u && odd ? 8u : 4u * odd);
            odd_word = !zero && odd;
            result->formula_inputs = X86_CORE_CPU_TIMING_INPUT_REPEAT |
                X86_CORE_CPU_TIMING_INPUT_REPEAT_PHASE | (zero ? 0u : X86_CORE_CPU_TIMING_INPUT_MEMORY);
            if (prefixes > 1u || (!comparison && form_extension != 0xf3u))
                origin = X86_CORE_CPU_RETIREMENT_TIMING_ORIGIN_L2_CONTROL_MODEL;
        } else {
            ticks += 4u * odd;
            odd_word = odd != 0u;
            result->formula_inputs = X86_CORE_CPU_TIMING_INPUT_MEMORY;
        }
    } else if (opcode == 0x0fu) {
        const lib_u8 suffix = data->nec_suffix;
        form_extension = suffix;
        if (suffix >= 0x10u && suffix <= 0x1fu) {
            static const lib_u8 registers[] = {3u,5u,4u,4u};
            static const lib_u8 memory[] = {12u,14u,13u,18u};
            const lib_u8 operation = (suffix >> 1u) & 3u;
            modrm = LIB_TRUE;
            ticks = (data->flagMem ? memory[operation] : registers[operation]) + (suffix >= 0x18u);
            odd_penalty = !(suffix & 1u) ? 0u : operation ? 8u : 4u;
        } else if (suffix == 0x28u || suffix == 0x2au) {
            modrm = LIB_TRUE;
            ticks = suffix == 0x28u ? (data->flagMem ? 28u : 25u) :
                data->flagMem ? 33u : 29u;
        } else if (suffix == 0x20u || suffix == 0x22u || suffix == 0x26u) {
            const lib_u8 digits = data->oldcpu.data.cl;
            if (!digits || digits == 255u) return;
            ticks = 7u + 19u * (((lib_u16)digits + 1u) / 2u);
            /* Full-byte operation is documented; rounding the nominal row is L2. */
            if (digits & 1u) origin = X86_CORE_CPU_RETIREMENT_TIMING_ORIGIN_L2_DYNAMIC_ARITHMETIC;
            result->formula_inputs = X86_CORE_CPU_TIMING_INPUT_MEMORY | X86_CORE_CPU_TIMING_INPUT_SIZE;
        } else if (suffix == 0x31u || suffix == 0x33u || suffix == 0x39u || suffix == 0x3bu) {
            /* Conservative source-range endpoints, not an inferred microsequence.
             * INS register's even upper bound really exceeds its odd row. */
            static const lib_u8 upper[][2] = {{117u,113u},{55u,59u},{87u,103u},{44u,52u}};
            const lib_bool extract = (suffix & 2u) != 0u;
            const lib_u16 index = extract ? data->oldcpu.data.si : data->oldcpu.data.di;
            const lib_u8 operation = ((suffix & 8u) ? 2u : 0u) + (extract ? 1u : 0u);
            odd_word = (index & 1u) != 0u;
            ticks = upper[operation][odd_word];
            result->formula_inputs = X86_CORE_CPU_TIMING_INPUT_MEMORY;
            origin = X86_CORE_CPU_RETIREMENT_TIMING_ORIGIN_L2_DYNAMIC_ARITHMETIC;
        } else if (suffix == 0xffu) {
            ticks = 38u;
            stack_penalty = 12u;
            control = LIB_TRUE;
        } else return;
    } else if (escape) {
        ticks = data->flagMem ? 11u : 2u;
        odd_penalty = 4u;
    } else if (scalar) {
        const lib_bool read_only = (opcode & 0x38u) == 0x38u || (opcode & 2u);
        ticks = !data->flagMem ? 2u : read_only ? 11u : 16u;
        odd_penalty = !(opcode & 1u) ? 0u : read_only ? 4u : 8u;
    } else if (opcode < 0x40u && (opcode & 7u) < 6u) ticks = 4u;
    else if (opcode >= 0x40u && opcode <= 0x4fu) ticks = 2u;
    else if (opcode >= 0x50u && opcode <= 0x5fu) { ticks = 8u; stack_penalty = 4u; }
    else if ((opcode >= 0x70u && opcode <= 0x7fu) || (opcode >= 0xe0u && opcode <= 0xe3u)) {
        if (!x86_core_cpu_timing_branch_taken(context, prefixes, &taken)) return;
        control = LIB_TRUE;
        ticks = taken ? (opcode >= 0xe2u ? 13u : 14u) : opcode < 0xe0u ? 4u : 5u;
    } else if (opcode >= 0xb0u && opcode <= 0xbfu) ticks = 4u;
    else if (opcode >= 0x90u && opcode <= 0x97u) ticks = 3u;
    else if (shift) {
        if (((data->nec_modrm >> 3u) & 7u) == 6u) return;
        const lib_bool single = opcode == 0xd0u || opcode == 0xd1u;
        const lib_u8 count = single ? 1u : opcode >= 0xd2u ?
            data->oldcpu.data.cl : (lib_u8)data->cimm;
        ticks = single ? (data->flagMem ? 16u : 2u) :
            (data->flagMem ? 19u : 7u) + count;
        odd_penalty = (opcode & 1u) ? 8u : 0u;
        if (!single) result->formula_inputs = X86_CORE_CPU_TIMING_INPUT_SIZE;
    }
    else switch (opcode) {
    case 0x62u: {
        if (!data->flagMem) return;
        taken = context->instruction_software_entry;
        const lib_bool operand_odd = (data->mrm.offset & 1u) != 0u;
        odd_word = operand_odd || (taken && (data->oldcpu.data.sp & 1u));
        /* Conservative range/mixed-alignment model; the existing comparator
         * and successful entry own the outcome, not another operand read. */
        ticks = taken ? (odd_word ? 76u : 56u) : operand_odd ? 26u : 18u;
        result->formula_inputs = X86_CORE_CPU_TIMING_INPUT_MEMORY;
        origin = X86_CORE_CPU_RETIREMENT_TIMING_ORIGIN_L2_CONTROL_MODEL;
        control = LIB_TRUE;
        break;
    }
    case 0xccu: case 0xcdu:
        ticks = 38u; stack_penalty = 12u; control = LIB_TRUE; break;
    case 0xceu:
        taken = context->instruction_software_entry;
        ticks = taken ? 40u : 3u;
        stack_penalty = taken ? 12u : 0u;
        if (!taken) origin = X86_CORE_CPU_RETIREMENT_TIMING_ORIGIN_L2_CONTROL_MODEL;
        control = LIB_TRUE;
        break;
    case 0xcfu:
        ticks = 27u; stack_penalty = 12u; control = LIB_TRUE; break;
    case 0x27u: case 0x37u: ticks = 3u; break;
    case 0x2fu: case 0x3fu: ticks = 7u; break;
    case 0xf4u: ticks = 2u; break;
    /* The instruction samples POLL once; later samples belong to wait service. */
    case 0x9bu: ticks = 7u; break;
    case 0x98u: ticks = 2u; break;
    case 0x99u:
        /* The source gives four or five according to data, without selection. */
        ticks = 5u;
        origin = X86_CORE_CPU_RETIREMENT_TIMING_ORIGIN_L2_DYNAMIC_ARITHMETIC;
        break;
    case 0xd4u: case 0xd5u:
        /* Only the encoded radix-ten forms have documented nominal rows. */
        if (data->cimm != 10u) return;
        ticks = opcode == 0xd4u ? 15u : 7u;
        break;
    case 0x80u: case 0x81u: case 0x82u: case 0x83u: {
        const lib_bool compare = ((data->nec_modrm >> 3u) & 7u) == 7u;
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
        const lib_u8 operation = (data->nec_modrm >> 3u) & 7u;
        if (operation >= 4u) {
            /* Conservative endpoints of NEC's data-dependent ranges are L2,
             * never an inferred operand-specific microsequence. DIVU is fixed. */
            static const lib_u8 upper[][2] = {{22u,30u},{39u,47u},{19u,25u},{34u,43u}};
            /* Table6-1 error aggregates replace successful division. Operand
             * deltas and mixed stack alignment are an explicit combined L2. */
            if (operation >= 6u && context->instruction_software_entry) {
                ticks = (operation == 6u ? 45u : 55u) + (data->flagMem ? 6u : 0u);
                stack_penalty = 12u;
                odd_penalty = (opcode & 1u) ? 4u : 0u;
                result->formula_inputs = X86_CORE_CPU_TIMING_INPUT_SIZE | X86_CORE_CPU_TIMING_INPUT_MEMORY;
                origin = X86_CORE_CPU_RETIREMENT_TIMING_ORIGIN_L2_CONTROL_MODEL;
                control = LIB_TRUE;
                break;
            }
            ticks = upper[operation - 4u][opcode & 1u] + (data->flagMem ? 6u : 0u);
            odd_penalty = (opcode & 1u) ? 4u : 0u;
            result->formula_inputs = X86_CORE_CPU_TIMING_INPUT_SIZE;
            if (operation != 6u) origin = X86_CORE_CPU_RETIREMENT_TIMING_ORIGIN_L2_DYNAMIC_ARITHMETIC;
            break;
        }
        /* The undocumented TEST alias has no source row. */
        if (operation == 1u) return;
        ticks = !data->flagMem ? (operation ? 2u : 4u) : operation ? 16u : 11u;
        odd_penalty = !(opcode & 1u) ? 0u : operation ? 8u : 4u;
        break;
    }
    case 0x69u: case 0x6bu:
        ticks = (opcode == 0x69u ? 42u : 34u) + (data->flagMem ? 6u : 0u);
        odd_penalty = 4u;
        result->formula_inputs = X86_CORE_CPU_TIMING_INPUT_SIZE;
        origin = X86_CORE_CPU_RETIREMENT_TIMING_ORIGIN_L2_DYNAMIC_ARITHMETIC;
        break;
    case 0xfeu: case 0xffu: {
        const lib_u8 operation = (data->nec_modrm >> 3u) & 7u;
        if (opcode == 0xffu && operation >= 2u && operation <= 5u) {
            if ((operation == 3u || operation == 5u) && !data->flagMem) return;
            if (operation <= 3u) {
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
            ticks = data->flagMem ? 18u : 8u;
            stack_penalty = data->flagMem ? 8u : 4u;
            if (!data->flagMem) origin = X86_CORE_CPU_RETIREMENT_TIMING_ORIGIN_L2_CONTROL_MODEL;
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
        ticks = data->flagMem ? 17u : 8u;
        stack_penalty = data->flagMem ? 8u : 4u;
        if (!data->flagMem) origin = X86_CORE_CPU_RETIREMENT_TIMING_ORIGIN_L2_CONTROL_MODEL;
        break;
    case 0xc8u: {
        const lib_u8 level = (lib_u8)data->cimm;
        /* Level one extends the printed n>1 formula as an explicit L2 model. */
        if (level == 1u) origin = X86_CORE_CPU_RETIREMENT_TIMING_ORIGIN_L2_CONTROL_MODEL;
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
        const lib_u16 port = opcode < 0xecu ? (lib_u8)data->cimm : data->oldcpu.data.dx;
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
        /* Register encodings use the approved equivalent MOV-immediate L2. */
        ticks = data->flagMem ? 11u : 4u;
        if (!data->flagMem) origin = X86_CORE_CPU_RETIREMENT_TIMING_ORIGIN_L2_CONTROL_MODEL;
        odd_penalty = (opcode & 1u) ? 4u : 0u;
        break;
    case 0xa0u: case 0xa1u: case 0xa2u: case 0xa3u:
        ticks = opcode & 2u ? 9u : 10u;
        if ((opcode & 1u) && (data->mrm.offset & 1u)) {
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
        lib_bool odd = (stack & 1u) != 0u;
        /* Divide-error L2 charges operand and stack alignment separately. */
        if ((modrm && data->flagMem && opcode != 0xf6u && opcode != 0xf7u) ||
            (opcode == 0xc8u && data->cimm > 1u)) {
            const lib_u16 operand = opcode == 0xc8u ? data->oldcpu.data.bp : data->mrm.offset;
            /* Missing mixed rows use the all-odd envelope, not a physical bound.
             * PREPARE level one never reads the BP frame chain. */
            if ((stack ^ operand) & 1u) origin = X86_CORE_CPU_RETIREMENT_TIMING_ORIGIN_L2_CONTROL_MODEL;
            odd = ((stack | operand) & 1u) != 0u;
        }
        if (odd) { ticks += stack_penalty; odd_word = LIB_TRUE; }
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
    if (prefixes) {
        /* Segment/BUSLOCK rows and approved extra/ignored REP decode costs. */
        const lib_u32 additional = prefixes - (string && data->prefix_rep != PREFIX_REP_NONE ? 1u : 0u);
        /* Combined repeat-prefix phase placement is an explicit L2 policy. */
        if (!string || result->repeat_phase != X86_CORE_CPU_RETIREMENT_REPEAT_CONTINUATION)
            ticks += 2u * additional;
        if (data->nec_lock_prefix)
            result->formula_inputs |= X86_CORE_CPU_TIMING_INPUT_LOCK;
        /* An override selects one segment for both default-segment routes. */
        if (data->roverds == data->roverss)
            result->formula_inputs |= X86_CORE_CPU_TIMING_INPUT_SEGMENT_OVERRIDE;
        if (!string && data->prefix_rep != PREFIX_REP_NONE)
            origin = X86_CORE_CPU_RETIREMENT_TIMING_ORIGIN_L2_CONTROL_MODEL;
    }
    result->ticks = ticks;
    result->retirement_origin = origin;
    result->form_id = opcode | ((lib_u32)form_extension << 16u) | (modrm && data->flagMem ? 0x100u : 0u);
    if (group) result->form_id |= ((lib_u32)(data->nec_modrm >> 3u) & 7u) << 9u;
    result->key_id = result->form_id;
}

lib_i32 x86_core_cpu_timing_select(x86_core_cpu_execution_context *context,
    x86_core_cpu_timing_result *out_result)
{
    x86_core_cpu_timing_result result;

    if (context == LIB_NULL || out_result == LIB_NULL) return 0;
    if (context->cpu_profile == X86_CORE_CPU_PROFILE_V30) {
        const lib_u32 fetch = context->instructions->data.timing_fetch_bus_ticks;
        x86_core_cpu_timing_select_v30(context, &result);
        if (result.ticks == 0u || !x86_core_cpu_timing_add_ticks(&result.ticks, fetch)) return 0;
        /* Explicit atomic L2: actual queue-fill service is serialized beside
         * the nominal row. Operand baseline and Core waits are not added here. */
        if (fetch != 0u) {
            result.retirement_origin = X86_CORE_CPU_RETIREMENT_TIMING_ORIGIN_V30_ELAPSED_L2;
            result.formula_inputs |= X86_CORE_CPU_TIMING_INPUT_FETCH;
        }
        result.source_timing_unallocated = LIB_FALSE;
        *out_result = result;
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
