#include "support/cpu_v30_memory_fixture.h"
#include "lib/types/file.h"

typedef enum v30_mode_operation {
    V30_MODE_BRKEM, V30_MODE_CALLN, V30_MODE_RETEM,
    V30_MODE_RETI, V30_MODE_POP_PSW, V30_MODE_NOP
} v30_mode_operation;

typedef struct v30_mode_case {
    v30_mode_operation operation;
    lib_u16 pc, sp, flags, restored, target, segment;
    lib_u8 vector, alias;
} v30_mode_case;

static void v30_mode_store(v30_memory_fixture *fixture, lib_u32 address, lib_u16 value)
{
    fixture->memory[address] = (lib_u8)value;
    fixture->memory[address+1u] = (lib_u8)(value >> 8u);
}

static void v30_mode_code(v30_memory_fixture *fixture, lib_u32 base, lib_u16 pc,
    const lib_u8 *code, lib_u8 length)
{
    for (lib_u8 byte = 0u; byte < 16u; ++byte)
        fixture->memory[base+(lib_u16)(pc+byte)] = 0x00u;
    for (lib_u8 byte = 0u; byte < length; ++byte)
        fixture->memory[base+(lib_u16)(pc+byte)] = code[byte];
}

static void v30_mode_clear_probes(v30_memory_fixture *fixture)
{
    lib_memory_set(&fixture->record_count,0,
        sizeof(*fixture)-lib_offsetof(v30_memory_fixture,record_count));
    fixture->bus.instruction_count = fixture->bus.faults = fixture->bus.delivered_exceptions = 0u;
    fixture->bus.execution.prefetch_valid = LIB_FALSE;
    fixture->bus.execution.prefetch_expected_valid = LIB_FALSE;
    fixture->bus.execution.prefetch_reservation_valid = LIB_FALSE;
}

static lib_u16 v30_mode_planned_word(const v30_memory_fixture *fixture,
    const v30_memory_plan *plan, lib_u32 address)
{
    lib_u16 value = 0u;
    for (lib_u8 byte = 0u; byte < 2u; ++byte) {
        lib_u8 part = fixture->memory[address+byte];
        for (lib_u8 index = 0u; index < plan->count; ++index) {
            const v30_memory_expected_transfer *transfer = &plan->transfers[index];
            if (transfer->write && address+byte >= transfer->address &&
                address+byte-transfer->address < transfer->bytes)
                part = (lib_u8)(transfer->value >> (8u*(address+byte-transfer->address)));
        }
        value |= (lib_u16)part << (8u*byte);
    }
    return value;
}

/* Reach write-enabled states by actual BRKEM/CALLN, not by setting the latch. */
static lib_i32 v30_mode_prepare(v30_memory_fixture *fixture, const v30_mode_case *item)
{
    static const lib_u8 brkem[] = {0x0fu,0xffu,0x80u};
    static const lib_u8 calln[] = {0xedu,0xedu,0x81u};
    v30_memory_restart(fixture);
    if (item->operation != V30_MODE_BRKEM) {
        v30_mode_code(fixture,0x80000u,0x100u,brkem,sizeof(brkem));
        v30_mode_store(fixture,0x200u,0x200u);
        v30_mode_store(fixture,0x202u,0x9000u);
        x86_core_cpu_execution_refresh(&fixture->bus.execution);
        if (fixture->bus.execution.stop_requested || !fixture->bus.execution.nec_md_write_enabled ||
            (fixture->bus.cpu.data.eflags & X86_CPU_NEC_PSW_MD)) return 1;
        if (item->operation == V30_MODE_RETI || item->operation == V30_MODE_POP_PSW) {
            v30_mode_code(fixture,0x90000u,0x200u,calln,sizeof(calln));
            v30_mode_store(fixture,0x204u,0x300u);
            v30_mode_store(fixture,0x206u,0x8000u);
            x86_core_cpu_execution_refresh(&fixture->bus.execution);
            if (fixture->bus.execution.stop_requested ||
                !(fixture->bus.cpu.data.eflags & X86_CPU_NEC_PSW_MD)) return 1;
        }
    }
    fixture->bus.cpu.data.cs.selector = 0x8000u;
    fixture->bus.cpu.data.cs.base = 0x80000u;
    fixture->bus.cpu.data.eip = item->pc;
    fixture->bus.cpu.data.esp = 0x12340000u | item->sp;
    fixture->bus.cpu.data.ebp = 0x23456789u;
    fixture->bus.cpu.data.eax = 0x34569a78u;
    fixture->bus.cpu.data.eflags = 2u | item->flags |
        (item->operation == V30_MODE_BRKEM || item->operation == V30_MODE_RETI ||
         item->operation == V30_MODE_POP_PSW ? X86_CPU_NEC_PSW_MD : 0u);
    const lib_u32 stack_base = item->alias == 1u ? 0x80000u : item->alias == 2u ? 0u : 0x20000u;
    fixture->bus.cpu.data.ss.base = stack_base;
    fixture->bus.cpu.data.ss.selector = (lib_u16)(stack_base >> 4u);
    for (lib_i8 word = -3; word <= 3; ++word)
        v30_mode_store(fixture,stack_base+(lib_u16)(item->sp+2*word),0xa55au);
    const lib_u16 words[] = {item->target,item->segment,item->restored};
    if (item->operation >= V30_MODE_RETEM && item->operation <= V30_MODE_POP_PSW) {
        for (lib_u8 word = 0u; word < 3u; ++word)
            v30_mode_store(fixture,stack_base+(lib_u16)(item->sp+2u*word),words[word]);
        if (item->operation == V30_MODE_POP_PSW)
            v30_mode_store(fixture,stack_base+item->sp,item->restored);
    }
    v30_mode_store(fixture,4u*item->vector,0x200u);
    v30_mode_store(fixture,4u*item->vector+2u,0x9000u);
    const lib_u8 code[][3] = {{0x0fu,0xffu,item->vector},{0xedu,0xedu,item->vector},
        {0xedu,0xfdu},{0xcfu},{0x9du},{0x00u}};
    const lib_u8 lengths[] = {3u,3u,2u,1u,1u,1u};
    v30_mode_code(fixture,0x80000u,item->pc,code[item->operation],lengths[item->operation]);
    v30_mode_clear_probes(fixture);
    return 0;
}

