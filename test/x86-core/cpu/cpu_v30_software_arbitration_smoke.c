#include "support/cpu_v30_memory_fixture.h"
#include "lib/types/file.h"

typedef enum v30_software_operation {
    V30_SOFTWARE_BRK3, V30_SOFTWARE_BRK, V30_SOFTWARE_BRKV,
    V30_SOFTWARE_DIVIDE, V30_SOFTWARE_INDEX, V30_SOFTWARE_BRKEM,
    V30_SOFTWARE_CALLN, V30_SOFTWARE_OVERFLOW_CLEAR, V30_SOFTWARE_INDEX_VALID
} v30_software_operation;

typedef struct v30_software_case {
    v30_software_operation operation;
    lib_u16 pc, sp, flags;
    lib_u8 events;
} v30_software_case;

static void v30_software_word(v30_memory_fixture *fixture, lib_u32 address, lib_u16 value)
{
    fixture->memory[address]=(lib_u8)value;
    fixture->memory[address+1u]=(lib_u8)(value >> 8u);
}

static void v30_software_clear(v30_memory_fixture *fixture)
{
    lib_memory_set(&fixture->record_count,0,
        sizeof(*fixture)-lib_offsetof(v30_memory_fixture,record_count));
    fixture->bus.faults=fixture->bus.delivered_exceptions=fixture->bus.instruction_count=0u;
    fixture->bus.acknowledgements=0u;
}

static lib_u8 v30_software_vector(v30_software_operation operation)
{
    static const lib_u8 vectors[]={3u,0x31u,4u,0u,5u,0x81u,0x82u,4u,5u};
    return vectors[operation];
}

static lib_u16 v30_software_target(lib_u8 vector)
{
    return (lib_u16)(0x400u+4u*vector);
}

static lib_u8 v30_software_prepare(v30_memory_fixture *fixture, const v30_software_case *item)
{
    static const x86_core_cpu_bus_provider provider={
        .read_memory=v30_memory_read,.write_memory=v30_memory_write,
        .interrupt_pending=cpu_bus_pending,.acknowledge_interrupt=cpu_bus_acknowledge
    };
    v30_memory_restart(fixture);
    if (item->operation == V30_SOFTWARE_CALLN) {
        lib_memory_copy(fixture->memory+0x80100u,(const lib_u8[]){0x0fu,0xffu,0x80u},3u);
        v30_software_word(fixture,0x200u,0x100u);
        v30_software_word(fixture,0x202u,0x9000u);
        x86_core_cpu_execution_refresh(&fixture->bus.execution);
        if (fixture->bus.execution.stop_requested || !fixture->bus.execution.nec_md_write_enabled ||
            (fixture->bus.cpu.data.flags & X86_CPU_NEC_PSW_MD)) return 0u;
    }
    fixture->bus.execution.bus=&provider;
    t_cpu *cpu=&fixture->bus.cpu;
    cpu->data.cs.selector=0x8000u;
    cpu->data.cs.base=0x80000u;
    cpu->data.eip=item->pc;
    cpu->data.eax=0x12340001u;
    cpu->data.ebx=0x23450000u;
    cpu->data.esp=0x34560000u | item->sp;
    cpu->data.eflags=2u | item->flags |
        (item->operation == V30_SOFTWARE_CALLN ? 0u : X86_CPU_NEC_PSW_MD);
    if (item->operation == V30_SOFTWARE_BRKV) cpu->data.flags |= VCPU_EFLAGS_OF;
    if (item->operation == V30_SOFTWARE_OVERFLOW_CLEAR) cpu->data.flags &= ~VCPU_EFLAGS_OF;
    for (lib_i16 byte=-12; byte < 2; ++byte)
        fixture->memory[0x20000u+(lib_u16)(item->sp+byte)]=0xa5u;
    v30_software_word(fixture,0x10500u,0u);
    v30_software_word(fixture,0x10502u,item->operation == V30_SOFTWARE_INDEX_VALID ? 2u : 0u);
    static const lib_u8 vectors[]={0u,1u,2u,3u,4u,5u,0x30u,0x31u,0x81u,0x82u};
    for (lib_u8 index=0u; index < sizeof(vectors); ++index) {
        const lib_u16 target=v30_software_target(vectors[index]);
        v30_software_word(fixture,4u*vectors[index],target);
        v30_software_word(fixture,4u*vectors[index]+2u,0x9000u);
        fixture->memory[0x90000u+target]=0x90u;
        fixture->memory[0x90001u+target]=0xcfu;
    }
    if (item->operation == V30_SOFTWARE_BRKEM)
        fixture->memory[0x90000u+v30_software_target(0x81u)]=0x00u;
    const lib_u8 code[][4]={{0xccu},{0xcdu,0x31u},{0xceu},{0xf6u,0xf3u},
        {0x62u,0x06u,0x00u,0x05u},{0x0fu,0xffu,0x81u},{0xedu,0xedu,0x82u},
        {0xceu},{0x62u,0x06u,0x00u,0x05u}};
    static const lib_u8 lengths[]={1u,2u,1u,2u,4u,3u,3u,1u,4u};
    const lib_u8 length=lengths[item->operation];
    for (lib_u8 byte=0u; byte < 8u; ++byte)
        fixture->memory[0x80000u+(lib_u16)(item->pc+byte)]=0x90u;
    for (lib_u8 byte=0u; byte < length; ++byte)
        fixture->memory[0x80000u+(lib_u16)(item->pc+byte)]=code[item->operation][byte];
    if ((item->events & 1u) && !x86_core_cpu_request_nmi(&fixture->bus.execution)) return 0u;
    fixture->bus.interrupt=(item->events & 2u) != 0u;
    v30_software_clear(fixture);
    return length;
}

