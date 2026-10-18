#include "support/cpu_v30_memory_fixture.h"
#include "lib/types/file.h"

typedef enum v30_segment_operation {
    V30_SEGMENT_STORE_REG, V30_SEGMENT_LOAD_REG,
    V30_SEGMENT_STORE_MEMORY, V30_SEGMENT_LOAD_MEMORY,
    V30_SEGMENT_PUSH, V30_SEGMENT_POP, V30_SEGMENT_LOAD_FAR
} v30_segment_operation;

typedef struct v30_segment_case {
    v30_segment_operation operation;
    lib_u8 code[7], bytes, segment, reg;
    lib_u16 sp, bp, selector;
    lib_u32 address;
} v30_segment_case;

typedef struct v30_segment_transfer {
    lib_u32 address;
    lib_u16 value;
    lib_u8 bytes;
    lib_bool write;
} v30_segment_transfer;

static lib_u16 *v30_segment_gpr(t_cpu *cpu, lib_u8 index)
{
    lib_u16 *registers[]={&cpu->data.ax,&cpu->data.cx,&cpu->data.dx,&cpu->data.bx,
        &cpu->data.sp,&cpu->data.bp,&cpu->data.si,&cpu->data.di};
    return registers[index];
}

static t_cpu_data_sreg *v30_segment_register(t_cpu *cpu, lib_u8 index)
{
    t_cpu_data_sreg *registers[]={&cpu->data.es,&cpu->data.cs,&cpu->data.ss,&cpu->data.ds};
    return registers[index];
}

static void v30_segment_prepare(v30_memory_fixture *fixture, const v30_segment_case *item)
{
    static const x86_core_cpu_bus_provider provider={
        .read_memory=v30_memory_read, .write_memory=v30_memory_write,
        .interrupt_pending=cpu_bus_pending, .acknowledge_interrupt=cpu_bus_acknowledge};
    v30_memory_prepare(fixture);
    fixture->bus.execution.bus=&provider;
    fixture->bus.cpu.data.eax=0x11114567u;
    fixture->bus.cpu.data.ecx=0x22222345u;
    fixture->bus.cpu.data.edx=0x33333456u;
    fixture->bus.cpu.data.ebx=0x44445678u;
    fixture->bus.cpu.data.esp=0x55550000u+item->sp;
    fixture->bus.cpu.data.ebp=0x66660000u+item->bp;
    fixture->bus.cpu.data.esi=0x77776789u;
    fixture->bus.cpu.data.edi=0x8888789au;
    fixture->bus.cpu.data.eflags |= VCPU_EFLAGS_CF | VCPU_EFLAGS_PF | VCPU_EFLAGS_AF |
        VCPU_EFLAGS_ZF | VCPU_EFLAGS_SF | VCPU_EFLAGS_DF | VCPU_EFLAGS_OF;
    const lib_u32 address=item->operation==V30_SEGMENT_POP ? 0x20000u+item->sp : item->address;
    if (item->operation==V30_SEGMENT_POP || item->operation==V30_SEGMENT_LOAD_MEMORY)
        lib_memory_copy(fixture->memory+address,&item->selector,2u);
    if (item->operation==V30_SEGMENT_LOAD_FAR) {
        const lib_u8 pointer[]={0x78u,0x9au,(lib_u8)item->selector,(lib_u8)(item->selector>>8u)};
        lib_memory_copy(fixture->memory+address,pointer,sizeof(pointer));
    }
    lib_memory_copy(fixture->memory+0x80100u,item->code,item->bytes);
    fixture->memory[0x80100u+item->bytes]=fixture->memory[0x80101u+item->bytes]=0x90u;
}

static lib_bool v30_segment_inhibits(const v30_segment_case *item)
{
    return item->operation!=V30_SEGMENT_PUSH && item->operation!=V30_SEGMENT_LOAD_FAR;
}