static lib_i32 v30_mode_check(const v30_mode_case *item, lib_bool deep)
{
    static v30_memory_fixture fixture;
    if (v30_mode_prepare(&fixture,item)) return 1;
    const t_cpu before = fixture.bus.cpu;
    const lib_bool latch_before = fixture.bus.execution.nec_md_write_enabled;
    t_cpu expected = before;
    v30_memory_plan plan = {0};
    const lib_bool call = item->operation <= V30_MODE_CALLN;
    const lib_bool returning = item->operation == V30_MODE_RETEM || item->operation == V30_MODE_RETI;
    const lib_u8 length = call ? 3u : item->operation == V30_MODE_RETEM ? 2u : 1u;
    const lib_u16 next = (lib_u16)(item->pc+length);
    if (call) {
        const lib_u16 words[] = {(lib_u16)(before.data.flags | 0x7000u),0x8000u,next};
        for (lib_u8 word = 0u; word < 3u; ++word) {
            expected.data.sp -= 2u;
            v30_memory_plan_transfer(&plan,&fixture,before.data.ss.base+expected.data.sp,2u,LIB_TRUE,words[word]);
        }
        const lib_u16 target = v30_mode_planned_word(&fixture,&plan,4u*item->vector);
        const lib_u16 segment = v30_mode_planned_word(&fixture,&plan,4u*item->vector+2u);
        v30_memory_plan_transfer(&plan,&fixture,4u*item->vector,2u,LIB_FALSE,target);
        v30_memory_plan_transfer(&plan,&fixture,4u*item->vector+2u,2u,LIB_FALSE,segment);
        expected.data.cs.selector = segment;
        expected.data.cs.base = 16u*segment;
        expected.data.eip = target;
        if (item->operation == V30_MODE_BRKEM) expected.data.flags &= ~X86_CPU_NEC_PSW_MD;
        else expected.data.flags |= X86_CPU_NEC_PSW_MD;
    } else if (returning) {
        lib_u16 words[3];
        for (lib_u8 word = 0u; word < 3u; ++word)
        {
            const lib_u32 address = before.data.ss.base+(lib_u16)(item->sp+2u*word);
            words[word] = v30_mode_planned_word(&fixture,&plan,address);
            v30_memory_plan_transfer(&plan,&fixture,address,2u,LIB_FALSE,words[word]);
        }
        expected.data.sp += 6u;
        expected.data.cs.selector = words[1];
        expected.data.cs.base = 16u*words[1];
        expected.data.eip = words[0];
        expected.data.eflags = 2u | (words[2] & 0x8fd5u);
    } else {
        expected.data.eip = next;
        if (item->operation == V30_MODE_POP_PSW) {
            const lib_u32 address = before.data.ss.base+item->sp;
            const lib_u16 flags = v30_mode_planned_word(&fixture,&plan,address);
            v30_memory_plan_transfer(&plan,&fixture,address,2u,LIB_FALSE,flags);
            expected.data.sp += 2u;
            expected.data.eflags = 2u | (flags & 0x8fd5u);
        }
    }
    lib_u32 observations = 0u;
    if (deep) {
        x86_core_cpu_instruction_lexeme lexeme;
        if (!x86_core_cpu_execution_preview_lexeme(&fixture.bus.execution,&lexeme) ||
            lexeme.byte_count != length || fixture.reads || fixture.writes || fixture.fetches ||
            fixture.phase_count || fixture.bus.faults || fixture.bus.instruction_count ||
            fixture.bus.execution.nec_md_write_enabled != latch_before ||
            lib_memory_compare(&fixture.bus.cpu,&before,sizeof(before))) return 1;
        observations = fixture.observations;
        fixture.record_count = fixture.observations = 0u;
    }
    x86_core_cpu_execution_refresh(&fixture.bus.execution);
    const lib_bool latch_after = item->operation != V30_MODE_RETEM;
    if (fixture.bus.execution.stop_requested || fixture.bus.faults || fixture.bus.delivered_exceptions ||
        fixture.bus.instruction_count != 1u || fixture.bus.execution.nec_md_write_enabled != latch_after ||
        lib_memory_compare(&fixture.bus.cpu,&expected,sizeof(expected)) ||
        v30_memory_check_plan(&fixture,&plan,LIB_FALSE)) {
        lib_c_printf("V30:MODE-TRANSITION:RESULT op=%u pc=%04x sp=%04x flags=%04x restored=%04x got=%04x/%04x expected=%04x/%04x latch=%u\n",
            item->operation,item->pc,item->sp,item->flags,item->restored,fixture.bus.cpu.data.eip,
            fixture.bus.cpu.data.flags,expected.data.eip,expected.data.flags,fixture.bus.execution.nec_md_write_enabled);
        return 1;
    }
    const lib_u32 counts[] = {fixture.reads,fixture.writes,fixture.fetches,observations};
    if (deep) for (lib_u8 kind = 0u; kind < 4u; ++kind)
        for (lib_u32 part = 1u; part <= counts[kind]; ++part) {
            if (v30_mode_prepare(&fixture,item)) return 1;
            if (kind == 0u) fixture.fail_read = part;
            if (kind == 1u) fixture.fail_write = part;
            if (kind == 2u) fixture.fail_fetch = part;
            if (kind == 3u) {
                x86_core_cpu_instruction_lexeme lexeme;
                fixture.fail_observe = part;
                if (x86_core_cpu_execution_preview_lexeme(&fixture.bus.execution,&lexeme) ||
                    fixture.bus.execution.stop_requested || fixture.bus.faults) return 1;
            } else {
                x86_core_cpu_execution_refresh(&fixture.bus.execution);
                if (!fixture.bus.execution.stop_requested || !fixture.bus.faults) return 1;
            }
            if (fixture.bus.delivered_exceptions || fixture.bus.execution.nec_md_write_enabled != latch_before ||
                lib_memory_compare(&fixture.bus.cpu,&before,sizeof(before)) ||
                (kind < 2u ? v30_memory_check_plan(&fixture,&plan,LIB_TRUE) :
                    fixture.reads || fixture.writes || fixture.phase_count)) return 1;
        }
    return 0;
}

