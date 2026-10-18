#include "support/cpu_v30_memory_fixture.h"
#include "lib/types/file.h"

typedef enum v30_flags_operation {
    V30_FLAGS_PUSH, V30_FLAGS_POP, V30_FLAGS_FROM_AH, V30_FLAGS_TO_AH,
    V30_FLAGS_CLEAR_CARRY, V30_FLAGS_SET_CARRY, V30_FLAGS_COMPLEMENT_CARRY,
    V30_FLAGS_CLEAR_DIRECTION, V30_FLAGS_SET_DIRECTION,
    V30_FLAGS_DISABLE_INTERRUPT, V30_FLAGS_ENABLE_INTERRUPT
} v30_flags_operation;

typedef struct v30_flags_case {
    v30_flags_operation operation;
    lib_u16 value, flags, pc, sp;
    lib_u8 prefix;
} v30_flags_case;

static lib_u16 v30_flags_bits(lib_u16 bits)
{
    static const lib_u16 masks[] = {VCPU_EFLAGS_CF,VCPU_EFLAGS_PF,VCPU_EFLAGS_AF,
        VCPU_EFLAGS_ZF,VCPU_EFLAGS_SF,VCPU_EFLAGS_IF,VCPU_EFLAGS_DF,VCPU_EFLAGS_OF};
    lib_u16 flags = 0u;
    for (lib_u8 bit = 0u; bit < sizeof(masks)/sizeof(masks[0]); ++bit)
        if (bits & (1u << bit)) flags |= masks[bit];
    return flags;
}

static lib_u8 v30_flags_prepare(v30_memory_fixture *fixture, const v30_flags_case *item)
{
    static const lib_u8 opcodes[] = {0x9cu,0x9du,0x9eu,0x9fu,0xf8u,0xf9u,
        0xf5u,0xfcu,0xfdu,0xfau,0xfbu};
    static const lib_u8 prefixes[] = {0u,0x26u,0x2eu,0x36u,0x3eu};
    v30_memory_restart(fixture);
    fixture->bus.cpu.data.eax = 0x123400abu | ((lib_u32)(item->value & 0xffu) << 8u);
    fixture->bus.cpu.data.ecx = 0x23451234u;
    fixture->bus.cpu.data.edx = 0x34562345u;
    fixture->bus.cpu.data.ebx = 0x45673456u;
    fixture->bus.cpu.data.ebp = 0x56784567u;
    fixture->bus.cpu.data.esi = 0x67895678u;
    fixture->bus.cpu.data.edi = 0x789a6789u;
    fixture->bus.cpu.data.esp = 0x89ab0000u | item->sp;
    fixture->bus.cpu.data.eip = item->pc;
    fixture->bus.cpu.data.eflags = 0x8002u | item->flags;
    for (lib_i8 word = -1; word <= 1; ++word) {
        const lib_u32 address = 0x20000u+(lib_u16)(item->sp+2*word);
        fixture->memory[address] = 0xa5u;
        fixture->memory[address+1u] = 0x5au;
    }
    if (item->operation == V30_FLAGS_POP) {
        fixture->memory[0x20000u+item->sp] = (lib_u8)item->value;
        fixture->memory[0x20001u+item->sp] = (lib_u8)(item->value >> 8u);
    }
    for (lib_u8 byte = 0u; byte < 8u; ++byte)
        fixture->memory[0x80000u+(lib_u16)(item->pc+byte)] = 0x90u;
    if (item->prefix) fixture->memory[0x80000u+item->pc] = prefixes[item->prefix];
    fixture->memory[0x80000u+(lib_u16)(item->pc+(item->prefix != 0u))] = opcodes[item->operation];
    return item->prefix ? 2u : 1u;
}

