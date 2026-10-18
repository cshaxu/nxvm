#include "support/cpu_v30_memory_fixture.h"
#include "lib/types/file.h"

typedef enum v30_control_form {
    V30_CONTROL_CONDITIONAL, V30_CONTROL_LOOP, V30_CONTROL_JUMP_BYTE,
    V30_CONTROL_JUMP_WORD, V30_CONTROL_CALL_WORD,
    V30_CONTROL_FAR_JUMP, V30_CONTROL_FAR_CALL,
    V30_CONTROL_INDIRECT_JUMP, V30_CONTROL_INDIRECT_CALL,
    V30_CONTROL_INDIRECT_FAR_JUMP, V30_CONTROL_INDIRECT_FAR_CALL,
    V30_CONTROL_RETURN, V30_CONTROL_FAR_RETURN,
    V30_CONTROL_RETURN_CLEANUP, V30_CONTROL_FAR_RETURN_CLEANUP
} v30_control_form;

typedef struct v30_control_case {
    v30_control_form form;
    lib_u16 pc, sp, count, target, segment, cleanup;
    lib_u8 condition, modrm, prefix;
    lib_u32 flags;
    lib_bool overlap;
} v30_control_case;

static lib_u32 *v30_control_word(t_cpu *cpu, lib_u8 reg)
{
    lib_u32 *words[] = {&cpu->data.eax,&cpu->data.ecx,&cpu->data.edx,&cpu->data.ebx,
        &cpu->data.esp,&cpu->data.ebp,&cpu->data.esi,&cpu->data.edi};
    return words[reg];
}

static lib_u32 v30_control_address(const t_cpu *cpu, const v30_control_case *item)
{
    const lib_u8 mod = item->modrm >> 6u, rm = item->modrm & 7u;
    const lib_u16 offsets[] = {cpu->data.bx+cpu->data.si,cpu->data.bx+cpu->data.di,
        cpu->data.bp+cpu->data.si,cpu->data.bp+cpu->data.di,cpu->data.si,
        cpu->data.di,cpu->data.bp,cpu->data.bx};
    const lib_bool stack = rm == 2u || rm == 3u || (rm == 6u && mod != 0u);
    const lib_u32 bases[] = {stack ? 0x20000u : 0x10000u,0x30000u,0x80000u,0x20000u,0x10000u};
    return bases[item->prefix] + (lib_u16)(mod == 0u && rm == 6u ? 0xffffu :
        offsets[rm] + (mod == 1u ? 0u : mod == 2u ? 0xffffu : 0u));
}

static lib_u32 v30_control_flags(lib_u8 bits)
{
    static const lib_u32 flags[] = {VCPU_EFLAGS_CF,VCPU_EFLAGS_PF,VCPU_EFLAGS_ZF,
        VCPU_EFLAGS_SF,VCPU_EFLAGS_OF};
    lib_u32 result = VCPU_EFLAGS_AF | VCPU_EFLAGS_DF | VCPU_EFLAGS_IF;
    for (lib_u8 bit = 0u; bit < sizeof(flags)/sizeof(flags[0]); ++bit)
        if (bits & (1u << bit)) result |= flags[bit];
    return result;
}

static lib_bool v30_control_condition(lib_u8 condition, lib_u32 flags)
{
    const lib_bool carry = (flags & VCPU_EFLAGS_CF) != 0u;
    const lib_bool zero = (flags & VCPU_EFLAGS_ZF) != 0u;
    const lib_bool sign = (flags & VCPU_EFLAGS_SF) != 0u;
    const lib_bool overflow = (flags & VCPU_EFLAGS_OF) != 0u;
    const lib_bool parity = (flags & VCPU_EFLAGS_PF) != 0u;
    const lib_bool predicates[] = {overflow,carry,zero,carry || zero,sign,parity,
        sign != overflow,zero || sign != overflow};
    return predicates[condition/2u] != (lib_bool)(condition & 1u);
}

static void v30_control_store(v30_memory_fixture *fixture, lib_u32 address,
    lib_u32 value, lib_u8 bytes)
{
    for (lib_u8 byte = 0u; byte < bytes; ++byte)
        fixture->memory[address+byte] = (lib_u8)(value >> (8u*byte));
}

