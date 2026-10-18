#include "support/cpu_v30_memory_fixture.h"
#include "lib/types/file.h"

#define V30_STACK_WORD_CAPACITY 512u

typedef enum v30_stack_operation {
    V30_STACK_PUSH_REG, V30_STACK_PUSH_DECREMENTED_SP, V30_STACK_POP_REG,
    V30_STACK_PUSH_VALUE, V30_STACK_PUSH_MEMORY, V30_STACK_POP_MEMORY,
    V30_STACK_PUSH_ALL, V30_STACK_POP_ALL, V30_STACK_ENTER, V30_STACK_LEAVE
} v30_stack_operation;

typedef struct v30_stack_case {
    v30_stack_operation operation;
    lib_u8 code[7], bytes, reg, level;
    lib_u16 sp, bp, value, allocation;
    lib_u32 address;
} v30_stack_case;

typedef struct v30_stack_word {
    lib_u32 address;
    lib_u16 value;
    lib_bool write;
} v30_stack_word;

typedef struct v30_stack_model {
    t_cpu cpu;
    lib_u8 memory[0x100000u];
    v30_stack_word words[V30_STACK_WORD_CAPACITY];
    lib_u32 count;
} v30_stack_model;

static lib_u16 *v30_stack_register(t_cpu *cpu, lib_u8 reg)
{
    lib_u16 *registers[] = {&cpu->data.ax,&cpu->data.cx,&cpu->data.dx,&cpu->data.bx,
        &cpu->data.sp,&cpu->data.bp,&cpu->data.si,&cpu->data.di};
    return registers[reg];
}

static lib_u16 v30_stack_word_transfer(v30_stack_model *model, lib_u32 address,
    lib_bool write, lib_u16 value)
{
    const lib_u32 low = address & 0xfffffu, high = (address + 1u) & 0xfffffu;
    if (model->count >= V30_STACK_WORD_CAPACITY) return 0u;
    if (write) {
        model->memory[low] = (lib_u8)value;
        model->memory[high] = (lib_u8)(value >> 8u);
    } else value = (lib_u16)(model->memory[low] + 256u * model->memory[high]);
    model->words[model->count++] = (v30_stack_word){low,value,write};
    return value;
}

static void v30_stack_push(v30_stack_model *model, lib_u16 value)
{
    model->cpu.data.sp = (lib_u16)(model->cpu.data.sp - 2u);
    (void)v30_stack_word_transfer(model, model->cpu.data.ss.base + model->cpu.data.sp,
        LIB_TRUE, value);
}

static lib_u16 v30_stack_pop(v30_stack_model *model)
{
    const lib_u16 value = v30_stack_word_transfer(model,
        model->cpu.data.ss.base + model->cpu.data.sp, LIB_FALSE, 0u);
    model->cpu.data.sp = (lib_u16)(model->cpu.data.sp + 2u);
    return value;
}