static lib_i32 v30_mode_round_trip(lib_u8 source, lib_bool enabled,
    lib_u8 fail_kind, lib_u32 fail_part, lib_u32 *reads, lib_u32 *writes)
{
    const lib_bool nmi = source == 1u, intr = source == 2u, calln = source == 3u;
    static v30_memory_fixture fixture;
    static const x86_core_cpu_bus_provider provider = {.read_memory=v30_memory_read,
        .write_memory=v30_memory_write,.interrupt_pending=cpu_bus_pending,
        .acknowledge_interrupt=cpu_bus_acknowledge};
    const v30_mode_case item = {.operation=V30_MODE_BRKEM,.pc=0x100u,.sp=0x701u,
        .flags=VCPU_EFLAGS_CF | VCPU_EFLAGS_DF | (enabled ? VCPU_EFLAGS_IF : 0u),.vector=0x80u};
    if (v30_mode_prepare(&fixture,&item)) return 1;
    const t_cpu initial = fixture.bus.cpu;
    x86_core_cpu_execution_refresh(&fixture.bus.execution);
    if (fixture.bus.execution.stop_requested) return 1;
    fixture.bus.execution.bus = &provider;
    const lib_u16 next = calln ? 0x203u : 0x201u;
    v30_mode_code(&fixture,0x90000u,0x200u,
        calln ? (const lib_u8[]){0xedu,0xedu,0x30u,0xedu,0xfdu} :
        (const lib_u8[]){0x00u,0xedu,0xfdu},calln ? 5u : 3u);
    const lib_u8 vector = nmi ? 2u : 0x30u;
    v30_mode_store(&fixture,4u*vector,0x300u);
    v30_mode_store(&fixture,4u*vector+2u,0xa000u);
    fixture.memory[0xa0300u] = 0xcfu;
    if (nmi && !x86_core_cpu_request_nmi(&fixture.bus.execution)) return 1;
    fixture.bus.interrupt = intr;
    v30_mode_clear_probes(&fixture);
    const t_cpu emulation = fixture.bus.cpu;
    const lib_bool delivered = nmi || (intr && enabled) || calln;
    v30_memory_plan plan = {0};
    if (delivered) {
        const lib_u16 frame[] = {(lib_u16)(emulation.data.flags | 0x7000u),0x9000u,next};
        for (lib_u8 word = 0u; word < 3u; ++word)
            v30_memory_plan_transfer(&plan,&fixture,
                0x20000u+(lib_u16)(emulation.data.sp-2u-2u*word),2u,LIB_TRUE,frame[word]);
        v30_memory_plan_transfer(&plan,&fixture,4u*vector,2u,LIB_FALSE,0x300u);
        v30_memory_plan_transfer(&plan,&fixture,4u*vector+2u,2u,LIB_FALSE,0xa000u);
    }
    if (fail_kind == 1u) fixture.fail_read = fail_part;
    if (fail_kind == 2u) fixture.fail_write = fail_part;
    x86_core_cpu_execution_refresh(&fixture.bus.execution);
    if (fail_part) {
        t_cpu rollback = emulation;
        if (!calln) rollback.data.eip = next;
        return !fixture.bus.execution.stop_requested || !fixture.bus.faults ||
            fixture.bus.delivered_exceptions || !fixture.bus.execution.nec_md_write_enabled ||
            lib_memory_compare(&fixture.bus.cpu,&rollback,sizeof(rollback)) ||
            v30_memory_check_plan(&fixture,&plan,LIB_TRUE);
    }
    *reads = fixture.reads;
    *writes = fixture.writes;
    if (fixture.bus.execution.stop_requested || !fixture.bus.execution.nec_md_write_enabled ||
        fixture.bus.cpu.data.bp != initial.data.bp || fixture.bus.cpu.data.ds.base != initial.data.ds.base ||
        fixture.bus.cpu.data.cs.selector != (delivered ? 0xa000u : 0x9000u) ||
        fixture.bus.cpu.data.eip != (delivered ? 0x300u : next) ||
        !!(fixture.bus.cpu.data.flags & X86_CPU_NEC_PSW_MD) != delivered ||
        fixture.bus.acknowledgements != (lib_u32)(intr && enabled) ||
        fixture.bus.cpu.data.sp != (lib_u16)(item.sp-(delivered ? 12u : 6u)) ||
        fixture.bus.cpu.data.flags != (lib_u16)((emulation.data.flags |
            (delivered ? X86_CPU_NEC_PSW_MD : 0u)) &
            (delivered && !calln ? ~(VCPU_EFLAGS_IF | VCPU_EFLAGS_TF) : LIB_UINT16_MAX)) ||
        v30_memory_check_plan(&fixture,&plan,LIB_FALSE)) return 1;
    if (delivered) {
        x86_core_cpu_execution_refresh(&fixture.bus.execution);
        if (fixture.bus.execution.stop_requested || fixture.bus.cpu.data.eip != next ||
            fixture.bus.cpu.data.cs.selector != 0x9000u ||
            (fixture.bus.cpu.data.flags & X86_CPU_NEC_PSW_MD) ||
            fixture.bus.cpu.data.sp != (lib_u16)(item.sp-6u)) return 1;
    }
    x86_core_cpu_execution_refresh(&fixture.bus.execution);
    t_cpu expected = initial;
    expected.data.eip += 3u;
    if (fixture.bus.execution.stop_requested || fixture.bus.execution.nec_md_write_enabled ||
        lib_memory_compare(&fixture.bus.cpu,&expected,sizeof(expected))) return 1;
    /* RETEM must lock MD again: a subsequent native POP PSW cannot clear it. */
    fixture.memory[0x80103u] = 0x9du;
    v30_mode_store(&fixture,0x20000u+fixture.bus.cpu.data.sp,0u);
    v30_mode_clear_probes(&fixture);
    x86_core_cpu_execution_refresh(&fixture.bus.execution);
    return fixture.bus.execution.stop_requested || fixture.bus.execution.nec_md_write_enabled ||
        fixture.bus.cpu.data.flags != 0x8002u;
}

