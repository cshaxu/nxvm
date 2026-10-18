#include "support/cpu_v30_memory_fixture.h"
#include "lib/types/file.h"

typedef enum v30_interrupt_operation {
    V30_INTERRUPT_NOP, V30_INTERRUPT_ADD, V30_INTERRUPT_DI, V30_INTERRUPT_EI,
    V30_INTERRUPT_HALT, V30_INTERRUPT_COMPARE, V30_INTERRUPT_SEGMENT
} v30_interrupt_operation;

typedef struct v30_interrupt_case {
    v30_interrupt_operation operation;
    lib_u16 pc, sp, flags;
    lib_u8 events;
    lib_bool emulation, masked;
} v30_interrupt_case;

static void v30_interrupt_word(v30_memory_fixture *fixture, lib_u32 address, lib_u16 value)
{
    fixture->memory[address]=(lib_u8)value;
    fixture->memory[address+1u]=(lib_u8)(value >> 8u);
}

static void v30_interrupt_clear(v30_memory_fixture *fixture)
{
    lib_memory_set(&fixture->record_count,0,
        sizeof(*fixture)-lib_offsetof(v30_memory_fixture,record_count));
    fixture->bus.faults=fixture->bus.delivered_exceptions=fixture->bus.instruction_count=0u;
    fixture->bus.acknowledgements=0u;
}

static lib_u16 v30_interrupt_target(lib_u8 vector)
{
    return (lib_u16)(0x200u+4u*vector);
}

static lib_u8 v30_interrupt_prepare(v30_memory_fixture *fixture, const v30_interrupt_case *item)
{
    static const x86_core_cpu_bus_provider provider={
        .read_memory=v30_memory_read,.write_memory=v30_memory_write,
        .interrupt_pending=cpu_bus_pending,.acknowledge_interrupt=cpu_bus_acknowledge
    };
    static const lib_u8 native[]={0x90u,0x04u,0xfau,0xfbu,0xf4u,0xf2u,0x8eu};
    static const lib_u8 emulation[]={0x00u,0xc6u,0xf3u,0xfbu,0x76u};
    v30_memory_restart(fixture);
    if (item->emulation) {
        lib_memory_copy(fixture->memory+0x80100u,(const lib_u8[]){0x0fu,0xffu,0x80u},3u);
        v30_interrupt_word(fixture,0x200u,0x100u);
        v30_interrupt_word(fixture,0x202u,0x9000u);
        x86_core_cpu_execution_refresh(&fixture->bus.execution);
        if (fixture->bus.execution.stop_requested || !fixture->bus.execution.nec_md_write_enabled ||
            (fixture->bus.cpu.data.flags & X86_CPU_NEC_PSW_MD)) return 0u;
    }
    fixture->bus.execution.bus=&provider;
    t_cpu *cpu=&fixture->bus.cpu;
    cpu->data.eax=0x12340007u;
    cpu->data.ecx=0x23450002u;
    cpu->data.edx=0x34561234u;
    cpu->data.ebx=0x45670500u;
    cpu->data.ebp=0x56780400u;
    cpu->data.esi=0x67890500u;
    cpu->data.edi=0x789a0600u;
    cpu->data.esp=0x89ab0000u | item->sp;
    cpu->data.eip=item->pc;
    cpu->data.eflags=(item->emulation ? 0u : X86_CPU_NEC_PSW_MD) | 2u | item->flags;
    for (lib_i16 byte=-12; byte < 2; ++byte)
        fixture->memory[0x20000u+(lib_u16)(item->sp+byte)]=0xa5u;
    fixture->memory[0x10500u]=fixture->memory[0x10501u]=1u;
    fixture->memory[0x30600u]=fixture->memory[0x30601u]=2u;
    static const lib_u8 vectors[]={1u,2u,0x30u};
    for (lib_u8 index=0u; index < sizeof(vectors); ++index) {
        const lib_u16 target=v30_interrupt_target(vectors[index]);
        v30_interrupt_word(fixture,4u*vectors[index],target);
        v30_interrupt_word(fixture,4u*vectors[index]+2u,0x9000u);
        fixture->memory[0x90000u+target]=0x90u;
        fixture->memory[0x90001u+target]=0xcfu;
    }
    lib_u8 code[2]={item->emulation ? emulation[item->operation] : native[item->operation],0u};
    lib_u8 length=1u;
    if (item->operation == V30_INTERRUPT_ADD) code[length++]=1u;
    if (item->operation == V30_INTERRUPT_COMPARE) code[length++]=0xa6u;
    if (item->operation == V30_INTERRUPT_SEGMENT) code[length++]=0xd8u;
    for (lib_u8 byte=0u; byte < length; ++byte)
        fixture->memory[cpu->data.cs.base+(lib_u16)(item->pc+byte)]=code[byte];
    for (lib_u8 byte=0u; byte < 4u; ++byte)
        fixture->memory[cpu->data.cs.base+(lib_u16)(item->pc+length+byte)]=item->emulation ? 0x00u : 0x90u;
    x86_core_cpu_set_nmi_mask(&fixture->bus.execution,item->masked);
    if ((item->events & 1u) && x86_core_cpu_request_nmi(&fixture->bus.execution) == item->masked) return 0u;
    fixture->bus.interrupt=(item->events & 2u) != 0u;
    v30_interrupt_clear(fixture);
    return length;
}