/* The input operation, not production decoding, selects the independent result. */
static v30_segment_transfer v30_segment_expect(t_cpu *cpu, const v30_segment_case *item)
{
    t_cpu_data_sreg *segment=v30_segment_register(cpu,item->segment);
    v30_segment_transfer transfer={.address=item->address,.value=segment->selector};
    switch (item->operation) {
    case V30_SEGMENT_STORE_REG: *v30_segment_gpr(cpu,item->reg)=transfer.value; break;
    case V30_SEGMENT_LOAD_REG:
        segment->selector=*v30_segment_gpr(cpu,item->reg);
        segment->base=(lib_u32)segment->selector<<4u;
        break;
    case V30_SEGMENT_STORE_MEMORY: transfer.bytes=2u; transfer.write=LIB_TRUE; break;
    case V30_SEGMENT_PUSH:
        cpu->data.sp=(lib_u16)(cpu->data.sp-2u);
        transfer.address=cpu->data.ss.base+cpu->data.sp;
        transfer.bytes=2u;
        transfer.write=LIB_TRUE;
        break;
    case V30_SEGMENT_POP:
        transfer.address=cpu->data.ss.base+cpu->data.sp;
        cpu->data.sp=(lib_u16)(cpu->data.sp+2u);
        transfer.bytes=2u;
        segment->selector=item->selector;
        segment->base=(lib_u32)item->selector<<4u;
        break;
    case V30_SEGMENT_LOAD_MEMORY:
        transfer.bytes=2u;
        segment->selector=item->selector;
        segment->base=(lib_u32)item->selector<<4u;
        break;
    case V30_SEGMENT_LOAD_FAR:
        transfer.bytes=4u;
        segment->selector=item->selector;
        segment->base=(lib_u32)item->selector<<4u;
        *v30_segment_gpr(cpu,item->reg)=0x9a78u;
        break;
    }
    cpu->data.eip+=item->bytes;
    return transfer;
}

static lib_i32 v30_segment_records(const v30_memory_fixture *fixture, lib_u8 *memory,
    lib_u32 address, lib_u8 bytes, lib_bool write, lib_u16 value, lib_u32 failure)
{
    lib_u32 part=0u, offset=0u;
    for (lib_u32 index=0u; index<fixture->record_count; ++index) {
        const v30_memory_record *record=&fixture->records[index];
        if (record->observe || record->provenance!=X86_CORE_CPU_MEMORY_ACCESS_DATA) continue;
        const lib_u8 width=((address+offset) & 1u) || (offset & 1u) ? 1u : 2u;
        const lib_bool accepted=++part!=failure;
        if (offset>=bytes || (failure && part>failure) || record->address!=address+offset ||
            record->bytes!=width || record->write!=write || record->accepted!=accepted ||
            fixture->phases[2u*part-2u]!=X86_CORE_CPU_EXTERNAL_CYCLE_PHASE_BEGIN ||
            fixture->phases[2u*part-1u]!=(accepted ? X86_CORE_CPU_EXTERNAL_CYCLE_PHASE_COMMIT :
                X86_CORE_CPU_EXTERNAL_CYCLE_PHASE_CANCEL)) return 1;
        if (write && accepted)
            for (lib_u8 byte=0u; byte<width; ++byte)
                memory[address+offset+byte]=(lib_u8)(value>>((offset+byte)*8u));
        offset+=width;
    }
    return fixture->invalid || fixture->phase_count!=2u*part ||
        fixture->reads!=(write ? 0u : part) || fixture->writes!=(write ? part : 0u) ||
        (failure ? part!=failure : offset!=bytes) ||
        lib_memory_compare(memory,fixture->memory,sizeof(fixture->memory))!=0;
}