/* Inputs describe operations, not decoded production instruction metadata. */
static void v30_stack_expect(v30_stack_model *model, const v30_stack_case *item)
{
    lib_u16 values[8], frame;
    switch (item->operation) {
    case V30_STACK_PUSH_REG: v30_stack_push(model, *v30_stack_register(&model->cpu,item->reg)); break;
    case V30_STACK_PUSH_DECREMENTED_SP: v30_stack_push(model, (lib_u16)(model->cpu.data.sp - 2u)); break;
    case V30_STACK_POP_REG:
        frame = v30_stack_pop(model);
        *v30_stack_register(&model->cpu,item->reg) = frame;
        break;
    case V30_STACK_PUSH_VALUE: v30_stack_push(model,item->value); break;
    case V30_STACK_PUSH_MEMORY:
        frame = v30_stack_word_transfer(model,item->address,LIB_FALSE,0u);
        v30_stack_push(model,frame);
        break;
    case V30_STACK_POP_MEMORY:
        frame = v30_stack_pop(model);
        (void)v30_stack_word_transfer(model,item->address,LIB_TRUE,frame);
        break;
    case V30_STACK_PUSH_ALL:
        for (lib_u8 reg=0u; reg<8u; ++reg) values[reg]=*v30_stack_register(&model->cpu,reg);
        for (lib_u8 reg=0u; reg<8u; ++reg) v30_stack_push(model,values[reg]);
        break;
    case V30_STACK_POP_ALL:
        for (lib_u8 slot=0u; slot<8u; ++slot) {
            if (slot==3u) model->cpu.data.sp=(lib_u16)(model->cpu.data.sp+2u);
            else *v30_stack_register(&model->cpu,7u-slot)=v30_stack_pop(model);
        }
        break;
    case V30_STACK_ENTER:
        v30_stack_push(model,model->cpu.data.bp);
        frame=model->cpu.data.sp;
        for (lib_u16 level=1u; level<item->level; ++level) {
            model->cpu.data.bp=(lib_u16)(model->cpu.data.bp-2u);
            const lib_u16 value=v30_stack_word_transfer(model,
                model->cpu.data.ss.base+model->cpu.data.bp,LIB_FALSE,0u);
            v30_stack_push(model,value);
        }
        if (item->level) v30_stack_push(model,frame);
        model->cpu.data.bp=frame;
        model->cpu.data.sp=(lib_u16)(model->cpu.data.sp-item->allocation);
        break;
    case V30_STACK_LEAVE:
        model->cpu.data.sp=model->cpu.data.bp;
        model->cpu.data.bp=v30_stack_pop(model);
        break;
    }
    model->cpu.data.eip += item->bytes;
}

/* POPR must not touch the saved-SP slot, even when that address refuses reads. */
static lib_status v30_stack_popr_read(void *opaque, lib_u32 address, void *data,
    lib_u8 bytes, x86_core_cpu_memory_access_provenance provenance,
    lib_bool observe, lib_bool reset_fetch, lib_u8 cache_control)
{
    v30_memory_fixture *fixture=opaque;
    const t_cpu *before=&fixture->bus.instructions.data.oldcpu;
    const lib_u32 skipped=(before->data.ss.base+(lib_u16)(before->data.sp+6u)) & 0xfffffu;
    if (!observe && provenance==X86_CORE_CPU_MEMORY_ACCESS_DATA)
        for (lib_u8 byte=0u; byte<bytes; ++byte)
            if (address+byte==skipped || address+byte==((skipped+1u) & 0xfffffu))
                return LIB_STATUS_IO_ERROR;
    return v30_memory_read(opaque,address,data,bytes,provenance,observe,reset_fetch,cache_control);
}

static void v30_stack_prepare(v30_memory_fixture *fixture, const v30_stack_case *item)
{
    static const x86_core_cpu_bus_provider popr_bus={
        .read_memory=v30_stack_popr_read, .write_memory=v30_memory_write,
        .interrupt_pending=cpu_bus_pending};
    static lib_u8 initial[0x100000u];
    static lib_bool initialized=LIB_FALSE;
    if (!initialized) {
        for (lib_u32 address=0u; address<sizeof(initial); ++address)
            initial[address]=(lib_u8)(address ^ (address>>8u) ^ (address>>16u) ^ 0x5au);
        initialized=LIB_TRUE;
    }
    v30_memory_prepare(fixture);
    lib_memory_copy(fixture->memory,initial,sizeof(initial));
    fixture->bus.cpu.data.eax=0x11111234u;
    fixture->bus.cpu.data.ecx=0x22222345u;
    fixture->bus.cpu.data.edx=0x33333456u;
    fixture->bus.cpu.data.ebx=0x44444567u;
    fixture->bus.cpu.data.esp=0x55550000u+item->sp;
    fixture->bus.cpu.data.ebp=0x66660000u+item->bp;
    fixture->bus.cpu.data.esi=0x77776789u;
    fixture->bus.cpu.data.edi=0x8888789au;
    fixture->bus.cpu.data.eflags |= VCPU_EFLAGS_CF | VCPU_EFLAGS_PF | VCPU_EFLAGS_AF |
        VCPU_EFLAGS_ZF | VCPU_EFLAGS_SF | VCPU_EFLAGS_DF | VCPU_EFLAGS_OF;
    lib_memory_copy(fixture->memory+0x80100u,item->code,item->bytes);
    if (item->operation==V30_STACK_POP_ALL) fixture->bus.execution.bus=&popr_bus;
}