static void v30_interrupt_result(v30_memory_fixture *fixture, const v30_interrupt_case *item,
    lib_u8 length, t_cpu *expected, v30_memory_plan *plan)
{
    expected->data.eip=(lib_u16)(item->pc+length);
    switch (item->operation) {
    case V30_INTERRUPT_ADD:
        expected->data.al=8u; /* 7+1 has no defined native/8080 status flag set. */
        expected->data.eflags &= ~(item->emulation ? 0xd5u : 0x8d5u);
        break;
    case V30_INTERRUPT_DI: expected->data.flags &= ~VCPU_EFLAGS_IF; break;
    case V30_INTERRUPT_EI: expected->data.flags |= VCPU_EFLAGS_IF; break;
    case V30_INTERRUPT_HALT: expected->data.flagHalt=LIB_TRUE; break;
    case V30_INTERRUPT_COMPARE:
        v30_memory_plan_transfer(plan,fixture,expected->data.ds.base+expected->data.si,1u,LIB_FALSE,1u);
        v30_memory_plan_transfer(plan,fixture,expected->data.es.base+expected->data.di,1u,LIB_FALSE,2u);
        expected->data.flags=(lib_u16)((expected->data.flags & ~0x8d5u) | 0x95u);
        ++expected->data.si;
        ++expected->data.di;
        --expected->data.cx;
        if (expected->data.cx) expected->data.eip=item->pc;
        break;
    case V30_INTERRUPT_SEGMENT:
        expected->data.ds.selector=7u;
        expected->data.ds.base=0x70u;
        break;
    default: break;
    }
    if ((item->flags & VCPU_EFLAGS_TF) && item->operation != V30_INTERRUPT_SEGMENT)
        expected->data.dr6 |= 0x4000u; /* Existing private DR6.BS step bookkeeping. */
}

static lib_u8 v30_interrupt_winner(const v30_interrupt_case *item, const t_cpu *after)
{
    if (item->operation == V30_INTERRUPT_SEGMENT) return 0u;
    if ((item->events & 1u) && !item->masked) return 2u;
    if ((item->events & 2u) && (after->data.flags & VCPU_EFLAGS_IF) &&
        item->operation != V30_INTERRUPT_EI) return 0x30u;
    if ((item->flags & VCPU_EFLAGS_TF) &&
        (item->operation != V30_INTERRUPT_HALT || (item->events & 2u))) return 1u;
    return 0u;
}