static lib_i32 v30_segment_check(const v30_segment_case *item)
{
    static v30_memory_fixture fixture;
    static lib_u8 memory[0x100000u];
    x86_core_cpu_instruction_lexeme lexeme;
    v30_segment_prepare(&fixture,item);
    const t_cpu before=fixture.bus.cpu;
    t_cpu after=before;
    lib_memory_copy(memory,fixture.memory,sizeof(memory));
    const v30_segment_transfer transfer=v30_segment_expect(&after,item);
    if (!x86_core_cpu_execution_preview_lexeme(&fixture.bus.execution,&lexeme) ||
        lexeme.byte_count!=item->bytes || fixture.reads || fixture.writes ||
        fixture.bus.execution.interrupt_shadow!=CPU_INTERRUPT_SHADOW_NONE ||
        lib_memory_compare(&fixture.bus.cpu,&before,sizeof(before))!=0) return 1;
    fixture.record_count=fixture.observations=0u;
    x86_core_cpu_execution_refresh(&fixture.bus.execution);
    if (item->operation==V30_SEGMENT_PUSH || item->operation==V30_SEGMENT_POP) {
        const lib_bool unprefixed=item->bytes==1u;
        const lib_u64 clocks=unprefixed ? 8u+4u*(item->sp & 1u) : 0u;
        const lib_u32 form=clocks ? item->code[0] : X86_CORE_CPU_RETIREMENT_SOURCE_FORM_UNATTRIBUTED;
        const x86_core_cpu_timing_result timing=x86_core_cpu_capture_timing(&fixture.bus.execution);
        if (timing.ticks!=clocks || !timing.source_timing_unallocated || timing.event_ticks ||
            timing.key_id!=form || timing.form_id!=form || timing.formula_inputs!=(clocks ?
                X86_CORE_CPU_TIMING_INPUT_MEMORY | ((item->sp & 1u) ? X86_CORE_CPU_TIMING_INPUT_ODD_WORD : 0u) : 0u) ||
            timing.retirement_origin!=(clocks ? X86_CORE_CPU_RETIREMENT_TIMING_ORIGIN_V30_NOMINAL_SOURCE :
                X86_CORE_CPU_RETIREMENT_TIMING_ORIGIN_UNATTRIBUTED)) return 1;
    }
    if (fixture.bus.execution.stop_requested || fixture.bus.faults || fixture.bus.delivered_exceptions ||
        lib_memory_compare(&fixture.bus.cpu,&after,sizeof(after))!=0 ||
        fixture.bus.execution.interrupt_shadow!=(v30_segment_inhibits(item) ?
            CPU_INTERRUPT_SHADOW_SEGMENT : CPU_INTERRUPT_SHADOW_NONE) ||
        v30_segment_records(&fixture,memory,transfer.address,transfer.bytes,transfer.write,transfer.value,0u)) {
        lib_c_printf("V30:SEGMENT-RESULT:%u:%u:%u:%u\n",
            item->operation,item->segment,item->reg,fixture.bus.execution.interrupt_shadow);
        return 1;
    }
    const lib_u32 parts=fixture.reads+fixture.writes;
    for (lib_u32 part=1u; part<=parts; ++part) {
        v30_segment_prepare(&fixture,item);
        lib_memory_copy(memory,fixture.memory,sizeof(memory));
        if (transfer.write) fixture.fail_write=part;
        else fixture.fail_read=part;
        x86_core_cpu_execution_refresh(&fixture.bus.execution);
        if (!fixture.bus.execution.stop_requested || !fixture.bus.faults || fixture.bus.delivered_exceptions ||
            fixture.bus.execution.completed_observation_valid || fixture.bus.execution.timing_result.ticks ||
            fixture.bus.execution.timing_result.retirement_origin!=X86_CORE_CPU_RETIREMENT_TIMING_ORIGIN_UNATTRIBUTED ||
            fixture.bus.execution.interrupt_shadow!=CPU_INTERRUPT_SHADOW_NONE ||
            lib_memory_compare(&fixture.bus.cpu,&before,sizeof(before))!=0 ||
            v30_segment_records(&fixture,memory,transfer.address,transfer.bytes,
                transfer.write,transfer.value,part)) return 1;
    }
    return 0;
}

/* Word starts wrap with SP; each word retains the S8 native data-tail model. */
static lib_bool v30_segment_frame(const v30_memory_fixture *fixture, lib_u16 ip, lib_u16 flags)
{
    const lib_u16 words[]={ip,0x8000u,(lib_u16)(flags | 0x7000u)};
    for (lib_u8 word=0u; word<sizeof(words)/sizeof(words[0]); ++word) {
        const lib_u32 address=(fixture->bus.cpu.data.ss.base+
            (lib_u16)(fixture->bus.cpu.data.sp+2u*word)) & 0xfffffu;
        if (fixture->memory[address]!=(lib_u8)words[word] ||
            fixture->memory[(address+1u) & 0xfffffu]!=(lib_u8)(words[word]>>8u)) return LIB_FALSE;
    }
    return LIB_TRUE;
}