static lib_u8 v30_control_prepare(v30_memory_fixture *fixture, const v30_control_case *item)
{
    static const lib_u8 opcodes[] = {0x70u,0xe0u,0xebu,0xe9u,0xe8u,0xeau,0x9au,
        0xffu,0xffu,0xffu,0xffu,0xc3u,0xcbu,0xc2u,0xcau};
    static const lib_u8 prefixes[] = {0u,0x26u,0x2eu,0x36u,0x3eu};
    lib_u8 code[8], length = 0u;
    v30_memory_restart(fixture);
    for (lib_u8 reg = 0u; reg < 8u; ++reg)
        *v30_control_word(&fixture->bus.cpu,reg) = 0x12340000u + reg*0x1100u + 0x701u;
    fixture->bus.cpu.data.eip = item->pc;
    fixture->bus.cpu.data.sp = item->sp;
    fixture->bus.cpu.data.cx = item->count;
    fixture->bus.cpu.data.eflags |= item->flags;
    if ((item->form == V30_CONTROL_INDIRECT_JUMP || item->form == V30_CONTROL_INDIRECT_CALL) &&
        (item->modrm >> 6u) == 3u && (item->modrm & 7u) == 4u)
        fixture->bus.cpu.data.sp = item->target;
    if (item->overlap) fixture->bus.cpu.data.bp = item->sp-4u;
    for (lib_i8 word = -3; word <= 3; ++word) {
        const lib_u32 address = 0x20000u+(lib_u16)(fixture->bus.cpu.data.sp+2*word);
        v30_control_store(fixture,address,0xa5a5u,2u);
    }
    if (item->form >= V30_CONTROL_RETURN) {
        v30_control_store(fixture,0x20000u+item->sp,item->target,2u);
        v30_control_store(fixture,0x20000u+(lib_u16)(item->sp+2u),item->segment,2u);
    }
    if (item->prefix) code[length++] = prefixes[item->prefix];
    code[length++] = opcodes[item->form] +
        (item->form <= V30_CONTROL_LOOP ? item->condition : 0u);
    if (item->form <= V30_CONTROL_CALL_WORD) {
        code[length++] = (lib_u8)item->target;
        if (item->form >= V30_CONTROL_JUMP_WORD) code[length++] = (lib_u8)(item->target >> 8u);
    } else if (item->form <= V30_CONTROL_FAR_CALL) {
        code[length++] = (lib_u8)item->target;
        code[length++] = (lib_u8)(item->target >> 8u);
        code[length++] = (lib_u8)item->segment;
        code[length++] = (lib_u8)(item->segment >> 8u);
    } else if (item->form <= V30_CONTROL_INDIRECT_FAR_CALL) {
        const lib_u8 mod = item->modrm >> 6u, rm = item->modrm & 7u;
        code[length++] = item->modrm;
        if (mod == 3u) {
            lib_u32 *word = v30_control_word(&fixture->bus.cpu,rm);
            *word = (*word & 0xffff0000u) | item->target;
        } else {
            if (mod || rm == 6u) {
                code[length++] = mod == 1u ? 0u : 0xffu;
                if (mod != 1u) code[length++] = 0xffu;
            }
            v30_control_store(fixture,v30_control_address(&fixture->bus.cpu,item),
                item->target + 65536u*item->segment,
                item->form >= V30_CONTROL_INDIRECT_FAR_JUMP ? 4u : 2u);
        }
    } else if (item->form >= V30_CONTROL_RETURN_CLEANUP) {
        code[length++] = (lib_u8)item->cleanup;
        code[length++] = (lib_u8)(item->cleanup >> 8u);
    }
    for (lib_u8 byte = 0u; byte < length; ++byte)
        fixture->memory[0x80000u+(lib_u16)(item->pc+byte)] = code[byte];
    return length;
}