static void v30_interrupt_entry(v30_memory_fixture *fixture, t_cpu *expected,
    v30_memory_plan *plan, lib_u8 vector)
{
    const lib_u16 words[]={(lib_u16)(expected->data.flags | 0x7000u),
        expected->data.cs.selector,(lib_u16)expected->data.eip};
    for (lib_u8 word=0u; word < 3u; ++word)
        v30_memory_plan_transfer(plan,fixture,expected->data.ss.base+
            (lib_u16)(expected->data.sp-2u*(word+1u)),2u,LIB_TRUE,words[word]);
    v30_memory_plan_transfer(plan,fixture,4u*vector,2u,LIB_FALSE,v30_interrupt_target(vector));
    v30_memory_plan_transfer(plan,fixture,4u*vector+2u,2u,LIB_FALSE,0x9000u);
    expected->data.sp-=6u;
    expected->data.cs.selector=0x9000u;
    expected->data.cs.base=0x90000u;
    expected->data.eip=v30_interrupt_target(vector);
    expected->data.flags=(lib_u16)((expected->data.flags & ~(VCPU_EFLAGS_IF | VCPU_EFLAGS_TF)) |
        X86_CPU_NEC_PSW_MD);
    expected->data.flagHalt=LIB_FALSE;
}

static void v30_interrupt_return_plan(v30_memory_fixture *fixture, const t_cpu *entered,
    const t_cpu *returned, v30_memory_plan *plan)
{
    const lib_u16 words[]={(lib_u16)returned->data.eip,returned->data.cs.selector,
        (lib_u16)(returned->data.flags | 0x7000u)};
    for (lib_u8 word=0u; word < 3u; ++word)
        v30_memory_plan_transfer(plan,fixture,entered->data.ss.base+
            (lib_u16)(entered->data.sp+2u*word),2u,LIB_FALSE,words[word]);
}