static lib_i32 v30_stack_records(const v30_memory_fixture *fixture,
    const v30_stack_model *model, lib_u8 *memory, lib_bool failed)
{
    lib_u32 word=0u, offset=0u, part=0u, reads=0u, writes=0u;
    lib_bool refused=LIB_FALSE;
    for (lib_u32 record=0u; record<fixture->record_count; ++record) {
        const v30_memory_record *actual=&fixture->records[record];
        if (actual->observe || actual->provenance!=X86_CORE_CPU_MEMORY_ACCESS_DATA) continue;
        if (refused || word>=model->count) return 1;
        const v30_stack_word *expected=&model->words[word];
        const lib_u32 address=(expected->address+offset) & 0xfffffu;
        const lib_u8 bytes=offset==0u && (address & 1u)==0u ? 2u : 1u;
        const lib_u32 ordinal=expected->write ? ++writes : ++reads;
        const lib_bool accepted=ordinal!=(expected->write ? fixture->fail_write : fixture->fail_read);
        if (actual->address!=address || actual->bytes!=bytes || actual->write!=expected->write ||
            actual->accepted!=accepted || fixture->phases[part*2u]!=X86_CORE_CPU_EXTERNAL_CYCLE_PHASE_BEGIN ||
            fixture->phases[part*2u+1u]!=(accepted ? X86_CORE_CPU_EXTERNAL_CYCLE_PHASE_COMMIT :
                X86_CORE_CPU_EXTERNAL_CYCLE_PHASE_CANCEL)) return 1;
        ++part;
        if (accepted && expected->write)
            for (lib_u8 byte=0u; byte<bytes; ++byte)
                memory[(address+byte) & 0xfffffu]=(lib_u8)(expected->value >> ((offset+byte)*8u));
        refused=!accepted;
        offset+=bytes;
        if (offset==2u) { ++word; offset=0u; }
    }
    return fixture->invalid || fixture->phase_count!=part*2u ||
        fixture->reads!=reads || fixture->writes!=writes || refused!=failed ||
        (!failed && (word!=model->count || offset)) ||
        lib_memory_compare(memory,fixture->memory,sizeof(fixture->memory))!=0;
}