/* NEC 12-142..146,154..166, selected by semantic form rather than opcode. */
static lib_i32 v30_control_timing(const v30_memory_fixture *fixture,
    const v30_control_case *item, const t_cpu *before, lib_u8 opcode, lib_bool taken)
{
    const lib_bool indirect=item->form>=V30_CONTROL_INDIRECT_JUMP && item->form<=V30_CONTROL_INDIRECT_FAR_CALL;
    const lib_bool memory=indirect && (item->modrm>>6u)!=3u;
    const lib_bool stack=item->form==V30_CONTROL_CALL_WORD || item->form==V30_CONTROL_FAR_CALL ||
        item->form==V30_CONTROL_INDIRECT_CALL || item->form==V30_CONTROL_INDIRECT_FAR_CALL ||
        item->form>=V30_CONTROL_RETURN;
    const lib_u32 pointer=memory ? v30_control_address(before,item) : 0u;
    const lib_bool mixed=stack && memory && ((pointer ^ before->data.sp) & 1u);
    const lib_bool odd=((stack ? before->data.sp : pointer) & 1u) || (stack && memory && (pointer & 1u));
    lib_u64 clocks=0u;
    switch (item->form) {
    case V30_CONTROL_CONDITIONAL: clocks=taken ? 14u : 4u; break;
    case V30_CONTROL_LOOP: clocks=taken ? (item->condition<2u ? 14u : 13u) : 5u; break;
    case V30_CONTROL_JUMP_BYTE: case V30_CONTROL_JUMP_WORD: clocks=12u; break;
    case V30_CONTROL_FAR_JUMP: clocks=15u; break;
    case V30_CONTROL_CALL_WORD: clocks=odd ? 20u : 16u; break;
    case V30_CONTROL_FAR_CALL: clocks=odd ? 29u : 21u; break;
    case V30_CONTROL_INDIRECT_JUMP: clocks=memory ? (odd ? 24u : 20u) : 11u; break;
    case V30_CONTROL_INDIRECT_FAR_JUMP: clocks=memory ? (odd ? 35u : 27u) : 0u; break;
    case V30_CONTROL_INDIRECT_CALL:
        clocks=memory ? (odd ? 31u : 23u) : odd ? 18u : 14u;
        break;
    case V30_CONTROL_INDIRECT_FAR_CALL:
        if (!memory) break;
        clocks=odd ? 47u : 31u;
        break;
    case V30_CONTROL_RETURN: clocks=odd ? 19u : 15u; break;
    case V30_CONTROL_FAR_RETURN: clocks=odd ? 29u : 21u; break;
    case V30_CONTROL_RETURN_CLEANUP: clocks=odd ? 24u : 20u; break;
    case V30_CONTROL_FAR_RETURN_CLEANUP: clocks=odd ? 32u : 24u; break;
    }
    lib_u32 form=X86_CORE_CPU_RETIREMENT_SOURCE_FORM_UNATTRIBUTED, inputs=0u;
    if (clocks) {
        if (item->prefix) { clocks+=2u; inputs|=X86_CORE_CPU_TIMING_INPUT_SEGMENT_OVERRIDE; }
        form=opcode | (memory ? 0x100u : 0u);
        if (indirect) form|=((lib_u32)(item->modrm>>3u) & 7u)<<9u;
        inputs|=X86_CORE_CPU_TIMING_INPUT_CONTROL | X86_CORE_CPU_TIMING_INPUT_BRANCH |
            (taken ? X86_CORE_CPU_TIMING_INPUT_BRANCH_TAKEN : 0u) |
            (stack ? X86_CORE_CPU_TIMING_INPUT_MEMORY : 0u) |
            (memory ? X86_CORE_CPU_TIMING_INPUT_MODRM | X86_CORE_CPU_TIMING_INPUT_EFFECTIVE_ADDRESS : 0u) |
            (odd ? X86_CORE_CPU_TIMING_INPUT_ODD_WORD : 0u);
    }
    const x86_core_cpu_timing_result timing=x86_core_cpu_capture_timing(&fixture->bus.execution);
    if (timing.ticks!=clocks || !timing.source_timing_unallocated || timing.event_ticks ||
        timing.form_id!=form || timing.key_id!=form || timing.formula_inputs!=inputs ||
        timing.retirement_origin!=(clocks ? (mixed ? X86_CORE_CPU_RETIREMENT_TIMING_ORIGIN_L2_CONTROL_MODEL :
            X86_CORE_CPU_RETIREMENT_TIMING_ORIGIN_V30_NOMINAL_SOURCE) :
            X86_CORE_CPU_RETIREMENT_TIMING_ORIGIN_UNATTRIBUTED)) {
        lib_c_printf("V30:NOMINAL-CONTROL:FAIL form=%u modrm=%02x prefix=%u taken=%u ticks=%llu/%llu inputs=%x/%x\n",
            item->form,item->modrm,item->prefix,taken,timing.ticks,clocks,timing.formula_inputs,inputs);
        return 1;
    }
    return 0;
}