static lib_i32 v30_interrupt_case_check(const v30_interrupt_case *item)
{
    static v30_memory_fixture fixture;
    const lib_u8 length=v30_interrupt_prepare(&fixture,item);
    if (!length) return 1;
    t_cpu after=fixture.bus.cpu;
    v30_memory_plan plan={0};
    v30_interrupt_result(&fixture,item,length,&after,&plan);
    lib_u8 winner=v30_interrupt_winner(item,&after);
    t_cpu expected=after;
    if (winner) v30_interrupt_entry(&fixture,&expected,&plan,winner);
    else if (item->operation == V30_INTERRUPT_HALT && (item->events & 2u))
        expected.data.flagHalt=LIB_FALSE;
    x86_core_cpu_execution_refresh(&fixture.bus.execution);
    const lib_bool nmi_pending=(item->events & 1u) && !item->masked && winner != 2u;
    const lib_bool int_pending=(item->events & 2u) && winner != 0x30u;
    if (fixture.bus.execution.stop_requested || fixture.bus.faults ||
        fixture.bus.execution.nmi_pending != nmi_pending || fixture.bus.interrupt != int_pending ||
        fixture.bus.acknowledgements != (lib_u32)(winner == 0x30u) ||
        fixture.bus.delivered_exceptions != (lib_u32)(winner == 1u) ||
        lib_memory_compare(&fixture.bus.cpu,&expected,sizeof(expected)) ||
        v30_memory_check_plan(&fixture,&plan,LIB_FALSE) ||
        (winner && (fixture.bus.execution.debug_trap_pending || fixture.bus.execution.debug_trap_cause))) {
        lib_c_printf("V30:INTERRUPT:ENTRY mode=%u op=%u flags=%04x event=%u mask=%u winner=%u pc=%x/%x sp=%x/%x\n",
            item->emulation,item->operation,item->flags,item->events,item->masked,winner,
            fixture.bus.cpu.data.eip,expected.data.eip,fixture.bus.cpu.data.sp,expected.data.sp);
        return 1;
    }
    if (!winner && (item->operation == V30_INTERRUPT_SEGMENT || item->operation == V30_INTERRUPT_EI)) {
        v30_interrupt_clear(&fixture);
        plan=(v30_memory_plan){0};
        ++after.data.eip;
        after.data.eip=(lib_u16)after.data.eip;
        if (after.data.flags & VCPU_EFLAGS_TF) after.data.dr6 |= 0x4000u;
        v30_interrupt_case next=*item;
        next.operation=V30_INTERRUPT_NOP;
        next.flags=after.data.flags;
        winner=v30_interrupt_winner(&next,&after);
        expected=after;
        if (winner) v30_interrupt_entry(&fixture,&expected,&plan,winner);
        x86_core_cpu_execution_refresh(&fixture.bus.execution);
        if (fixture.bus.execution.stop_requested || fixture.bus.faults ||
            fixture.bus.acknowledgements != (lib_u32)(winner == 0x30u) ||
            fixture.bus.delivered_exceptions != (lib_u32)(winner == 1u) ||
            lib_memory_compare(&fixture.bus.cpu,&expected,sizeof(expected)) ||
            v30_memory_check_plan(&fixture,&plan,LIB_FALSE)) return 1;
    }
    if (!winner) return 0;
    v30_interrupt_clear(&fixture);
    plan=(v30_memory_plan){0};
    ++expected.data.eip;
    x86_core_cpu_execution_refresh(&fixture.bus.execution);
    if (fixture.bus.execution.stop_requested || fixture.bus.faults || fixture.bus.writes ||
        fixture.bus.delivered_exceptions || lib_memory_compare(&fixture.bus.cpu,&expected,sizeof(expected))) return 1;
    v30_interrupt_return_plan(&fixture,&expected,&after,&plan);
    expected=after;
    expected.data.flagHalt=LIB_FALSE;
    const lib_bool retained_int=winner != 0x30u && (item->events & 2u) &&
        (after.data.flags & VCPU_EFLAGS_IF);
    if (retained_int) v30_interrupt_entry(&fixture,&expected,&plan,0x30u);
    x86_core_cpu_execution_refresh(&fixture.bus.execution);
    if (fixture.bus.execution.stop_requested || fixture.bus.faults || fixture.bus.delivered_exceptions ||
        lib_memory_compare(&fixture.bus.cpu,&expected,sizeof(expected)) ||
        fixture.bus.acknowledgements != (lib_u32)retained_int ||
        v30_memory_check_plan(&fixture,&plan,LIB_FALSE)) {
        lib_c_printf("V30:INTERRUPT:RETURN mode=%u op=%u flags=%04x event=%u mask=%u winner=%u pc=%x/%x flags=%x/%x sp=%x/%x dr6=%x/%x faults=%u delivered=%u ack=%u/%u reads=%u writes=%u plan=%u\n",
            item->emulation,item->operation,item->flags,item->events,item->masked,winner,
            fixture.bus.cpu.data.eip,expected.data.eip,fixture.bus.cpu.data.flags,expected.data.flags,
            fixture.bus.cpu.data.sp,expected.data.sp,fixture.bus.cpu.data.dr6,expected.data.dr6,
            fixture.bus.faults,fixture.bus.delivered_exceptions,fixture.bus.acknowledgements,
            retained_int,fixture.reads,fixture.writes,plan.count);
        return 1;
    }
    if (retained_int) {
        v30_interrupt_clear(&fixture);
        plan=(v30_memory_plan){0};
        ++expected.data.eip;
        x86_core_cpu_execution_refresh(&fixture.bus.execution);
        if (fixture.bus.execution.stop_requested || fixture.bus.faults || fixture.bus.writes ||
            fixture.bus.delivered_exceptions || lib_memory_compare(&fixture.bus.cpu,&expected,sizeof(expected))) return 1;
        v30_interrupt_return_plan(&fixture,&expected,&after,&plan);
        expected=after;
        expected.data.flagHalt=LIB_FALSE;
        x86_core_cpu_execution_refresh(&fixture.bus.execution);
        if (fixture.bus.execution.stop_requested || fixture.bus.faults || fixture.bus.delivered_exceptions ||
            lib_memory_compare(&fixture.bus.cpu,&expected,sizeof(expected)) ||
            v30_memory_check_plan(&fixture,&plan,LIB_FALSE)) return 1;
    }
    /* RETI restores BRK, but a fresh guest instruction must precede the next
     * step. Neither an old request nor a step in either handler may substitute. */
    if (after.data.flags & VCPU_EFLAGS_TF) {
        v30_interrupt_clear(&fixture);
        plan=(v30_memory_plan){0};
        v30_interrupt_case next=*item;
        next.pc=(lib_u16)after.data.eip;
        next.flags=after.data.flags;
        next.operation=item->operation == V30_INTERRUPT_COMPARE ? V30_INTERRUPT_COMPARE : V30_INTERRUPT_NOP;
        expected=after;
        expected.data.flagHalt=LIB_FALSE;
        v30_interrupt_result(&fixture,&next,next.operation == V30_INTERRUPT_COMPARE ? 2u : 1u,&expected,&plan);
        v30_interrupt_entry(&fixture,&expected,&plan,1u);
        x86_core_cpu_execution_refresh(&fixture.bus.execution);
        if (fixture.bus.execution.stop_requested || fixture.bus.faults ||
            fixture.bus.delivered_exceptions != 1u || fixture.bus.acknowledgements ||
            lib_memory_compare(&fixture.bus.cpu,&expected,sizeof(expected)) ||
            v30_memory_check_plan(&fixture,&plan,LIB_FALSE)) return 1;
    }
    return 0;
}