/* Original NEC 12-146..153 rows, keyed by independent semantic input. */
static lib_i32 v30_stack_timing(const v30_memory_fixture *fixture, const v30_stack_case *item)
{
    lib_u64 clocks=0u;
    lib_u32 inputs=0u;
    const lib_bool memory=item->operation==V30_STACK_PUSH_MEMORY || item->operation==V30_STACK_POP_MEMORY;
    const lib_bool prefix=item->code[0]==0x26u || item->code[0]==0x2eu ||
        item->code[0]==0x36u || item->code[0]==0x3eu;
    const lib_u8 opcode=item->code[prefix];
    lib_bool odd=(item->operation==V30_STACK_LEAVE ? item->bp : item->sp) & 1u;
    const lib_bool alias=!memory && (opcode==0xffu || opcode==0x8fu);
    lib_bool modeled=alias;
    switch (item->operation) {
    case V30_STACK_PUSH_REG: case V30_STACK_PUSH_DECREMENTED_SP: case V30_STACK_POP_REG:
        clocks=odd ? 12u : 8u; break;
    case V30_STACK_PUSH_VALUE: clocks=(item->bytes==2u ? 7u : 8u)+(odd ? 4u : 0u); break;
    case V30_STACK_PUSH_MEMORY: case V30_STACK_POP_MEMORY:
        modeled=((item->address ^ item->sp) & 1u)!=0u;
        odd=((item->address | item->sp) & 1u)!=0u;
        clocks=(item->operation==V30_STACK_PUSH_MEMORY ? 18u : 17u)+(odd ? 8u : 0u);
        inputs=X86_CORE_CPU_TIMING_INPUT_MODRM | X86_CORE_CPU_TIMING_INPUT_EFFECTIVE_ADDRESS;
        break;
    case V30_STACK_PUSH_ALL: clocks=odd ? 67u : 35u; break;
    case V30_STACK_POP_ALL: clocks=odd ? 75u : 43u; break;
    case V30_STACK_ENTER:
        modeled=item->level==1u || (item->level>1u && ((item->sp ^ item->bp) & 1u));
        if (item->level>1u) odd=((item->sp | item->bp) & 1u)!=0u;
        clocks=item->level ? (odd ? 23u+16u*(item->level-1u) : 19u+8u*(item->level-1u)) : odd ? 16u : 12u;
        inputs=X86_CORE_CPU_TIMING_INPUT_SIZE;
        break;
    case V30_STACK_LEAVE: clocks=odd ? 10u : 6u; break;
    }
    lib_u32 form=X86_CORE_CPU_RETIREMENT_SOURCE_FORM_UNATTRIBUTED;
    if (clocks) {
        if (prefix) { clocks+=2u; inputs|=X86_CORE_CPU_TIMING_INPUT_SEGMENT_OVERRIDE; }
        inputs|=X86_CORE_CPU_TIMING_INPUT_MEMORY | (odd ? X86_CORE_CPU_TIMING_INPUT_ODD_WORD : 0u);
        form=opcode | (memory ? 0x100u : 0u);
        if (memory || alias) form|=((lib_u32)(item->code[1u+prefix]>>3u) & 7u)<<9u;
    }
    const x86_core_cpu_timing_result timing=x86_core_cpu_capture_timing(&fixture->bus.execution);
    if (timing.ticks!=clocks || timing.formula_inputs!=inputs || timing.key_id!=form || timing.form_id!=form ||
        !timing.source_timing_unallocated || timing.event_ticks ||
        timing.retirement_origin!=(clocks ? (modeled ? X86_CORE_CPU_RETIREMENT_TIMING_ORIGIN_L2_CONTROL_MODEL :
            X86_CORE_CPU_RETIREMENT_TIMING_ORIGIN_V30_NOMINAL_SOURCE) :
            X86_CORE_CPU_RETIREMENT_TIMING_ORIGIN_UNATTRIBUTED) || fixture->observations ||
        x86_core_cpu_timing_maximum_ticks(X86_CORE_CPU_PROFILE_V30,LIB_NULL)) {
        lib_c_printf("V30:NOMINAL-STACK:FAIL op=%u sp=%04x bp=%04x level=%u ticks=%llu/%llu\n",
            item->operation,item->sp,item->bp,item->level,timing.ticks,clocks);
        return 1;
    }
    return 0;
}