static lib_i32 v30_mode_guards(void)
{
    static v30_memory_fixture fixture;
    const v30_mode_case item = {.operation=V30_MODE_CALLN,.pc=0x100u,.sp=0x700u,.vector=0x81u};
    if (v30_mode_prepare(&fixture,&item)) return 1;
    x86_core_cpu_execution_refresh(&fixture.bus.execution);
    const t_cpu before = fixture.bus.cpu;
    v30_mode_code(&fixture,fixture.bus.cpu.data.cs.base,(lib_u16)fixture.bus.cpu.data.eip,
        (const lib_u8[]){0x0fu,0xffu,0x80u},3u);
    v30_mode_clear_probes(&fixture);
    x86_core_cpu_execution_refresh(&fixture.bus.execution);
    if (!fixture.bus.execution.stop_requested || !fixture.bus.execution.nec_md_write_enabled ||
        fixture.bus.delivered_exceptions || fixture.reads || fixture.writes ||
        lib_memory_compare(&fixture.bus.cpu,&before,sizeof(before))) return 1;
    for (lib_u16 byte = 1u; byte < 256u; ++byte) {
        if (byte == 0xedu) continue;
        const v30_mode_case nop = {.operation=V30_MODE_NOP,.pc=0x100u,.sp=0x700u};
        if (v30_mode_prepare(&fixture,&nop)) return 1;
        fixture.memory[0x80100u] = (lib_u8)byte;
        const t_cpu unchanged = fixture.bus.cpu;
        x86_core_cpu_execution_refresh(&fixture.bus.execution);
        if (!fixture.bus.execution.stop_requested || fixture.bus.delivered_exceptions ||
            fixture.reads || fixture.writes || !fixture.bus.execution.nec_md_write_enabled ||
            lib_memory_compare(&fixture.bus.cpu,&unchanged,sizeof(unchanged))) return 1;
    }
    for (lib_u16 suffix = 0u; suffix < 256u; ++suffix) {
        if (suffix == 0xedu || suffix == 0xfdu) continue;
        const v30_mode_case nop = {.operation=V30_MODE_NOP,.pc=0xffffu,.sp=0x701u};
        if (v30_mode_prepare(&fixture,&nop)) return 1;
        v30_mode_code(&fixture,0x80000u,0xffffu,
            (const lib_u8[]){0xedu,(lib_u8)suffix},2u);
        const t_cpu unchanged = fixture.bus.cpu;
        x86_core_cpu_instruction_lexeme lexeme;
        if (x86_core_cpu_execution_preview_lexeme(&fixture.bus.execution,&lexeme)) return 1;
        x86_core_cpu_execution_refresh(&fixture.bus.execution);
        if (!fixture.bus.execution.stop_requested || fixture.bus.delivered_exceptions ||
            fixture.reads || fixture.writes || !fixture.bus.execution.nec_md_write_enabled ||
            lib_memory_compare(&fixture.bus.cpu,&unchanged,sizeof(unchanged))) return 1;
    }
    for (lib_u8 native = 0u; native < 2u; ++native) {
        if (v30_mode_prepare(&fixture,&item)) return 1;
        if (native) x86_core_cpu_execution_refresh(&fixture.bus.execution);
        x86_core_cpu_state_reset(&fixture.bus.execution);
        if (fixture.bus.execution.nec_md_write_enabled ||
            !(fixture.bus.cpu.data.flags & X86_CPU_NEC_PSW_MD)) return 1;
    }
    /* Context initialization, like reset, cannot inherit an old invocation. */
    fixture.bus.execution.nec_md_write_enabled = LIB_TRUE;
    x86_core_cpu_execution_context_initialize(&fixture.bus.execution,
        &fixture.bus.cpu,&fixture.bus.instructions,&cpu_bus_provider,&fixture.bus);
    if (fixture.bus.execution.nec_md_write_enabled) return 1;
    return 0;
}