/* An actual HALT queues BRK without accepting it. Its subsequent wake gives
 * each entry-refusal probe a known post-instruction checkpoint in both modes. */
static lib_i32 v30_interrupt_wake(v30_memory_fixture *fixture, const v30_interrupt_case *item,
    t_cpu *checkpoint, v30_memory_plan *plan)
{
    v30_interrupt_case halted=*item;
    halted.events=0u;
    if (!v30_interrupt_prepare(fixture,&halted)) return 1;
    x86_core_cpu_execution_refresh(&fixture->bus.execution);
    if (fixture->bus.execution.stop_requested || !fixture->bus.cpu.data.flagHalt ||
        !fixture->bus.execution.debug_trap_pending) return 1;
    *checkpoint=fixture->bus.cpu;
    checkpoint->data.flagHalt=LIB_FALSE;
    t_cpu entered=*checkpoint;
    v30_interrupt_entry(fixture,&entered,plan,item->events == 1u ? 2u : 0x30u);
    if (item->events == 1u) {
        if (!x86_core_cpu_request_nmi(&fixture->bus.execution)) return 1;
    } else fixture->bus.interrupt=LIB_TRUE;
    v30_interrupt_clear(fixture);
    return 0;
}

static lib_i32 v30_interrupt_refusals(void)
{
    static v30_memory_fixture fixture;
    for (lib_u8 emulation=0u; emulation < 2u; ++emulation)
        for (lib_u8 source=0u; source < 2u; ++source)
            for (lib_u8 odd=0u; odd < 2u; ++odd) {
                const v30_interrupt_case item={.operation=V30_INTERRUPT_HALT,.pc=0xffffu,
                    .sp=odd ? 1u : 0u,.flags=VCPU_EFLAGS_IF | VCPU_EFLAGS_TF | 0x8d5u,
                    .events=source ? 2u : 1u,.emulation=emulation != 0u};
                t_cpu checkpoint;
                v30_memory_plan plan={0};
                if (v30_interrupt_wake(&fixture,&item,&checkpoint,&plan)) return 1;
                const t_cpu halted=fixture.bus.cpu;
                x86_core_cpu_instruction_lexeme lexeme;
                if (!x86_core_cpu_execution_preview_lexeme(&fixture.bus.execution,&lexeme) ||
                    lexeme.byte_count != 1u || !fixture.bus.execution.debug_trap_pending ||
                    fixture.bus.execution.nmi_pending != !source || fixture.bus.interrupt != (source != 0u) ||
                    fixture.bus.acknowledgements || fixture.bus.instruction_count || fixture.reads || fixture.writes ||
                    lib_memory_compare(&fixture.bus.cpu,&halted,sizeof(halted))) return 1;
                const lib_u32 observations=fixture.observations;
                v30_interrupt_clear(&fixture);
                x86_core_cpu_execution_refresh(&fixture.bus.execution);
                if (fixture.bus.execution.stop_requested || fixture.bus.faults ||
                    fixture.bus.execution.debug_trap_pending || v30_memory_check_plan(&fixture,&plan,LIB_FALSE)) return 1;
                const lib_u32 counts[]={fixture.reads,fixture.writes,fixture.fetches,observations};
                for (lib_u8 kind=0u; kind < sizeof(counts)/sizeof(counts[0]); ++kind)
                    for (lib_u32 ordinal=1u; ordinal <= counts[kind]; ++ordinal) {
                        v30_memory_plan refused_plan={0};
                        if (v30_interrupt_wake(&fixture,&item,&checkpoint,&refused_plan)) return 1;
                        if (kind == 3u) {
                            fixture.fail_observe=ordinal;
                            const t_cpu before=fixture.bus.cpu;
                            if (x86_core_cpu_execution_preview_lexeme(&fixture.bus.execution,&lexeme) ||
                                fixture.bus.execution.stop_requested || fixture.bus.faults ||
                                fixture.bus.execution.nmi_pending != !source || fixture.bus.interrupt != (source != 0u) ||
                                !fixture.bus.execution.debug_trap_pending || fixture.reads || fixture.writes ||
                                fixture.bus.acknowledgements || lib_memory_compare(&fixture.bus.cpu,&before,sizeof(before))) return 1;
                            continue;
                        }
                        if (kind == 0u) fixture.fail_read=ordinal;
                        else if (kind == 1u) fixture.fail_write=ordinal;
                        else fixture.fail_fetch=ordinal;
                        x86_core_cpu_execution_refresh(&fixture.bus.execution);
                        if (!fixture.bus.execution.stop_requested || fixture.bus.faults != 1u ||
                            fixture.bus.delivered_exceptions || !fixture.bus.execution.debug_trap_pending ||
                            fixture.bus.execution.nmi_pending != !source || fixture.bus.interrupt ||
                            fixture.bus.acknowledgements != source ||
                            fixture.bus.execution.debug_trap_cause != 0x4000u ||
                            lib_memory_compare(&fixture.bus.cpu,&checkpoint,sizeof(checkpoint)) ||
                            (kind < 2u && v30_memory_check_plan(&fixture,&refused_plan,LIB_TRUE))) {
                            lib_c_printf("V30:INTERRUPT:REFUSAL mode=%u source=%u odd=%u kind=%u ordinal=%u pc=%x/%x halt=%u/%u pending=%u\n",
                                emulation,source,odd,kind,ordinal,fixture.bus.cpu.data.eip,checkpoint.data.eip,
                                fixture.bus.cpu.data.flagHalt,checkpoint.data.flagHalt,fixture.bus.execution.debug_trap_pending);
                            return 1;
                        }
                        if (kind == 2u && (fixture.reads || fixture.writes)) return 1;
                    }
                if (source) {
                    plan=(v30_memory_plan){0};
                    if (v30_interrupt_wake(&fixture,&item,&checkpoint,&plan)) return 1;
                    fixture.bus.fail_acknowledge=LIB_TRUE;
                    x86_core_cpu_execution_refresh(&fixture.bus.execution);
                    if (!fixture.bus.execution.stop_requested || fixture.bus.faults != 1u ||
                        !fixture.bus.interrupt || fixture.bus.acknowledgements || fixture.reads || fixture.writes ||
                        !fixture.bus.execution.debug_trap_pending ||
                        lib_memory_compare(&fixture.bus.cpu,&checkpoint,sizeof(checkpoint))) return 1;
                }
            }
    return 0;
}