static lib_i32 v30_stack_check(const v30_stack_case *item, lib_bool failures, lib_u32 *cases)
{
    static v30_memory_fixture fixture;
    static v30_stack_model model;
    static lib_u8 memory[0x100000u];
    x86_core_cpu_instruction_lexeme lexeme;
    v30_stack_prepare(&fixture,item);
    const t_cpu before=fixture.bus.cpu;
    model.cpu=before;
    model.count=0u;
    lib_memory_copy(model.memory,fixture.memory,sizeof(model.memory));
    v30_stack_expect(&model,item);
    if (model.count>=V30_STACK_WORD_CAPACITY) return 1;
    if (!x86_core_cpu_execution_preview_lexeme(&fixture.bus.execution,&lexeme) ||
        lexeme.byte_count!=item->bytes || fixture.reads || fixture.writes || fixture.phase_count ||
        lib_memory_compare(&fixture.bus.cpu,&before,sizeof(before))!=0) return 1;
    fixture.record_count=fixture.observations=0u;
    lib_memory_copy(memory,fixture.memory,sizeof(memory));
    x86_core_cpu_execution_refresh(&fixture.bus.execution);
    if (fixture.bus.execution.stop_requested || fixture.bus.faults || fixture.bus.delivered_exceptions ||
        v30_stack_timing(&fixture,item) ||
        lib_memory_compare(&fixture.bus.cpu,&model.cpu,sizeof(model.cpu))!=0 ||
        v30_stack_records(&fixture,&model,memory,LIB_FALSE) ||
        lib_memory_compare(model.memory,fixture.memory,sizeof(model.memory))!=0) {
        lib_c_printf("V30:STACK-RESULT:%u:%04x:%u:reads=%u:writes=%u:stop=%u\n",
            item->operation,item->sp,item->level,fixture.reads,fixture.writes,fixture.bus.execution.stop_requested);
        return 1;
    }
    const lib_u32 read_parts=fixture.reads, write_parts=fixture.writes, fetch_parts=fixture.fetches;
    ++*cases;
    if (!failures) return 0;
    for (lib_u8 write=0u; write<2u; ++write)
        for (lib_u32 part=1u; part<=(write ? write_parts : read_parts); ++part) {
            v30_stack_prepare(&fixture,item);
            lib_memory_copy(memory,fixture.memory,sizeof(memory));
            if (write) fixture.fail_write=part;
            else fixture.fail_read=part;
            x86_core_cpu_execution_refresh(&fixture.bus.execution);
            if (!fixture.bus.execution.stop_requested || !fixture.bus.faults || fixture.bus.delivered_exceptions ||
                fixture.bus.execution.completed_observation_valid || fixture.bus.execution.timing_result.ticks ||
                fixture.bus.execution.timing_result.retirement_origin!=X86_CORE_CPU_RETIREMENT_TIMING_ORIGIN_UNATTRIBUTED ||
                lib_memory_compare(&fixture.bus.cpu,&before,sizeof(before))!=0 ||
                v30_stack_records(&fixture,&model,memory,LIB_TRUE)) {
                lib_c_printf("V30:STACK-FAILURE:%u:%04x:%u:%u:%u\n",
                    item->operation,item->sp,item->level,write,part);
                return 1;
            }
        }
    for (lib_u32 fetch=1u; fetch<=fetch_parts; ++fetch) {
        v30_stack_prepare(&fixture,item);
        fixture.fail_fetch=fetch;
        x86_core_cpu_execution_refresh(&fixture.bus.execution);
        if (!fixture.bus.execution.stop_requested || !fixture.bus.faults || fixture.reads || fixture.writes ||
            fixture.bus.execution.completed_observation_valid || fixture.bus.execution.timing_result.ticks ||
            fixture.bus.execution.timing_result.retirement_origin!=X86_CORE_CPU_RETIREMENT_TIMING_ORIGIN_UNATTRIBUTED ||
            lib_memory_compare(&fixture.bus.cpu,&before,sizeof(before))) {
            lib_c_printf("V30:NOMINAL-STACK:FETCH op=%u opcode=%02x sp=%04x level=%u fetch=%u reads=%u writes=%u stop=%u\n",
                item->operation,item->code[0],item->sp,item->level,fetch,fixture.reads,fixture.writes,
                fixture.bus.execution.stop_requested);
            return 1;
        }
    }
    return 0;
}