static void v30_software_entry(v30_memory_fixture *fixture, t_cpu *expected,
    v30_memory_plan *plan, lib_u8 vector, lib_bool mode_call)
{
    const lib_u16 words[]={(lib_u16)(expected->data.flags | 0x7000u),
        expected->data.cs.selector,(lib_u16)expected->data.eip};
    for (lib_u8 word=0u; word < 3u; ++word)
        v30_memory_plan_transfer(plan,fixture,expected->data.ss.base+
            (lib_u16)(expected->data.sp-2u*(word+1u)),2u,LIB_TRUE,words[word]);
    v30_memory_plan_transfer(plan,fixture,4u*vector,2u,LIB_FALSE,v30_software_target(vector));
    v30_memory_plan_transfer(plan,fixture,4u*vector+2u,2u,LIB_FALSE,0x9000u);
    expected->data.sp-=6u;
    expected->data.cs.selector=0x9000u;
    expected->data.cs.base=0x90000u;
    expected->data.eip=v30_software_target(vector);
    if (!mode_call) expected->data.flags &= ~(VCPU_EFLAGS_IF | VCPU_EFLAGS_TF);
    expected->data.flags |= X86_CPU_NEC_PSW_MD;
    if (vector == 0x81u) expected->data.flags &= ~X86_CPU_NEC_PSW_MD;
}

static void v30_software_operands(v30_memory_fixture *fixture,
    const v30_software_case *item, v30_memory_plan *plan)
{
    if (item->operation == V30_SOFTWARE_INDEX || item->operation == V30_SOFTWARE_INDEX_VALID) {
        v30_memory_plan_transfer(plan,fixture,0x10500u,2u,LIB_FALSE,0u);
        v30_memory_plan_transfer(plan,fixture,0x10502u,2u,LIB_FALSE,
            item->operation == V30_SOFTWARE_INDEX_VALID ? 2u : 0u);
    }
}