static lib_i32 v30_interrupt_step_refusals(void)
{
    static v30_memory_fixture fixture;
    for (lib_u8 emulation=0u; emulation < 2u; ++emulation)
        for (lib_u8 odd=0u; odd < 2u; ++odd) {
            const v30_interrupt_case item={.operation=V30_INTERRUPT_NOP,.pc=0xffffu,
                .sp=odd ? 1u : 0u,.flags=VCPU_EFLAGS_IF | VCPU_EFLAGS_TF | 0x8d5u,
                .emulation=emulation != 0u};
            const lib_u8 length=v30_interrupt_prepare(&fixture,&item);
            if (!length) return 1;
            t_cpu checkpoint=fixture.bus.cpu;
            v30_memory_plan plan={0};
            v30_interrupt_result(&fixture,&item,length,&checkpoint,&plan);
            t_cpu entered=checkpoint;
            v30_interrupt_entry(&fixture,&entered,&plan,1u);
            x86_core_cpu_execution_refresh(&fixture.bus.execution);
            if (fixture.bus.execution.stop_requested || fixture.bus.faults ||
                lib_memory_compare(&fixture.bus.cpu,&entered,sizeof(entered)) ||
                v30_memory_check_plan(&fixture,&plan,LIB_FALSE)) return 1;
            const lib_u32 counts[]={fixture.reads,fixture.writes};
            for (lib_u8 kind=0u; kind < 2u; ++kind)
                for (lib_u32 ordinal=1u; ordinal <= counts[kind]; ++ordinal) {
                    if (!v30_interrupt_prepare(&fixture,&item)) return 1;
                    v30_memory_plan refused={0};
                    checkpoint=fixture.bus.cpu;
                    v30_interrupt_result(&fixture,&item,length,&checkpoint,&refused);
                    entered=checkpoint;
                    v30_interrupt_entry(&fixture,&entered,&refused,1u);
                    if (kind == 0u) fixture.fail_read=ordinal;
                    else fixture.fail_write=ordinal;
                    x86_core_cpu_execution_refresh(&fixture.bus.execution);
                    /* The winning step consumes its own request before entry;
                     * host failure stops execution, never retries that prefix. */
                    if (!fixture.bus.execution.stop_requested || fixture.bus.faults != 1u ||
                        fixture.bus.delivered_exceptions || fixture.bus.acknowledgements ||
                        fixture.bus.execution.debug_trap_pending || fixture.bus.execution.debug_trap_cause ||
                        lib_memory_compare(&fixture.bus.cpu,&checkpoint,sizeof(checkpoint)) ||
                        v30_memory_check_plan(&fixture,&refused,LIB_TRUE)) return 1;
                }
        }
    return 0;
}