static lib_i32 v30_stack_scalars(lib_u32 *cases)
{
    static const lib_u16 stacks[]={0x700u,0x701u,0u,1u,0xffffu};
    static const lib_u8 prefixes[]={0u,0x26u,0x2eu,0x36u,0x3eu};
    for (lib_size stack=0u; stack<sizeof(stacks)/sizeof(stacks[0]); ++stack)
        for (lib_u8 reg=0u; reg<8u; ++reg)
            for (lib_u8 pop=0u; pop<2u; ++pop)
                for (lib_u8 alias=0u; alias<2u; ++alias) {
                    v30_stack_case item={.sp=stacks[stack],.bp=0x600u,.reg=reg,
                        .operation=pop ? V30_STACK_POP_REG : reg==4u && !alias ?
                            V30_STACK_PUSH_DECREMENTED_SP : V30_STACK_PUSH_REG};
                    item.code[0]=alias ? pop ? 0x8fu : 0xffu : (pop ? 0x58u : 0x50u)+reg;
                    item.bytes=alias ? 2u : 1u;
                    if (alias) item.code[1]=(pop ? 0xc0u : 0xf0u)+reg;
                    if (v30_stack_check(&item,LIB_TRUE,cases)) return 1;
                }
    for (lib_u16 value=0u; value<=255u; ++value)
        for (lib_u8 alignment=0u; alignment<2u; ++alignment) {
            v30_stack_case item={.operation=V30_STACK_PUSH_VALUE,.code={0x6au,(lib_u8)value},
                .bytes=2u,.sp=(lib_u16)(0x700u+alignment),.bp=0x600u,
                .value=(lib_u16)(value<128u ? value : value+0xff00u)};
            if (v30_stack_check(&item,LIB_TRUE,cases)) return 1;
        }
    for (lib_size stack=0u; stack<sizeof(stacks)/sizeof(stacks[0]); ++stack)
        for (lib_u8 prefix=0u; prefix<sizeof(prefixes); ++prefix)
            for (lib_u8 pop=0u; pop<2u; ++pop)
                for (lib_u8 location=0u; location<5u; ++location)
                    for (lib_u8 bp=0u; bp<2u; ++bp) {
                        const lib_u16 offsets[]={0x400u,0xffffu,stacks[stack],
                            (lib_u16)(stacks[stack]-1u),(lib_u16)(stacks[stack]+1u)};
                        const lib_u16 offset=offsets[location];
                        const lib_u32 bases[]={0x10000u,0x30000u,0x80000u,0x20000u,0x10000u};
                        v30_stack_case item={.operation=pop ? V30_STACK_POP_MEMORY : V30_STACK_PUSH_MEMORY,
                            .sp=stacks[stack],.bp=bp ? offset : 0x600u,
                            .address=(bp && !prefix ? 0x20000u : bases[prefix])+offset};
                        const lib_u8 extra=prefix!=0u;
                        if (extra) item.code[0]=prefixes[prefix];
                        item.code[extra]=pop ? 0x8fu : 0xffu;
                        item.code[extra+1u]=bp ? pop ? 0x46u : 0x76u : pop ? 6u : 0x36u;
                        item.code[extra+2u]=bp ? 0u : (lib_u8)offset;
                        if (!bp) item.code[extra+3u]=(lib_u8)(offset>>8u);
                        item.bytes=extra+(bp ? 3u : 4u);
                        if (v30_stack_check(&item,LIB_TRUE,cases)) return 1;
                    }
    static const lib_u16 values[]={0u,0x1234u,0x7fffu,0x8000u,0xffffu};
    for (lib_size stack=0u; stack<sizeof(stacks)/sizeof(stacks[0]); ++stack)
        for (lib_size value=0u; value<sizeof(values)/sizeof(values[0]); ++value) {
            v30_stack_case item={.operation=V30_STACK_PUSH_VALUE,
                .code={0x68u,(lib_u8)values[value],(lib_u8)(values[value]>>8u)},
                .bytes=3u,.sp=stacks[stack],.bp=0x600u,.value=values[value]};
            if (v30_stack_check(&item,LIB_TRUE,cases)) return 1;
        }
    return 0;
}