static lib_i32 v30_segment_interrupt(const v30_segment_case *item, lib_u8 event)
{
    static v30_memory_fixture fixture;
    const lib_bool nmi=event==1u, trap=event==2u;
    v30_segment_prepare(&fixture,item);
    fixture.bus.cpu.data.eflags|=VCPU_EFLAGS_IF;
    if (trap) fixture.bus.cpu.data.eflags|=VCPU_EFLAGS_TF;
    const lib_u16 flags=(lib_u16)fixture.bus.cpu.data.eflags;
    t_cpu after=fixture.bus.cpu;
    (void)v30_segment_expect(&after,item);
    const lib_u16 sp=after.data.sp;
    const lib_u8 vector=trap ? 1u : nmi ? 2u : 0x30u;
    lib_memory_copy(fixture.memory+4u*vector,(const lib_u8[]){0u,2u,0u,0x90u},4u);
    fixture.memory[0x90200u]=0xcfu;
    if (nmi) {
        if (!x86_core_cpu_request_nmi(&fixture.bus.execution)) return 1;
    } else if (!trap) fixture.bus.interrupt=LIB_TRUE;
    x86_core_cpu_execution_refresh(&fixture.bus.execution);
    const lib_bool inhibits=v30_segment_inhibits(item);
    if (inhibits) {
        if (fixture.bus.execution.stop_requested ||
            lib_memory_compare(&fixture.bus.cpu,&after,sizeof(after))!=0 || fixture.bus.acknowledgements ||
            fixture.bus.delivered_exceptions ||
            fixture.bus.execution.nmi_pending!=nmi) return 1;
        x86_core_cpu_execution_refresh(&fixture.bus.execution);
    }
    const lib_u16 ip=(lib_u16)(0x100u+item->bytes+inhibits);
    if (fixture.bus.execution.stop_requested || fixture.bus.cpu.data.cs.selector!=0x9000u ||
        fixture.bus.cpu.data.ss.base!=after.data.ss.base || fixture.bus.cpu.data.ss.selector!=after.data.ss.selector ||
        fixture.bus.cpu.data.eip!=0x200u || fixture.bus.cpu.data.sp!=(lib_u16)(sp-6u) ||
        fixture.bus.acknowledgements!=(lib_u32)(!nmi && !trap) ||
        fixture.bus.delivered_exceptions!=(lib_u32)trap ||
        !v30_segment_frame(&fixture,ip,flags)) {
        lib_c_printf("V30:SEGMENT-ENTRY:%04x:%04x:%04x:%04x:%u:%u:%04x\n",
            fixture.bus.cpu.data.cs.selector,fixture.bus.cpu.data.eip,fixture.bus.cpu.data.sp,sp,
            fixture.bus.acknowledgements,fixture.bus.delivered_exceptions,flags);
        return 1;
    }
    x86_core_cpu_execution_refresh(&fixture.bus.execution);
    const lib_bool failed=fixture.bus.execution.stop_requested || fixture.bus.cpu.data.cs.selector!=0x8000u ||
        fixture.bus.cpu.data.eip!=ip || fixture.bus.cpu.data.sp!=sp ||
        (lib_u16)fixture.bus.cpu.data.eflags!=flags || fixture.bus.execution.nmi_pending ||
        fixture.bus.interrupt || fixture.bus.acknowledgements!=(lib_u32)(!nmi && !trap);
    if (failed) lib_c_printf("V30:SEGMENT-RETURN:%04x:%04x:%04x:%04x:%04x:%u\n",
        fixture.bus.cpu.data.cs.selector,fixture.bus.cpu.data.eip,fixture.bus.cpu.data.sp,
        fixture.bus.cpu.data.eflags,flags,fixture.bus.execution.stop_requested);
    return failed;
}

static lib_i32 v30_segment_case_check(const v30_segment_case *item)
{
    if (v30_segment_check(item)) return 1;
    for (lib_u8 event=0u; event<3u; ++event)
        if (v30_segment_interrupt(item,event)) {
            lib_c_printf("V30:SEGMENT-EVENT:%u:%u:%u:%u:%04x\n",
                item->operation,item->segment,item->reg,event,item->sp);
            return 1;
        }
    return 0;
}