static lib_i32 v30_software_check(const v30_software_case *item)
{
    static v30_memory_fixture fixture;
    const lib_u8 length=v30_software_prepare(&fixture,item);
    if (!length) return 1;
    t_cpu expected=fixture.bus.cpu;
    expected.data.eip=(lib_u16)(item->pc+length);
    v30_memory_plan plan={0};
    v30_software_operands(&fixture,item,&plan);
    const lib_bool taken=item->operation <= V30_SOFTWARE_CALLN;
    const lib_bool mode_call=item->operation == V30_SOFTWARE_BRKEM || item->operation == V30_SOFTWARE_CALLN;
    const lib_u8 vector=v30_software_vector(item->operation);
    lib_u8 winner=0u;
    if (taken) v30_software_entry(&fixture,&expected,&plan,vector,mode_call);
    else {
        if (item->flags & VCPU_EFLAGS_TF) expected.data.dr6 |= 0x4000u;
        winner=item->events & 1u ? 2u :
            (item->events & 2u) && (item->flags & VCPU_EFLAGS_IF) ? 0x30u :
            item->flags & VCPU_EFLAGS_TF ? 1u : 0u;
        if (winner) v30_software_entry(&fixture,&expected,&plan,winner,LIB_FALSE);
    }
    x86_core_cpu_execution_refresh(&fixture.bus.execution);
    if (fixture.bus.execution.stop_requested || fixture.bus.faults ||
        fixture.bus.execution.nmi_pending != ((item->events & 1u) && winner != 2u) ||
        fixture.bus.interrupt != ((item->events & 2u) && winner != 0x30u) ||
        fixture.bus.acknowledgements != (lib_u32)(winner == 0x30u) ||
        fixture.bus.execution.instruction_software_entry != taken ||
        fixture.bus.execution.debug_trap_pending ||
        lib_memory_compare(&fixture.bus.cpu,&expected,sizeof(expected)) ||
        v30_memory_check_plan(&fixture,&plan,LIB_FALSE)) {
        lib_c_printf("V30:SOFTWARE-ARBITRATION:ENTRY op=%u flags=%04x events=%u pc=%x/%x sp=%x/%x flags=%x/%x\n",
            item->operation,item->flags,item->events,fixture.bus.cpu.data.eip,expected.data.eip,
            fixture.bus.cpu.data.sp,expected.data.sp,fixture.bus.cpu.data.flags,expected.data.flags);
        return 1;
    }
    if (!taken) return 0;
    /* A real handler instruction, not a second refresh of the old completion,
     * reopens asynchronous admission; mode calls preserve IE/BRK. */
    v30_software_clear(&fixture);
    plan=(v30_memory_plan){0};
    ++expected.data.eip;
    if (expected.data.flags & VCPU_EFLAGS_TF) expected.data.dr6 |= 0x4000u;
    winner=item->events & 1u ? 2u :
        (item->events & 2u) && (expected.data.flags & VCPU_EFLAGS_IF) ? 0x30u :
        expected.data.flags & VCPU_EFLAGS_TF ? 1u : 0u;
    if (winner) v30_software_entry(&fixture,&expected,&plan,winner,LIB_FALSE);
    x86_core_cpu_execution_refresh(&fixture.bus.execution);
    if (fixture.bus.execution.stop_requested || fixture.bus.faults ||
        fixture.bus.execution.nmi_pending ||
        fixture.bus.interrupt != ((item->events & 2u) && winner != 0x30u) ||
        fixture.bus.acknowledgements != (lib_u32)(winner == 0x30u) ||
        fixture.bus.execution.instruction_software_entry ||
        fixture.bus.execution.debug_trap_pending ||
        lib_memory_compare(&fixture.bus.cpu,&expected,sizeof(expected)) ||
        v30_memory_check_plan(&fixture,&plan,LIB_FALSE)) {
        lib_c_printf("V30:SOFTWARE-ARBITRATION:FOLLOW op=%u flags=%04x events=%u winner=%u pc=%x/%x sp=%x/%x\n",
            item->operation,item->flags,item->events,winner,fixture.bus.cpu.data.eip,expected.data.eip,
            fixture.bus.cpu.data.sp,expected.data.sp);
        return 1;
    }
    return 0;
}