lib_i32 main(void)
{
    static const lib_u16 pcs[] = {0x100u,1u,0xfffeu,0xffffu};
    static const lib_u16 stacks[] = {0x700u,0x701u,0u,1u,0xffffu};
    static const lib_u16 targets[] = {0u,1u,0x8000u,0xffffu};
    static const lib_u16 bits[] = {VCPU_EFLAGS_CF,VCPU_EFLAGS_PF,VCPU_EFLAGS_AF,
        VCPU_EFLAGS_ZF,VCPU_EFLAGS_SF,VCPU_EFLAGS_IF,VCPU_EFLAGS_DF,VCPU_EFLAGS_OF};
    lib_u32 cases = 0u;
    for (lib_u8 operation = V30_MODE_BRKEM; operation <= V30_MODE_CALLN; ++operation)
        for (lib_u16 flags = 0u; flags < 256u; ++flags) {
            lib_u16 psw = 0u;
            for (lib_u8 bit = 0u; bit < sizeof(bits)/sizeof(bits[0]); ++bit)
                if (flags & (1u << bit)) psw |= bits[bit];
            for (lib_u16 vector = 0u; vector < 256u; ++vector) {
                const v30_mode_case item = {.operation=(v30_mode_operation)operation,
                    .pc=0x100u,.sp=0x701u,.flags=psw,.vector=(lib_u8)vector};
                if (v30_mode_check(&item,LIB_FALSE)) return 1;
                ++cases;
            }
        }
    for (lib_u8 operation = V30_MODE_RETEM; operation <= V30_MODE_POP_PSW; ++operation)
        for (lib_u32 flags = 0u; flags < 65536u; ++flags) {
            const v30_mode_case item = {.operation=(v30_mode_operation)operation,
                .pc=0x100u,.sp=0x701u,.restored=(lib_u16)flags,.target=0x300u,.segment=0x9000u};
            if (v30_mode_check(&item,LIB_FALSE)) return 1;
            ++cases;
        }
    for (lib_u8 operation = V30_MODE_BRKEM; operation <= V30_MODE_NOP; ++operation)
        for (lib_u8 pc = 0u; pc < sizeof(pcs)/sizeof(pcs[0]); ++pc)
            for (lib_u8 stack = 0u; stack < sizeof(stacks)/sizeof(stacks[0]); ++stack)
                for (lib_u8 value = 0u; value < sizeof(targets)/sizeof(targets[0]); ++value) {
                    const v30_mode_case item = {.operation=(v30_mode_operation)operation,
                        .pc=pcs[pc],.sp=stacks[stack],.flags=0x0ed5u,
                        .restored=targets[value] | 0x0ed5u,.target=targets[value],
                        .segment=targets[3u-value],.vector=(lib_u8)targets[value]};
                    if (v30_mode_check(&item,LIB_TRUE)) return 1;
                    ++cases;
                }
    for (lib_u8 alias = 1u; alias <= 2u; ++alias)
        for (lib_u8 operation = V30_MODE_BRKEM; operation <= V30_MODE_NOP; ++operation)
            for (lib_u8 odd = 0u; odd < 2u; ++odd) {
                const lib_u16 offset = operation <= V30_MODE_CALLN ? 6u : 1u;
                const v30_mode_case item = {.operation=(v30_mode_operation)operation,
                    .pc=0x101u,.sp=(lib_u16)((alias == 1u ? 0x101u : 0x200u)+offset+odd),
                    .flags=0x0ed5u,.restored=0x8ed5u,.target=0x300u,.segment=0x9000u,
                    .vector=0x80u,.alias=alias};
                if (v30_mode_check(&item,LIB_TRUE)) return 1;
                ++cases;
            }
    for (lib_u8 source = 0u; source < 4u; ++source)
        for (lib_u8 enabled = 0u; enabled < 2u; ++enabled) {
            lib_u32 reads = 0u, writes = 0u;
            if (v30_mode_round_trip(source,enabled != 0u,0u,0u,&reads,&writes)) return 1;
            for (lib_u8 kind = 1u; kind <= 2u; ++kind)
                for (lib_u32 part = 1u; part <= (kind == 1u ? reads : writes); ++part)
                    if (v30_mode_round_trip(source,enabled != 0u,kind,part,&reads,&writes)) return 1;
            ++cases;
        }
    if (v30_mode_guards()) return 1;
    lib_c_printf("V30:MODE-TRANSITION:OK cases=%u\n",cases);
    return 0;
}