static lib_i32 v30_flags_check(const v30_flags_case *item, lib_bool deep)
{
    static v30_memory_fixture fixture;
    const lib_u8 length = v30_flags_prepare(&fixture,item);
    const t_cpu before = fixture.bus.cpu;
    t_cpu expected = before;
    v30_memory_plan plan = {0};
    switch (item->operation) {
    case V30_FLAGS_PUSH:
        expected.data.sp -= 2u;
        v30_memory_plan_transfer(&plan,&fixture,0x20000u+expected.data.sp,2u,LIB_TRUE,
            (lib_u16)(before.data.eflags | 0x7000u));
        break;
    case V30_FLAGS_POP:
        expected.data.eflags = 0x8002u | (item->value & 0x0fd5u);
        expected.data.sp += 2u;
        v30_memory_plan_transfer(&plan,&fixture,0x20000u+before.data.sp,2u,LIB_FALSE,item->value);
        break;
    case V30_FLAGS_FROM_AH:
        expected.data.eflags = (before.data.eflags & ~0xd5u) | (before.data.ah & 0xd5u);
        break;
    case V30_FLAGS_TO_AH: expected.data.ah = before.data.eflags & 0xd5u; break;
    case V30_FLAGS_CLEAR_CARRY: expected.data.eflags &= ~VCPU_EFLAGS_CF; break;
    case V30_FLAGS_SET_CARRY: expected.data.eflags |= VCPU_EFLAGS_CF; break;
    case V30_FLAGS_COMPLEMENT_CARRY: expected.data.eflags ^= VCPU_EFLAGS_CF; break;
    case V30_FLAGS_CLEAR_DIRECTION: expected.data.eflags &= ~VCPU_EFLAGS_DF; break;
    case V30_FLAGS_SET_DIRECTION: expected.data.eflags |= VCPU_EFLAGS_DF; break;
    case V30_FLAGS_DISABLE_INTERRUPT: expected.data.eflags &= ~VCPU_EFLAGS_IF; break;
    case V30_FLAGS_ENABLE_INTERRUPT: expected.data.eflags |= VCPU_EFLAGS_IF; break;
    }
    expected.data.eip = (lib_u16)(item->pc+length);
    lib_u32 observations = 0u;
    if (deep) {
        x86_core_cpu_instruction_lexeme lexeme;
        if (!x86_core_cpu_execution_preview_lexeme(&fixture.bus.execution,&lexeme) ||
            lexeme.byte_count != length || fixture.reads || fixture.writes || fixture.fetches || fixture.phase_count ||
            fixture.bus.instruction_count || fixture.bus.faults || fixture.bus.delivered_exceptions ||
            lib_memory_compare(&fixture.bus.cpu,&before,sizeof(before))) return 1;
        for (lib_u32 index = 0u; index < fixture.record_count; ++index)
            if (!fixture.records[index].observe ||
                fixture.records[index].provenance == X86_CORE_CPU_MEMORY_ACCESS_DATA) return 1;
        observations = fixture.observations;
        fixture.record_count = fixture.observations = 0u;
    }
    x86_core_cpu_execution_refresh(&fixture.bus.execution);
    const lib_bool stack=item->operation<=V30_FLAGS_POP;
    const lib_u64 clocks = item->prefix ? 0u : stack ? 8u+4u*(item->sp & 1u) :
        item->operation == V30_FLAGS_FROM_AH ? 3u : 2u;
    const lib_u32 form = clocks ? fixture.memory[0x80000u+item->pc] :
        X86_CORE_CPU_RETIREMENT_SOURCE_FORM_UNATTRIBUTED;
    const x86_core_cpu_timing_result timing = x86_core_cpu_capture_timing(&fixture.bus.execution);
    if (timing.ticks != clocks || !timing.source_timing_unallocated || timing.event_ticks ||
        timing.formula_inputs != (clocks && stack ? X86_CORE_CPU_TIMING_INPUT_MEMORY |
            ((item->sp & 1u) ? X86_CORE_CPU_TIMING_INPUT_ODD_WORD : 0u) : 0u) ||
        timing.form_id != form || timing.key_id != form ||
        timing.retirement_origin != (clocks ? X86_CORE_CPU_RETIREMENT_TIMING_ORIGIN_V30_NOMINAL_SOURCE :
            X86_CORE_CPU_RETIREMENT_TIMING_ORIGIN_UNATTRIBUTED)) return 1;
    t_cpu actual = fixture.bus.cpu;
    /* NEC leaves AH bits 5,3,1 undefined for MOV AH,PSW. */
    if (item->operation == V30_FLAGS_TO_AH) actual.data.ah &= 0xd5u;
    /* Carry controls leave V,S,Z,AC,P undefined (NEC Table 12-4). */
    if (item->operation >= V30_FLAGS_CLEAR_CARRY && item->operation <= V30_FLAGS_COMPLEMENT_CARRY) {
        actual.data.eflags &= ~0x08d4u;
        expected.data.eflags &= ~0x08d4u;
    }
    if (fixture.bus.execution.stop_requested || fixture.bus.faults || fixture.bus.delivered_exceptions ||
        fixture.bus.instruction_count != 1u || fixture.bus.execution.interrupt_shadow !=
            (item->operation == V30_FLAGS_ENABLE_INTERRUPT ? CPU_INTERRUPT_SHADOW_INTR : CPU_INTERRUPT_SHADOW_NONE) ||
        lib_memory_compare(&actual,&expected,sizeof(expected)) || v30_memory_check_plan(&fixture,&plan,LIB_FALSE)) {
        lib_c_printf("V30:FLAGS:FAIL op=%u raw=%04x flags=%04x prefix=%u pc=%04x sp=%04x actual=%08x/%08x ah=%02x/%02x plan=%u\n",
            item->operation,item->value,item->flags,item->prefix,item->pc,item->sp,
            actual.data.eflags,expected.data.eflags,actual.data.ah,expected.data.ah,
            v30_memory_check_plan(&fixture,&plan,LIB_FALSE));
        return 1;
    }
    const lib_u32 reads = fixture.reads, writes = fixture.writes, fetches = fixture.fetches;
    if (deep && item->operation == V30_FLAGS_PUSH) {
        fixture.memory[0x80000u+expected.data.eip] = 0x9du;
        x86_core_cpu_execution_invalidate_prefetch(&fixture.bus.execution);
        x86_core_cpu_execution_refresh(&fixture.bus.execution);
        t_cpu returned = before;
        returned.data.eip = (lib_u16)(expected.data.eip+1u);
        if (fixture.bus.execution.stop_requested || lib_memory_compare(&fixture.bus.cpu,&returned,sizeof(returned))) return 1;
    }
    if (deep) for (lib_u8 kind = 0u; kind < 4u; ++kind) {
        const lib_u32 counts[] = {reads,writes,fetches,observations};
        for (lib_u32 part = 1u; part <= counts[kind]; ++part) {
            v30_flags_prepare(&fixture,item);
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
                    fixture.bus.execution.completed_observation_valid ||
                    fixture.bus.execution.timing_result.ticks ||
                    fixture.bus.execution.timing_result.retirement_origin != X86_CORE_CPU_RETIREMENT_TIMING_ORIGIN_UNATTRIBUTED)
                    return 1;
            }
            if (fixture.bus.delivered_exceptions || fixture.bus.execution.interrupt_shadow != CPU_INTERRUPT_SHADOW_NONE ||
                lib_memory_compare(&fixture.bus.cpu,&before,sizeof(before)) ||
                (kind < 2u ? v30_memory_check_plan(&fixture,&plan,LIB_TRUE) :
                    fixture.reads || fixture.writes || fixture.phase_count)) {
                lib_c_printf("V30:FLAGS:REFUSAL op=%u prefix=%u pc=%04x sp=%04x kind=%u part=%u\n",
                    item->operation,item->prefix,item->pc,item->sp,kind,part);
                return 1;
            }
        }
    }
    return 0;
}