static lib_i32 v30_control_check(const v30_control_case *item, lib_bool deep)
{
    static v30_memory_fixture fixture;
    const lib_u8 length = v30_control_prepare(&fixture,item);
    const t_cpu before = fixture.bus.cpu;
    const lib_u8 opcode=fixture.memory[0x80000u+(lib_u16)(item->pc+(item->prefix != 0u))];
    const lib_u16 next = item->pc+length;
    const lib_bool call = item->form == V30_CONTROL_CALL_WORD || item->form == V30_CONTROL_FAR_CALL ||
        item->form == V30_CONTROL_INDIRECT_CALL || item->form == V30_CONTROL_INDIRECT_FAR_CALL;
    const lib_bool far = item->form == V30_CONTROL_FAR_CALL || item->form == V30_CONTROL_FAR_JUMP ||
        item->form == V30_CONTROL_INDIRECT_FAR_CALL || item->form == V30_CONTROL_INDIRECT_FAR_JUMP ||
        item->form == V30_CONTROL_FAR_RETURN || item->form == V30_CONTROL_FAR_RETURN_CLEANUP;
    const lib_bool returning = item->form >= V30_CONTROL_RETURN;
    t_cpu expected = before;
    v30_memory_plan plan = {0};
    lib_u16 target = item->target, segment = item->segment;
    if (item->form <= V30_CONTROL_JUMP_BYTE) target = next+(lib_i8)item->target;
    else if (item->form <= V30_CONTROL_CALL_WORD) target = next+(lib_i16)item->target;
    lib_bool taken=LIB_TRUE;
    if (item->form == V30_CONTROL_CONDITIONAL) {
        taken=v30_control_condition(item->condition,before.data.eflags);
        if (!taken) target=next;
    }
    if (item->form == V30_CONTROL_LOOP) {
        if (item->condition < 3u) --expected.data.cx;
        const lib_bool zero = (before.data.eflags & VCPU_EFLAGS_ZF) != 0u;
        taken = item->condition == 3u ? before.data.cx == 0u :
            expected.data.cx != 0u && (item->condition == 2u || zero == (item->condition == 1u));
        if (!taken) target = next;
    }
    if (item->form >= V30_CONTROL_INDIRECT_JUMP && item->form <= V30_CONTROL_INDIRECT_FAR_CALL) {
        if ((item->modrm >> 6u) != 3u) {
            const lib_u32 address = v30_control_address(&before,item);
            target = fixture.memory[address] + 256u*fixture.memory[address+1u];
            if (far) segment = fixture.memory[address+2u] + 256u*fixture.memory[address+3u];
            v30_memory_plan_transfer(&plan,&fixture,address,2u,LIB_FALSE,target);
            if (far) v30_memory_plan_transfer(&plan,&fixture,address+2u,2u,LIB_FALSE,segment);
        } else target = (lib_u16)*v30_control_word(&expected,item->modrm & 7u);
    }
    if (returning) {
        const lib_u32 address = 0x20000u+before.data.sp;
        target = fixture.memory[address] + 256u*fixture.memory[address+1u];
        v30_memory_plan_transfer(&plan,&fixture,address,2u,LIB_FALSE,target);
        if (far) {
            const lib_u32 selector = 0x20000u+(lib_u16)(before.data.sp+2u);
            segment = fixture.memory[selector] + 256u*fixture.memory[selector+1u];
            v30_memory_plan_transfer(&plan,&fixture,selector,2u,LIB_FALSE,segment);
        }
        expected.data.sp += (far ? 4u : 2u) +
            (item->form >= V30_CONTROL_RETURN_CLEANUP ? item->cleanup : 0u);
    }
    if (call) {
        if (far) {
            expected.data.sp -= 2u;
            v30_memory_plan_transfer(&plan,&fixture,0x20000u+expected.data.sp,2u,LIB_TRUE,0x8000u);
        }
        expected.data.sp -= 2u;
        v30_memory_plan_transfer(&plan,&fixture,0x20000u+expected.data.sp,2u,LIB_TRUE,next);
    }
    expected.data.eip = target;
    if (far) { expected.data.cs.selector = segment; expected.data.cs.base = 16u*segment; }
    lib_u32 observations = 0u;
    if (deep) {
        x86_core_cpu_instruction_lexeme lexeme;
        if (!x86_core_cpu_execution_preview_lexeme(&fixture.bus.execution,&lexeme) ||
            lexeme.byte_count != length || fixture.reads || fixture.writes || fixture.fetches || fixture.phase_count ||
            lib_memory_compare(&fixture.bus.cpu,&before,sizeof(before))) return 1;
        for (lib_u32 index = 0u; index < fixture.record_count; ++index)
            if (!fixture.records[index].observe ||
                fixture.records[index].provenance == X86_CORE_CPU_MEMORY_ACCESS_DATA) return 1;
        observations = fixture.observations;
        fixture.record_count = fixture.observations = 0u;
    }
    x86_core_cpu_execution_refresh(&fixture.bus.execution);
    if (fixture.bus.execution.stop_requested || fixture.bus.faults || fixture.bus.delivered_exceptions ||
        fixture.observations || v30_control_timing(&fixture,item,&before,opcode,taken) ||
        fixture.bus.instruction_count != 1u || fixture.bus.execution.interrupt_shadow != CPU_INTERRUPT_SHADOW_NONE ||
        lib_memory_compare(&fixture.bus.cpu,&expected,sizeof(expected)) ||
        v30_memory_check_plan(&fixture,&plan,LIB_FALSE)) {
        lib_c_printf("V30:CONTROL-FLOW:FAIL form=%u condition=%u modrm=%02x prefix=%u pc=%04x sp=%04x result=%04x:%04x/%04x:%04x stack=%04x/%04x plan=%u\n",
            item->form,item->condition,item->modrm,item->prefix,item->pc,before.data.sp,
            fixture.bus.cpu.data.cs.selector,fixture.bus.cpu.data.eip,expected.data.cs.selector,
            expected.data.eip,fixture.bus.cpu.data.sp,expected.data.sp,
            v30_memory_check_plan(&fixture,&plan,LIB_FALSE));
        return 1;
    }
    const lib_u32 reads = fixture.reads, writes = fixture.writes, fetches = fixture.fetches;
    if (deep && call) {
        fixture.memory[(expected.data.cs.base+target) & 0xfffffu] = far ? 0xcbu : 0xc3u;
        x86_core_cpu_execution_invalidate_prefetch(&fixture.bus.execution);
        x86_core_cpu_execution_refresh(&fixture.bus.execution);
        const v30_control_case returning_item={.form=far ? V30_CONTROL_FAR_RETURN : V30_CONTROL_RETURN};
        if (v30_control_timing(&fixture,&returning_item,&expected,far ? 0xcbu : 0xc3u,LIB_TRUE)) return 1;
        t_cpu returned = before;
        returned.data.eip = next;
        if (fixture.bus.execution.stop_requested || lib_memory_compare(&fixture.bus.cpu,&returned,sizeof(returned))) return 1;
    }
    if (deep) for (lib_u8 kind = 0u; kind < 4u; ++kind) {
        const lib_u32 counts[] = {reads,writes,fetches,observations};
        for (lib_u32 part = 1u; part <= counts[kind]; ++part) {
            v30_control_prepare(&fixture,item);
            if (kind == 0u) fixture.fail_read = part;
            if (kind == 1u) fixture.fail_write = part;
            if (kind == 2u) fixture.fail_fetch = part;
            if (kind == 3u) {
                x86_core_cpu_instruction_lexeme lexeme;
                fixture.fail_observe = part;
                if (x86_core_cpu_execution_preview_lexeme(&fixture.bus.execution,&lexeme) ||
                    fixture.bus.execution.stop_requested || fixture.bus.faults || fixture.fetches) return 1;
            } else {
                x86_core_cpu_execution_refresh(&fixture.bus.execution);
                if (!fixture.bus.execution.stop_requested || !fixture.bus.faults ||
                    fixture.bus.execution.completed_observation_valid || fixture.bus.execution.timing_result.ticks ||
                    fixture.bus.execution.timing_result.retirement_origin!=X86_CORE_CPU_RETIREMENT_TIMING_ORIGIN_UNATTRIBUTED)
                    return 1;
            }
            if (fixture.bus.delivered_exceptions ||
                fixture.bus.execution.interrupt_shadow != CPU_INTERRUPT_SHADOW_NONE ||
                lib_memory_compare(&fixture.bus.cpu,&before,sizeof(before)) ||
                (kind < 2u ? v30_memory_check_plan(&fixture,&plan,LIB_TRUE) :
                    fixture.reads || fixture.writes || fixture.phase_count)) {
                lib_c_printf("V30:CONTROL-FLOW:REFUSAL form=%u modrm=%02x prefix=%u pc=%04x sp=%04x kind=%u part=%u\n",
                    item->form,item->modrm,item->prefix,item->pc,before.data.sp,kind,part);
                return 1;
            }
        }
    }
    return 0;
}