lib_i32 main(void)
{
    const v30_interrupt_case witness={.operation=V30_INTERRUPT_NOP,.pc=0x100u,.sp=0x701u,
        .flags=VCPU_EFLAGS_IF | VCPU_EFLAGS_TF,.events=3u};
    if (v30_interrupt_case_check(&witness)) return 1;
    static const lib_u16 pcs[]={0x100u,0x101u,0xfffeu,0xffffu};
    static const lib_u16 stacks[]={0u,1u,5u,0x700u,0xffffu};
    lib_u32 cases=1u;
    for (lib_u8 emulation=0u; emulation < 2u; ++emulation)
        for (lib_u8 operation=0u; operation < (emulation ? 5u : 7u); ++operation)
            for (lib_u8 flags=0u; flags < 8u; ++flags)
                for (lib_u8 events=0u; events < 4u; ++events)
                    for (lib_u8 masked=0u; masked < 2u; ++masked)
                        for (lib_u8 pc=0u; pc < sizeof(pcs)/sizeof(pcs[0]); ++pc)
                            for (lib_u8 sp=0u; sp < sizeof(stacks)/sizeof(stacks[0]); ++sp) {
                                const v30_interrupt_case item={.operation=operation,.pc=pcs[pc],.sp=stacks[sp],
                                    .flags=(lib_u16)((flags & 1u ? VCPU_EFLAGS_IF : 0u) |
                                        (flags & 2u ? VCPU_EFLAGS_TF : 0u) | (flags & 4u ? 0x8d5u : 0u)),
                                    .events=events,.emulation=emulation != 0u,.masked=masked != 0u};
                                if (v30_interrupt_case_check(&item)) return 1;
                                ++cases;
                            }
    if (v30_interrupt_refusals()) { lib_c_printf("V30:INTERRUPT:REFUSALS:FAIL\n"); return 1; }
    if (v30_interrupt_step_refusals()) { lib_c_printf("V30:INTERRUPT:STEP-REFUSALS:FAIL\n"); return 1; }
    lib_c_printf("V30:INTERRUPT:OK cases=%u\n",cases);
    return 0;
}