static lib_i32 v30_flags_debug_entry(lib_u32 *cases)
{
    static v30_memory_fixture fixture;
    for (lib_u32 value = 0u; value < 65536u; ++value)
        for (lib_u8 high = 0u; high < 2u; ++high) {
            v30_memory_restart(&fixture);
            const t_cpu before = fixture.bus.cpu;
            t_cpu expected = before;
            const x86_core_cpu_debug_register_patch patch = {
                .mask=X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_EFLAGS),
                .values={[X86_CORE_CPU_DEBUG_EFLAGS]=value | (high ? 0xffff0000u : 0u)}};
            expected.data.eflags = 0x8002u | (value & 0x0fd5u);
            lib_u32 flags = 0u;
            if (x86_core_cpu_debug_patch_registers(&fixture.bus.execution,&patch) != LIB_STATUS_OK ||
                x86_core_cpu_debug_read_register(&fixture.bus.execution,X86_CORE_CPU_DEBUG_EFLAGS,&flags) != LIB_STATUS_OK ||
                flags != expected.data.eflags || lib_memory_compare(&fixture.bus.cpu,&expected,sizeof(expected)) ||
                fixture.record_count || fixture.bus.instruction_count) return 1;
            ++*cases;
        }
    for (lib_u16 value = 0u; value < 512u; ++value) {
        v30_memory_restart(&fixture);
        const t_cpu before = fixture.bus.cpu;
        const x86_core_cpu_entry_plan_state plan = {.cs=0x1234u,.ds=0x2345u,.es=0x3456u,.ss=0x4567u,
            .ip=0x6789u,.sp=0x789au,.eflags=v30_flags_bits(value & 255u) | (value & 256u ? VCPU_EFLAGS_TF : 0u)};
        x86_core_cpu_prepared_entry *entry = LIB_NULL;
        if (x86_core_cpu_prepare_entry(&fixture.bus.execution,&plan,&entry) != LIB_STATUS_OK || entry == LIB_NULL ||
            lib_memory_compare(&fixture.bus.cpu,&before,sizeof(before))) return 1;
        x86_core_cpu_finish_entry(entry,LIB_FALSE);
        if (lib_memory_compare(&fixture.bus.cpu,&before,sizeof(before)) ||
            x86_core_cpu_prepare_entry(&fixture.bus.execution,&plan,&entry) != LIB_STATUS_OK) return 1;
        x86_core_cpu_finish_entry(entry,LIB_TRUE);
        x86_core_cpu_state state;
        x86_core_cpu_capture_state(&fixture.bus.execution,&state);
        if (state.eflags != (0x8002u | plan.eflags) || state.cs != plan.cs || state.eip != plan.ip || fixture.bus.cpu.data.sp != plan.sp ||
            fixture.bus.cpu.data.ds.selector != plan.ds || fixture.bus.cpu.data.es.selector != plan.es ||
            fixture.bus.cpu.data.ss.selector != plan.ss || fixture.record_count || fixture.bus.instruction_count) return 1;
        ++*cases;
    }
    for (lib_u8 bit = 0u; bit < 32u; ++bit) {
        if ((1u << bit) & 0x0fd5u) continue;
        v30_memory_restart(&fixture);
        const t_cpu before = fixture.bus.cpu;
        const x86_core_cpu_entry_plan_state plan = {.eflags=1u << bit};
        x86_core_cpu_prepared_entry *entry = LIB_NULL;
        if (x86_core_cpu_prepare_entry(&fixture.bus.execution,&plan,&entry) != LIB_STATUS_INVALID_ARGUMENT ||
            entry != LIB_NULL || lib_memory_compare(&fixture.bus.cpu,&before,sizeof(before))) return 1;
        ++*cases;
    }
    v30_memory_restart(&fixture);
    const t_cpu before = fixture.bus.cpu;
    const x86_core_cpu_debug_register_patch patch = {
        .mask=X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_EAX) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_EFLAGS) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_CR0),
        .values={[X86_CORE_CPU_DEBUG_EAX]=0xabcdef12u,[X86_CORE_CPU_DEBUG_EFLAGS]=0xffffu,
            [X86_CORE_CPU_DEBUG_CR0]=VCPU_CR0_PE}};
    if (x86_core_cpu_debug_patch_registers(&fixture.bus.execution,&patch) != LIB_STATUS_UNSUPPORTED ||
        lib_memory_compare(&fixture.bus.cpu,&before,sizeof(before))) return 1;
    ++*cases;
    return 0;
}