lib_i32 main(void)
{
    static const lib_u16 pcs[] = {0x100u,1u,0xfffcu,0xffffu};
    static const lib_u16 stacks[] = {0x700u,0x701u,0u,1u,0xffffu};
    static const lib_u16 values[] = {0u,1u,127u,128u,0xffffu};
    static const lib_u16 counts[] = {0u,1u,2u,0xffffu};
    static const lib_u16 segments[] = {0u,0x8000u,0x9000u,0xffffu};
    lib_u32 cases = 0u;
    for (lib_u8 form = V30_CONTROL_CONDITIONAL; form <= V30_CONTROL_CALL_WORD; ++form)
        for (lib_u8 condition = 0u; condition < (form == V30_CONTROL_CONDITIONAL ? 16u :
            form == V30_CONTROL_LOOP ? 4u : 1u); ++condition)
            for (lib_u8 flags = 0u; flags < (form <= V30_CONTROL_LOOP ? 32u : 1u); ++flags)
                for (lib_u8 count = 0u; count < (form == V30_CONTROL_LOOP ? 4u : 1u); ++count)
                    for (lib_u32 value = 0u; value < (form >= V30_CONTROL_JUMP_WORD ? 65536u : 256u); ++value) {
                        const v30_control_case item = {.form=(v30_control_form)form,.condition=condition,
                            .pc=0x100u,.sp=0x701u,.count=counts[count],.target=(lib_u16)value,.flags=v30_control_flags(flags)};
                        if (v30_control_check(&item,LIB_FALSE)) return 1;
                        ++cases;
                    }
    for (lib_u8 form = 0u; form <= V30_CONTROL_FAR_RETURN_CLEANUP; ++form)
        for (lib_u8 prefix = 0u; prefix < 5u; ++prefix)
            for (lib_u8 pc = 0u; pc < sizeof(pcs)/sizeof(pcs[0]); ++pc)
                for (lib_u8 stack = 0u; stack < sizeof(stacks)/sizeof(stacks[0]); ++stack)
                    for (lib_u8 value = 0u; value < sizeof(values)/sizeof(values[0]); ++value) {
                        const lib_bool indirect = form >= V30_CONTROL_INDIRECT_JUMP && form <= V30_CONTROL_INDIRECT_FAR_CALL;
                        for (lib_u16 modrm = 0u; modrm < (indirect ? 256u : 1u); ++modrm) {
                            const lib_u8 extension = form == V30_CONTROL_INDIRECT_JUMP ? 4u :
                                form == V30_CONTROL_INDIRECT_CALL ? 2u : form == V30_CONTROL_INDIRECT_FAR_JUMP ? 5u : 3u;
                            if (indirect && (((modrm >> 3u) & 7u) != extension ||
                                (form >= V30_CONTROL_INDIRECT_FAR_JUMP && (modrm >> 6u) == 3u))) continue;
                            for (lib_u8 condition = 0u; condition < (form == V30_CONTROL_CONDITIONAL ? 16u :
                                form == V30_CONTROL_LOOP ? 4u : 1u); ++condition) {
                                const v30_control_case item = {.form=(v30_control_form)form,.prefix=prefix,.condition=condition,
                                    .pc=pcs[pc],.sp=stacks[stack],.count=counts[value % 4u],.target=values[value],
                                    .segment=segments[value % 4u],.cleanup=values[value],.modrm=(lib_u8)modrm,
                                    .flags=v30_control_flags((lib_u8)(value*7u))};
                                if (v30_control_check(&item,LIB_TRUE)) return 1;
                                ++cases;
                            }
                        }
                    }
    /* Pointer bytes may be overwritten by CALL only after the full read. */
    for (lib_u8 far = 0u; far < 2u; ++far)
        for (lib_u8 stack = 0u; stack < sizeof(stacks)/sizeof(stacks[0]); ++stack) {
            const v30_control_case item = {.form=far ? V30_CONTROL_INDIRECT_FAR_CALL : V30_CONTROL_INDIRECT_CALL,
                .pc=0x100u,.sp=stacks[stack],.target=0x3456u,.segment=0x9000u,
                .modrm=far ? 0x5eu : 0x56u,.overlap=LIB_TRUE,.flags=v30_control_flags(31u)};
            if (v30_control_check(&item,LIB_TRUE)) return 1;
            ++cases;
        }
    lib_c_printf("V30:CONTROL-FLOW:OK cases=%u\n",cases);
    lib_c_printf("V30:NOMINAL-CONTROL:OK cases=%u\n",cases);
    lib_c_printf("V30:CONTROL-ALIGNMENT-MODEL:OK\n");
    return 0;
}