static lib_i32 v30_segment_forms(void)
{
    static const lib_u8 prefixes[]={0u,0x26u,0x2eu,0x36u,0x3eu};
    static const lib_u16 offsets[]={0x400u,0x401u,0x700u,0xffffu};
    for (lib_u8 segment=0u; segment<4u; ++segment)
        for (lib_u8 load=0u; load<2u; ++load) {
            if (load && segment==1u) continue;
            for (lib_u8 prefix=0u; prefix<sizeof(prefixes); ++prefix) {
                const lib_u8 extra=prefix!=0u;
                for (lib_u8 reg=0u; reg<8u; ++reg) {
                    v30_segment_case item={.operation=load ? V30_SEGMENT_LOAD_REG : V30_SEGMENT_STORE_REG,
                        .sp=0x700u,.bp=0x600u,.segment=segment,.reg=reg,.bytes=extra+2u};
                    if (extra) item.code[0]=prefixes[prefix];
                    item.code[extra]=load ? 0x8eu : 0x8cu;
                    item.code[extra+1u]=0xc0u+(segment<<3u)+reg;
                    if (v30_segment_case_check(&item)) return 1;
                }
                for (lib_u8 location=0u; location<sizeof(offsets)/sizeof(offsets[0]); ++location)
                    for (lib_u8 bp=0u; bp<2u; ++bp) {
                        const lib_u32 bases[]={0x10000u,0x30000u,0x80000u,0x20000u,0x10000u};
                        v30_segment_case item={.operation=load ? V30_SEGMENT_LOAD_MEMORY : V30_SEGMENT_STORE_MEMORY,
                            .sp=0x700u,.bp=bp ? offsets[location] : 0x600u,.selector=0x4567u,
                            .segment=segment,.bytes=extra+(bp ? 3u : 4u),
                            .address=(bp && !prefix ? 0x20000u : bases[prefix])+offsets[location]};
                        if (extra) item.code[0]=prefixes[prefix];
                        item.code[extra]=load ? 0x8eu : 0x8cu;
                        item.code[extra+1u]=(segment<<3u)+(bp ? 0x46u : 6u);
                        item.code[extra+2u]=bp ? 0u : (lib_u8)offsets[location];
                        if (!bp) item.code[extra+3u]=(lib_u8)(offsets[location]>>8u);
                        if (v30_segment_case_check(&item)) return 1;
                    }
            }
        }
    static const lib_u16 stacks[]={0x700u,0x701u,0u,1u,0xffffu};
    for (lib_u8 segment=0u; segment<4u; ++segment)
        for (lib_u8 pop=0u; pop<2u; ++pop) {
            if (pop && segment==1u) continue;
            for (lib_u8 prefix=0u; prefix<sizeof(prefixes); ++prefix)
                for (lib_u8 stack=0u; stack<sizeof(stacks)/sizeof(stacks[0]); ++stack) {
                    const lib_u8 extra=prefix!=0u;
                    v30_segment_case item={.operation=pop ? V30_SEGMENT_POP : V30_SEGMENT_PUSH,
                        .sp=stacks[stack],.bp=0x600u,.selector=0x4567u,.segment=segment,.bytes=extra+1u};
                    if (extra) item.code[0]=prefixes[prefix];
                    item.code[extra]=(segment<<3u)+(pop ? 7u : 6u);
                    if (v30_segment_case_check(&item)) return 1;
                }
        }
    return 0;
}