static lib_i32 v30_software_refusals(void)
{
    static v30_memory_fixture fixture;
    for (lib_u8 operation=0u; operation <= V30_SOFTWARE_CALLN; ++operation)
        for (lib_u8 odd=0u; odd < 2u; ++odd) {
            const v30_software_case item={.operation=operation,.pc=0xffffu,.sp=odd ? 1u : 0u,
                .flags=VCPU_EFLAGS_IF | VCPU_EFLAGS_TF,.events=3u};
            const lib_u8 length=v30_software_prepare(&fixture,&item);
            if (!length) return 1;
            t_cpu checkpoint=fixture.bus.cpu;
            v30_memory_plan plan={0};
            v30_software_operands(&fixture,&item,&plan);
            t_cpu entered=checkpoint;
            entered.data.eip=(lib_u16)(item.pc+length);
            v30_software_entry(&fixture,&entered,&plan,v30_software_vector(operation),
                operation == V30_SOFTWARE_BRKEM || operation == V30_SOFTWARE_CALLN);
            x86_core_cpu_instruction_lexeme lexeme;
            if (!x86_core_cpu_execution_preview_lexeme(&fixture.bus.execution,&lexeme) ||
                lexeme.byte_count != length || fixture.reads || fixture.writes || fixture.fetches ||
                fixture.bus.acknowledgements || !fixture.bus.interrupt ||
                !fixture.bus.execution.nmi_pending ||
                lib_memory_compare(&fixture.bus.cpu,&checkpoint,sizeof(checkpoint))) return 1;
            const lib_u32 observations=fixture.observations;
            v30_software_clear(&fixture);
            x86_core_cpu_execution_refresh(&fixture.bus.execution);
            if (fixture.bus.execution.stop_requested || v30_memory_check_plan(&fixture,&plan,LIB_FALSE)) return 1;
            const lib_u32 counts[]={fixture.reads,fixture.writes,fixture.fetches,observations};
            for (lib_u8 kind=0u; kind < sizeof(counts)/sizeof(counts[0]); ++kind)
                for (lib_u32 ordinal=1u; ordinal <= counts[kind]; ++ordinal) {
                    if (!v30_software_prepare(&fixture,&item)) return 1;
                    plan=(v30_memory_plan){0};
                    v30_software_operands(&fixture,&item,&plan);
                    entered=checkpoint;
                    entered.data.eip=(lib_u16)(item.pc+length);
                    v30_software_entry(&fixture,&entered,&plan,v30_software_vector(operation),
                        operation == V30_SOFTWARE_BRKEM || operation == V30_SOFTWARE_CALLN);
                    if (kind == 3u) {
                        const lib_bool prior_entry=fixture.bus.execution.instruction_software_entry;
                        fixture.fail_observe=ordinal;
                        if (x86_core_cpu_execution_preview_lexeme(&fixture.bus.execution,&lexeme) ||
                            fixture.bus.execution.stop_requested || fixture.bus.faults ||
                            fixture.bus.execution.instruction_software_entry != prior_entry ||
                            fixture.reads || fixture.writes || fixture.bus.acknowledgements ||
                            !fixture.bus.interrupt || !fixture.bus.execution.nmi_pending ||
                            lib_memory_compare(&fixture.bus.cpu,&checkpoint,sizeof(checkpoint))) {
                            lib_c_printf("V30:SOFTWARE-ARBITRATION:PREVIEW op=%u odd=%u ordinal=%u\n",
                                operation,odd,ordinal);
                            return 1;
                        }
                        continue;
                    }
                    if (kind == 0u) fixture.fail_read=ordinal;
                    else if (kind == 1u) fixture.fail_write=ordinal;
                    else fixture.fail_fetch=ordinal;
                    x86_core_cpu_execution_refresh(&fixture.bus.execution);
                    if (!fixture.bus.execution.stop_requested || fixture.bus.faults != 1u ||
                        fixture.bus.execution.instruction_software_entry ||
                        fixture.bus.acknowledgements || !fixture.bus.interrupt ||
                        !fixture.bus.execution.nmi_pending ||
                        lib_memory_compare(&fixture.bus.cpu,&checkpoint,sizeof(checkpoint)) ||
                        (kind < 2u && v30_memory_check_plan(&fixture,&plan,LIB_TRUE))) {
                        lib_c_printf("V30:SOFTWARE-ARBITRATION:REFUSAL op=%u odd=%u kind=%u ordinal=%u pc=%x/%x flags=%x/%x fault=%u pending=%u/%u plan=%d\n",
                            operation,odd,kind,ordinal,fixture.bus.cpu.data.eip,checkpoint.data.eip,
                            fixture.bus.cpu.data.flags,checkpoint.data.flags,fixture.bus.faults,
                            fixture.bus.execution.nmi_pending,fixture.bus.interrupt,
                            v30_memory_check_plan(&fixture,&plan,LIB_TRUE));
                        return 1;
                    }
                    if (kind == 2u && (fixture.reads || fixture.writes)) return 1;
                }
        }
    return 0;
}

lib_i32 main(void)
{
    const v30_software_case witness={.operation=V30_SOFTWARE_BRK3,.pc=0x100u,.sp=0x701u,
        .flags=VCPU_EFLAGS_IF | VCPU_EFLAGS_TF,.events=3u};
    if (v30_software_check(&witness)) return 1;
    static const lib_u16 pcs[]={0x100u,0xfffeu,0xffffu};
    static const lib_u16 stacks[]={0u,1u,0x700u,0xffffu};
    lib_u32 cases=1u;
    for (lib_u8 operation=0u; operation <= V30_SOFTWARE_INDEX_VALID; ++operation)
        for (lib_u8 flags=0u; flags < 8u; ++flags)
            for (lib_u8 events=0u; events < 4u; ++events)
                for (lib_u8 pc=0u; pc < sizeof(pcs)/sizeof(pcs[0]); ++pc)
                    for (lib_u8 sp=0u; sp < sizeof(stacks)/sizeof(stacks[0]); ++sp) {
                        const v30_software_case item={.operation=operation,.pc=pcs[pc],.sp=stacks[sp],
                            .flags=(lib_u16)((flags & 1u ? VCPU_EFLAGS_IF : 0u) |
                                (flags & 2u ? VCPU_EFLAGS_TF : 0u) | (flags & 4u ? 0x8d5u : 0u)),
                            .events=events};
                        if (v30_software_check(&item)) return 1;
                        ++cases;
                    }
    if (v30_software_refusals()) return 1;
    lib_c_printf("V30:SOFTWARE-ARBITRATION:OK cases=%u\n",cases);
    return 0;
}