lib_i32 main(void)
{
    static const lib_u16 pcs[] = {0x100u,1u,0xfffeu,0xffffu};
    static const lib_u16 stacks[] = {0x700u,0x701u,0u,1u,0xffffu};
    static const lib_u16 values[] = {0u,0x0100u,0x7fffu,0x8000u,0xffffu};
    lib_u32 cases = 0u;
    for (lib_u8 operation = V30_FLAGS_PUSH; operation <= V30_FLAGS_ENABLE_INTERRUPT; ++operation)
        for (lib_u16 flags = 0u; flags < (operation == V30_FLAGS_POP ? 2u : 256u); ++flags)
            for (lib_u32 value = 0u; value < (operation == V30_FLAGS_POP ? 65536u :
                operation == V30_FLAGS_FROM_AH || operation == V30_FLAGS_TO_AH ? 256u : 1u); ++value) {
                const v30_flags_case item = {.operation=(v30_flags_operation)operation,.value=(lib_u16)value,
                    .flags=v30_flags_bits(operation == V30_FLAGS_POP ? 255u*flags : flags),.pc=0x100u,.sp=0x701u};
                if (v30_flags_check(&item,LIB_FALSE)) return 1;
                ++cases;
            }
    for (lib_u8 operation = 0u; operation <= V30_FLAGS_ENABLE_INTERRUPT; ++operation)
        for (lib_u8 prefix = 0u; prefix < 5u; ++prefix)
            for (lib_u8 pc = 0u; pc < sizeof(pcs)/sizeof(pcs[0]); ++pc)
                for (lib_u8 stack = 0u; stack < sizeof(stacks)/sizeof(stacks[0]); ++stack)
                    for (lib_u8 value = 0u; value < sizeof(values)/sizeof(values[0]); ++value)
                        for (lib_u8 flags = 0u; flags < 2u; ++flags) {
                            const v30_flags_case item = {.operation=(v30_flags_operation)operation,
                                .value=values[value],.flags=v30_flags_bits(255u*flags),.prefix=prefix,
                                .pc=pcs[pc],.sp=stacks[stack]};
                            if (v30_flags_check(&item,LIB_TRUE)) return 1;
                            ++cases;
                        }
    if (v30_flags_debug_entry(&cases)) { lib_c_printf("V30:FLAGS:DEBUG-ENTRY:FAIL\n"); return 1; }
    lib_c_printf("V30:FLAGS:OK cases=%u\n",cases);
    return 0;
}