static lib_i32 v30_stack_sets_and_frames(lib_u32 *cases)
{
    static const lib_u16 stacks[]={0x700u,0x701u,0u,1u,0xffffu};
    static const lib_u8 prefixes[]={0u,0x26u,0x2eu,0x36u,0x3eu};
    for (lib_size stack=0u; stack<sizeof(stacks)/sizeof(stacks[0]); ++stack)
        for (lib_u8 prefix=0u; prefix<sizeof(prefixes); ++prefix)
            for (lib_u8 pop=0u; pop<2u; ++pop) {
                v30_stack_case item={.operation=pop ? V30_STACK_POP_ALL : V30_STACK_PUSH_ALL,
                    .sp=stacks[stack],.bp=0x600u};
                const lib_u8 extra=prefix!=0u;
                if (extra) item.code[0]=prefixes[prefix];
                item.code[extra]=pop ? 0x61u : 0x60u;
                item.bytes=extra+1u;
                if (v30_stack_check(&item,LIB_TRUE,cases)) return 1;
            }
    for (lib_u16 level=0u; level<=255u; ++level)
        for (lib_u8 alignment=0u; alignment<4u; ++alignment)
            for (lib_u8 overlap=0u; overlap<2u; ++overlap)
                for (lib_u8 allocation=0u; allocation<2u; ++allocation) {
                    /* Two independent parities; true overlap needs equal indices. */
                    if (overlap && alignment>=2u) continue;
                    v30_stack_case item={.operation=V30_STACK_ENTER,.code={0xc8u,0u,0u,(lib_u8)level},
                        .bytes=4u,.sp=(lib_u16)(0x700u+(alignment & 1u)),
                        .bp=(lib_u16)(overlap ? 0x700u+(alignment & 1u) : 0x600u+(alignment>>1u)),
                        .level=(lib_u8)level,.allocation=allocation ? 0xffffu : 0u};
                    item.code[1]=item.code[2]=allocation ? 0xffu : 0u;
                    if (v30_stack_check(&item,level==0u || level==1u || level==2u ||
                        level==31u || level==32u || level==255u,cases)) return 1;
                }
    static const lib_u8 levels[]={0u,1u,2u,3u,31u,32u,255u};
    for (lib_size stack=0u; stack<sizeof(stacks)/sizeof(stacks[0]); ++stack)
        for (lib_size level=0u; level<sizeof(levels); ++level) {
            v30_stack_case item={.operation=V30_STACK_ENTER,.code={0xc8u,0xffu,0x7fu,levels[level]},
                .bytes=4u,.sp=stacks[stack],.bp=stacks[stack],
                .level=levels[level],.allocation=0x7fffu};
            if (v30_stack_check(&item,LIB_TRUE,cases)) return 1;
        }
    for (lib_size stack=0u; stack<sizeof(stacks)/sizeof(stacks[0]); ++stack)
        for (lib_size frame=0u; frame<sizeof(stacks)/sizeof(stacks[0]); ++frame) {
            const v30_stack_case item={.operation=V30_STACK_LEAVE,.code={0xc9u},
                .bytes=1u,.sp=stacks[stack],.bp=stacks[frame]};
            if (v30_stack_check(&item,LIB_TRUE,cases)) return 1;
        }
    return 0;
}

lib_i32 main(void)
{
    lib_u32 cases=0u;
    const v30_stack_case popr={.operation=V30_STACK_POP_ALL,.code={0x61u},
        .bytes=1u,.sp=0x700u,.bp=0x600u};
    if (v30_stack_check(&popr,LIB_TRUE,&cases)) return 1;
    if (v30_stack_scalars(&cases)) return 1;
    if (v30_stack_sets_and_frames(&cases)) return 1;
    lib_c_printf("V30:STACK:OK\n");
    lib_c_printf("V30:NOMINAL-STACK:OK\n");
    lib_c_printf("V30:STACK-ALIGNMENT-MODEL:OK cases=%u\n",cases);
    return 0;
}