static lib_i32 v30_segment_far_and_values(void)
{
    static const lib_u8 prefixes[]={0u,0x26u,0x2eu,0x36u,0x3eu};
    static const lib_u16 offsets[]={0x400u,0x401u,0x700u,0xffffu};
    for (lib_u8 target=0u; target<2u; ++target)
        for (lib_u8 reg=0u; reg<8u; ++reg)
            for (lib_u8 prefix=0u; prefix<sizeof(prefixes); ++prefix)
                for (lib_u8 location=0u; location<sizeof(offsets)/sizeof(offsets[0]); ++location)
                    for (lib_u8 bp=0u; bp<2u; ++bp) {
                        const lib_u8 extra=prefix!=0u;
                        const lib_u32 bases[]={0x10000u,0x30000u,0x80000u,0x20000u,0x10000u};
                        v30_segment_case item={.operation=V30_SEGMENT_LOAD_FAR,
                            .sp=0x700u,.bp=bp ? offsets[location] : 0x600u,.selector=0x4567u,
                            .segment=target ? 3u : 0u,.reg=reg,.bytes=extra+(bp ? 3u : 4u),
                            .address=(bp && !prefix ? 0x20000u : bases[prefix])+offsets[location]};
                        if (extra) item.code[0]=prefixes[prefix];
                        item.code[extra]=target ? 0xc5u : 0xc4u;
                        item.code[extra+1u]=(reg<<3u)+(bp ? 0x46u : 6u);
                        item.code[extra+2u]=bp ? 0u : (lib_u8)offsets[location];
                        if (!bp) item.code[extra+3u]=(lib_u8)(offsets[location]>>8u);
                        if (v30_segment_case_check(&item)) return 1;
                    }
    for (lib_u8 segment=0u; segment<4u; ++segment) {
        if (segment==1u) continue;
        for (lib_u8 top=0u; top<2u; ++top) {
            const v30_segment_case item={.operation=V30_SEGMENT_LOAD_MEMORY,
                .code={0x8eu,(lib_u8)(6u+(segment<<3u)),0u,4u},.bytes=4u,
                .sp=0x700u,.bp=0x600u,.selector=top ? 0xffffu : 0u,
                .segment=segment,.address=0x10400u};
            if (v30_segment_case_check(&item)) return 1;
        }
    }
    return 0;
}

static lib_i32 v30_segment_chain_and_reset(void)
{
    static v30_memory_fixture fixture;
    const v30_segment_case item={.operation=V30_SEGMENT_LOAD_REG,
        .code={0x8eu,0xc0u},.bytes=2u,.segment=0u,.sp=0x700u,.bp=0x600u};
    for (lib_u8 nmi=0u; nmi<2u; ++nmi) {
        v30_segment_prepare(&fixture,&item);
        fixture.bus.cpu.data.eflags|=VCPU_EFLAGS_IF;
        const lib_u16 flags=(lib_u16)fixture.bus.cpu.data.eflags;
        lib_memory_copy(fixture.memory+0x80100u,(const lib_u8[]){0x8eu,0xc0u,0x8eu,0xd8u,0x17u,0x90u},6u);
        lib_memory_copy(fixture.memory+0x20700u,(const lib_u8[]){0x67u,0x45u},2u);
        lib_memory_copy(fixture.memory+(nmi ? 8u : 0xc0u),(const lib_u8[]){0u,2u,0u,0x90u},4u);
        fixture.memory[0x90200u]=0xcfu;
        if (nmi) {
            if (!x86_core_cpu_request_nmi(&fixture.bus.execution)) return 1;
        } else fixture.bus.interrupt=LIB_TRUE;
        static const lib_u16 pcs[]={0x102u,0x104u,0x105u};
        for (lib_u8 step=0u; step<sizeof(pcs)/sizeof(pcs[0]); ++step) {
            x86_core_cpu_execution_refresh(&fixture.bus.execution);
            if (fixture.bus.execution.stop_requested || fixture.bus.cpu.data.cs.selector!=0x8000u ||
                fixture.bus.cpu.data.eip!=pcs[step] || fixture.bus.acknowledgements ||
                fixture.bus.execution.interrupt_shadow!=CPU_INTERRUPT_SHADOW_SEGMENT) return 1;
        }
        x86_core_cpu_execution_refresh(&fixture.bus.execution);
        if (fixture.bus.execution.stop_requested || fixture.bus.cpu.data.cs.selector!=0x9000u ||
            fixture.bus.cpu.data.eip!=0x200u || fixture.bus.cpu.data.sp!=0x6fcu ||
            fixture.bus.cpu.data.ss.selector!=0x4567u ||
            !v30_segment_frame(&fixture,0x106u,flags)) return 1;
        x86_core_cpu_execution_refresh(&fixture.bus.execution);
        if (fixture.bus.execution.stop_requested || fixture.bus.cpu.data.cs.selector!=0x8000u ||
            fixture.bus.cpu.data.eip!=0x106u || fixture.bus.cpu.data.sp!=0x702u) return 1;
    }
    v30_segment_prepare(&fixture,&item);
    if (!x86_core_cpu_request_nmi(&fixture.bus.execution)) return 1;
    x86_core_cpu_execution_refresh(&fixture.bus.execution);
    if (!fixture.bus.execution.nmi_pending ||
        fixture.bus.execution.interrupt_shadow!=CPU_INTERRUPT_SHADOW_SEGMENT) return 1;
    x86_core_cpu_execution_reset(&fixture.bus.execution);
    return fixture.bus.execution.nmi_pending || fixture.bus.execution.debug_trap_pending ||
        fixture.bus.execution.interrupt_shadow!=CPU_INTERRUPT_SHADOW_NONE;
}

static lib_i32 v30_segment_masks(void)
{
    static v30_memory_fixture fixture;
    const v30_segment_case item={.operation=V30_SEGMENT_LOAD_REG,
        .code={0x8eu,0xc0u},.bytes=2u,.segment=0u,.sp=0x700u,.bp=0x600u};
    for (lib_u8 nmi=0u; nmi<2u; ++nmi) {
        v30_segment_prepare(&fixture,&item);
        const lib_u16 flags=(lib_u16)(fixture.bus.cpu.data.eflags | (nmi ? 0u : VCPU_EFLAGS_IF));
        lib_memory_copy(fixture.memory+0x80100u,(const lib_u8[]){0x8eu,0xc0u,0x90u,0xfbu,0x90u},5u);
        lib_memory_copy(fixture.memory+(nmi ? 8u : 0xc0u),(const lib_u8[]){0u,2u,0u,0x90u},4u);
        fixture.memory[0x90200u]=0xcfu;
        if (nmi) {
            if (!x86_core_cpu_request_nmi(&fixture.bus.execution)) return 1;
        } else fixture.bus.interrupt=LIB_TRUE;
        x86_core_cpu_execution_refresh(&fixture.bus.execution);
        if (fixture.bus.execution.stop_requested || fixture.bus.cpu.data.cs.selector!=0x8000u ||
            fixture.bus.cpu.data.eip!=0x102u || fixture.bus.acknowledgements) return 1;
        x86_core_cpu_execution_refresh(&fixture.bus.execution);
        if (!nmi) {
            if (fixture.bus.cpu.data.cs.selector!=0x8000u || fixture.bus.cpu.data.eip!=0x103u ||
                !fixture.bus.interrupt || fixture.bus.acknowledgements) return 1;
            x86_core_cpu_execution_refresh(&fixture.bus.execution); /* Actual STI, still no INT. */
            if (fixture.bus.cpu.data.cs.selector!=0x8000u || fixture.bus.cpu.data.eip!=0x104u ||
                fixture.bus.acknowledgements || fixture.bus.execution.interrupt_shadow!=CPU_INTERRUPT_SHADOW_INTR)
                return 1;
            x86_core_cpu_execution_refresh(&fixture.bus.execution);
        }
        const lib_u16 ip=nmi ? 0x103u : 0x105u;
        if (fixture.bus.execution.stop_requested || fixture.bus.cpu.data.cs.selector!=0x9000u ||
            fixture.bus.cpu.data.eip!=0x200u || !v30_segment_frame(&fixture,ip,flags) ||
            fixture.bus.acknowledgements!=(lib_u32)!nmi) return 1;
        x86_core_cpu_execution_refresh(&fixture.bus.execution);
        if (fixture.bus.execution.stop_requested || fixture.bus.cpu.data.cs.selector!=0x8000u ||
            fixture.bus.cpu.data.eip!=ip || fixture.bus.cpu.data.sp!=0x700u ||
            (lib_u16)fixture.bus.cpu.data.eflags!=flags) return 1;
    }
    return 0;
}

lib_i32 main(void)
{
    const v30_segment_case load_es={.operation=V30_SEGMENT_LOAD_REG,
        .code={0x8eu,0xc0u},.bytes=2u,.segment=0u,.sp=0x700u,.bp=0x600u};
    if (v30_segment_check(&load_es)) return 1;
    if (v30_segment_forms()) return 1;
    if (v30_segment_far_and_values()) return 1;
    if (v30_segment_chain_and_reset()) {
        lib_c_printf("V30:SEGMENT-CHAIN-RESET\n");
        return 1;
    }
    if (v30_segment_masks()) {
        lib_c_printf("V30:SEGMENT-MASKS\n");
        return 1;
    }
    lib_c_printf("V30:SEGMENT:OK\n");
    return 0;
}
