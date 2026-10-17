#include "support/cpu_v30_memory_fixture.h"
#include "lib/types/file.h"

typedef enum v30_transfer_operation {
    V30_TRANSFER_REGISTER, V30_TRANSFER_LOAD, V30_TRANSFER_STORE,
    V30_TRANSFER_IMMEDIATE_REGISTER, V30_TRANSFER_IMMEDIATE_MEMORY,
    V30_TRANSFER_EXCHANGE_REGISTER, V30_TRANSFER_EXCHANGE_MEMORY,
    V30_TRANSFER_ADDRESS, V30_TRANSFER_TRANSLATE
} v30_transfer_operation;

typedef struct v30_transfer_case {
    v30_transfer_operation operation;
    lib_u8 code[8], length, width, reg, other, scenario, al;
    lib_u16 value, offset, bx;
    lib_u32 address;
} v30_transfer_case;

static lib_u16 v30_transfer_get(const t_cpu *cpu, lib_u8 reg, lib_u8 width)
{
    const lib_u16 words[]={cpu->data.ax,cpu->data.cx,cpu->data.dx,cpu->data.bx,
        cpu->data.sp,cpu->data.bp,cpu->data.si,cpu->data.di};
    return width==2u ? words[reg] : (lib_u8)(words[reg & 3u]>>(reg<4u ? 0u : 8u));
}

static void v30_transfer_set(t_cpu *cpu, lib_u8 reg, lib_u8 width, lib_u16 value)
{
    lib_u16 *words[]={&cpu->data.ax,&cpu->data.cx,&cpu->data.dx,&cpu->data.bx,
        &cpu->data.sp,&cpu->data.bp,&cpu->data.si,&cpu->data.di};
    if (width==2u) *words[reg]=value;
    else {
        const lib_u8 shift=reg<4u ? 0u : 8u;
        *words[reg & 3u]=(lib_u16)((*words[reg & 3u] & ~(0xffu<<shift)) |
            ((value & 0xffu)<<shift));
    }
}

static lib_bool v30_transfer_reads(const v30_transfer_case *item)
{
    return item->operation==V30_TRANSFER_LOAD || item->operation==V30_TRANSFER_EXCHANGE_MEMORY ||
        item->operation==V30_TRANSFER_TRANSLATE;
}

static lib_bool v30_transfer_writes(const v30_transfer_case *item)
{
    return item->operation==V30_TRANSFER_STORE || item->operation==V30_TRANSFER_EXCHANGE_MEMORY ||
        item->operation==V30_TRANSFER_IMMEDIATE_MEMORY;
}

static void v30_transfer_prepare(v30_memory_fixture *fixture, const v30_transfer_case *item)
{
    v30_memory_prepare(fixture);
    fixture->bus.cpu.data.eax=0x11119a78u;
    fixture->bus.cpu.data.ecx=0x22222345u;
    fixture->bus.cpu.data.edx=0x33333456u;
    fixture->bus.cpu.data.ebx=0x4444fff0u+(item->scenario==1u);
    fixture->bus.cpu.data.esp=0x55550700u;
    fixture->bus.cpu.data.ebp=0x6666fff0u+(item->scenario==1u);
    fixture->bus.cpu.data.esi=0x77770020u+(item->scenario==2u);
    fixture->bus.cpu.data.edi=0x88880030u+(item->scenario==2u);
    if (item->scenario==1u) fixture->bus.cpu.data.eflags |= VCPU_EFLAGS_CF | VCPU_EFLAGS_PF |
        VCPU_EFLAGS_AF | VCPU_EFLAGS_ZF | VCPU_EFLAGS_SF | VCPU_EFLAGS_DF | VCPU_EFLAGS_OF;
    if (item->operation==V30_TRANSFER_TRANSLATE) {
        fixture->bus.cpu.data.bx=item->bx;
        fixture->bus.cpu.data.al=item->al;
    }
    if (v30_transfer_reads(item) || v30_transfer_writes(item)) {
        const lib_u16 initial=v30_transfer_reads(item) ? item->value : 0xc35au;
        fixture->memory[item->address]=(lib_u8)initial;
        fixture->memory[item->address+1u]=(lib_u8)(initial>>8u);
    }
    /* Code may deliberately alias a CS-selected data operand. */
    lib_memory_copy(fixture->memory+0x80100u,item->code,item->length);
}

/* Explicit input operations select results; production metadata is not an oracle. */
static lib_u16 v30_transfer_expect(t_cpu *after, const v30_transfer_case *item, lib_u16 operand)
{
    const lib_u16 original=v30_transfer_get(after,item->reg,item->width);
    switch (item->operation) {
    case V30_TRANSFER_REGISTER:
        v30_transfer_set(after,item->reg,item->width,v30_transfer_get(after,item->other,item->width));
        break;
    case V30_TRANSFER_LOAD: case V30_TRANSFER_TRANSLATE:
        v30_transfer_set(after,item->reg,item->width,operand);
        break;
    case V30_TRANSFER_STORE: break;
    case V30_TRANSFER_IMMEDIATE_REGISTER:
        v30_transfer_set(after,item->reg,item->width,item->value);
        break;
    case V30_TRANSFER_IMMEDIATE_MEMORY: break;
    case V30_TRANSFER_EXCHANGE_REGISTER:
        v30_transfer_set(after,item->reg,item->width,v30_transfer_get(after,item->other,item->width));
        v30_transfer_set(after,item->other,item->width,original);
        break;
    case V30_TRANSFER_EXCHANGE_MEMORY:
        v30_transfer_set(after,item->reg,item->width,operand);
        break;
    case V30_TRANSFER_ADDRESS: v30_transfer_set(after,item->reg,2u,item->offset); break;
    }
    after->data.eip+=item->length;
    return item->operation==V30_TRANSFER_IMMEDIATE_MEMORY ? item->value : original;
}

static lib_i32 v30_transfer_records(const v30_memory_fixture *fixture,
    const v30_transfer_case *item, const lib_u8 *initial, lib_u16 value, lib_bool failed)
{
    lib_u8 expected[]={initial[0],initial[1]};
    lib_u32 read_bytes=0u, write_bytes=0u, parts=0u, reads=0u, writes=0u;
    lib_bool refused=LIB_FALSE;
    for (lib_u32 index=0u; index<fixture->record_count; ++index) {
        const v30_memory_record *record=&fixture->records[index];
        if (record->observe || record->provenance!=X86_CORE_CPU_MEMORY_ACCESS_DATA) continue;
        const lib_bool write=!v30_transfer_reads(item) || read_bytes==item->width;
        const lib_u32 offset=write ? write_bytes : read_bytes;
        const lib_u8 width=item->width==2u && offset==0u && !(item->address & 1u) ? 2u : 1u;
        const lib_u32 ordinal=write ? ++writes : ++reads;
        const lib_bool accepted=ordinal!=(write ? fixture->fail_write : fixture->fail_read);
        if (refused || offset>=item->width || (write && !v30_transfer_writes(item)) ||
            record->write!=write || record->address!=item->address+offset || record->bytes!=width ||
            record->accepted!=accepted || fixture->phases[2u*parts]!=X86_CORE_CPU_EXTERNAL_CYCLE_PHASE_BEGIN ||
            fixture->phases[2u*parts+1u]!=(accepted ? X86_CORE_CPU_EXTERNAL_CYCLE_PHASE_COMMIT :
                X86_CORE_CPU_EXTERNAL_CYCLE_PHASE_CANCEL)) return 1;
        if (write && accepted)
            for (lib_u8 byte=0u; byte<width; ++byte)
                expected[offset+byte]=(lib_u8)(value>>((offset+byte)*8u));
        if (write) write_bytes+=width;
        else read_bytes+=width;
        refused=!accepted;
        ++parts;
    }
    return fixture->invalid || fixture->phase_count!=2u*parts || fixture->reads!=reads ||
        fixture->writes!=writes || refused!=failed || (!failed &&
            (read_bytes!=(v30_transfer_reads(item) ? item->width : 0u) ||
             write_bytes!=(v30_transfer_writes(item) ? item->width : 0u))) ||
        lib_memory_compare(fixture->memory+item->address,expected,sizeof(expected))!=0;
}

static lib_i32 v30_transfer_check(const v30_transfer_case *item)
{
    static v30_memory_fixture fixture;
    x86_core_cpu_instruction_lexeme lexeme;
    v30_transfer_prepare(&fixture,item);
    const t_cpu before=fixture.bus.cpu;
    const lib_u8 initial[]={fixture.memory[item->address],fixture.memory[item->address+1u]};
    const lib_u16 operand=(lib_u16)(initial[0]+256u*initial[1]);
    t_cpu after=before;
    const lib_u16 value=v30_transfer_expect(&after,item,operand);
    if (!x86_core_cpu_execution_preview_lexeme(&fixture.bus.execution,&lexeme) ||
        lexeme.byte_count!=item->length || fixture.reads || fixture.writes || fixture.phase_count ||
        lib_memory_compare(&fixture.bus.cpu,&before,sizeof(before))!=0) return 1;
    fixture.record_count=fixture.observations=0u;
    x86_core_cpu_execution_refresh(&fixture.bus.execution);
    if (fixture.bus.execution.stop_requested || fixture.bus.faults || fixture.bus.delivered_exceptions ||
        fixture.bus.execution.interrupt_shadow!=CPU_INTERRUPT_SHADOW_NONE ||
        lib_memory_compare(&fixture.bus.cpu,&after,sizeof(after))!=0 ||
        v30_transfer_records(&fixture,item,initial,value,LIB_FALSE)) {
        lib_c_printf("V30:TRANSFER-RESULT:%u:%02x:%u:%u:%05x\n",
            item->operation,item->code[0],item->reg,item->other,item->address);
        return 1;
    }
    const lib_u32 reads=fixture.reads, writes=fixture.writes;
    for (lib_u8 write=0u; write<2u; ++write)
        for (lib_u32 part=1u; part<=(write ? writes : reads); ++part) {
            v30_transfer_prepare(&fixture,item);
            if (write) fixture.fail_write=part;
            else fixture.fail_read=part;
            x86_core_cpu_execution_refresh(&fixture.bus.execution);
            if (!fixture.bus.execution.stop_requested || !fixture.bus.faults || fixture.bus.delivered_exceptions ||
                fixture.bus.execution.interrupt_shadow!=CPU_INTERRUPT_SHADOW_NONE ||
                lib_memory_compare(&fixture.bus.cpu,&before,sizeof(before))!=0 ||
                v30_transfer_records(&fixture,item,initial,value,LIB_TRUE)) {
                lib_c_printf("V30:TRANSFER-FAILURE:%u:%02x:%05x:%u:%u\n",
                    item->operation,item->code[0],item->address,write,part);
                return 1;
            }
        }
    return 0;
}

static v30_transfer_case v30_transfer_memory(v30_transfer_operation operation,
    lib_u8 opcode, lib_u8 width, lib_u8 modrm, lib_u8 prefix, lib_u8 scenario, lib_u16 value)
{
    static const lib_u8 prefixes[]={0u,0x26u,0x2eu,0x36u,0x3eu};
    const lib_u8 mod=modrm>>6u, rm=modrm & 7u, extra=prefix!=0u;
    const lib_u16 bx=(lib_u16)(0xfff0u+(scenario==1u)), bp=bx;
    const lib_u16 si=(lib_u16)(0x20u+(scenario==2u)), di=(lib_u16)(0x30u+(scenario==2u));
    const lib_u32 offsets[]={bx+si,bx+di,bp+si,bp+di,si,di,bp,bx};
    static const lib_u16 direct[]={0x400u,0x401u,0xffffu};
    static const lib_u16 short_displacements[]={0x7fu,0xff80u,0xffffu};
    static const lib_u16 long_displacements[]={0x7fffu,0x8000u,0xffffu};
    const lib_u16 displacement=mod==0u ? direct[scenario] :
        mod==1u ? short_displacements[scenario] : long_displacements[scenario];
    const lib_u16 offset=(lib_u16)(mod==0u && rm==6u ? displacement :
        offsets[rm]+(mod ? displacement : 0u));
    const lib_bool stack=rm==2u || rm==3u || (rm==6u && mod!=0u);
    const lib_u32 bases[]={stack ? 0x20000u : 0x10000u,0x30000u,0x80000u,0x20000u,0x10000u};
    v30_transfer_case item={.operation=operation,.width=width,.reg=(modrm>>3u) & 7u,
        .scenario=scenario,.value=value,.offset=offset,.address=bases[prefix]+offset};
    if (extra) item.code[0]=prefixes[prefix];
    item.code[extra]=opcode;
    item.code[extra+1u]=modrm;
    item.length=extra+2u;
    if (mod || rm==6u) {
        item.code[item.length++]=(lib_u8)displacement;
        if (mod!=1u) item.code[item.length++]=(lib_u8)(displacement>>8u);
    }
    if (operation==V30_TRANSFER_IMMEDIATE_MEMORY) {
        item.code[item.length++]=(lib_u8)value;
        if (width==2u) item.code[item.length++]=(lib_u8)(value>>8u);
    }
    return item;
}

static lib_i32 v30_transfer_modrm(void)
{
    static const lib_u8 opcodes[]={0x88u,0x89u,0x8au,0x8bu,0x86u,0x87u};
    static const lib_u8 prefixes[]={0u,0x26u,0x2eu,0x36u,0x3eu};
    for (lib_u8 form=0u; form<sizeof(opcodes); ++form)
        for (lib_u16 modrm=0u; modrm<=255u; ++modrm)
            for (lib_u8 prefix=0u; prefix<sizeof(prefixes); ++prefix)
                for (lib_u8 scenario=0u; scenario<(modrm<0xc0u ? 3u : 2u); ++scenario) {
                    const lib_u8 width=(form & 1u)+1u, reg=(modrm>>3u) & 7u, rm=modrm & 7u;
                    const v30_transfer_operation operation=form>=4u ? V30_TRANSFER_EXCHANGE_MEMORY :
                        form>=2u ? V30_TRANSFER_LOAD : V30_TRANSFER_STORE;
                    v30_transfer_case item=v30_transfer_memory(operation,opcodes[form],width,
                        (lib_u8)modrm,prefix,scenario,(lib_u16)(0x5aa5u ^ scenario));
                    if (modrm>=0xc0u) {
                        item.operation=form>=4u ? V30_TRANSFER_EXCHANGE_REGISTER : V30_TRANSFER_REGISTER;
                        item.reg=form>=2u ? reg : rm;
                        item.other=form>=2u ? rm : reg;
                        item.length=(prefix!=0u)+2u;
                    }
                    if (v30_transfer_check(&item)) return 1;
                }
    for (lib_u8 prefix=0u; prefix<sizeof(prefixes); ++prefix)
        for (lib_u16 modrm=0u; modrm<0xc0u; ++modrm)
            for (lib_u8 scenario=0u; scenario<3u; ++scenario) {
                const v30_transfer_case item=v30_transfer_memory(V30_TRANSFER_ADDRESS,0x8du,2u,
                    (lib_u8)modrm,prefix,scenario,0u);
                if (v30_transfer_check(&item)) return 1;
            }
    for (lib_u8 reg=0u; reg<8u; ++reg)
        for (lib_u8 prefix=0u; prefix<sizeof(prefixes); ++prefix)
            for (lib_u8 scenario=0u; scenario<2u; ++scenario) {
                const lib_u8 extra=prefix!=0u;
                v30_transfer_case item={.operation=V30_TRANSFER_EXCHANGE_REGISTER,
                    .width=2u,.reg=0u,.other=reg,.scenario=scenario,.length=extra+1u};
                if (extra) item.code[0]=prefixes[prefix];
                item.code[extra]=0x90u+reg;
                if (v30_transfer_check(&item)) return 1;
            }
    return 0;
}

static lib_i32 v30_transfer_immediates(void)
{
    static const lib_u8 prefixes[]={0u,0x26u,0x2eu,0x36u,0x3eu};
    static const lib_u16 words[]={0u,1u,0x1234u,0x7fffu,0x8000u,0xffffu};
    for (lib_u8 width=1u; width<=2u; ++width)
        for (lib_u8 reg=0u; reg<8u; ++reg)
            for (lib_u16 value=0u; value<(width==1u ? 256u : sizeof(words)/sizeof(words[0])); ++value)
                for (lib_u8 prefix=0u; prefix<sizeof(prefixes); ++prefix)
                    for (lib_u8 alias=0u; alias<2u; ++alias) {
                        const lib_u8 extra=prefix!=0u;
                        v30_transfer_case item={.operation=V30_TRANSFER_IMMEDIATE_REGISTER,
                            .width=width,.reg=reg,.scenario=(lib_u8)(value & 1u),
                            .value=width==1u ? value : words[value]};
                        if (extra) item.code[0]=prefixes[prefix];
                        item.code[extra]=alias ? 0xc5u+width : (width==1u ? 0xb0u : 0xb8u)+reg;
                        item.length=extra+1u;
                        if (alias) item.code[item.length++]=0xc0u+reg;
                        item.code[item.length++]=(lib_u8)item.value;
                        if (width==2u) item.code[item.length++]=(lib_u8)(item.value>>8u);
                        if (v30_transfer_check(&item)) return 1;
                    }
    for (lib_u8 width=1u; width<=2u; ++width)
        for (lib_u8 mod=0u; mod<3u; ++mod)
            for (lib_u8 rm=0u; rm<8u; ++rm)
                for (lib_u8 prefix=0u; prefix<sizeof(prefixes); ++prefix)
                    for (lib_u8 scenario=0u; scenario<3u; ++scenario)
                        for (lib_u8 value=0u; value<sizeof(words)/sizeof(words[0]); ++value) {
                            const v30_transfer_case item=v30_transfer_memory(V30_TRANSFER_IMMEDIATE_MEMORY,
                                0xc5u+width,width,(mod<<6u)+rm,prefix,scenario,words[value]);
                            if (v30_transfer_check(&item)) return 1;
                        }
    return 0;
}

static lib_i32 v30_transfer_direct_and_translate(void)
{
    static const lib_u8 prefixes[]={0u,0x26u,0x2eu,0x36u,0x3eu};
    static const lib_u16 offsets[]={0u,1u,0x400u,0x401u,0xfffeu,0xffffu};
    static const lib_u16 values[]={0u,1u,0x1234u,0x7fffu,0x8000u,0xffffu};
    static const lib_u32 bases[]={0x10000u,0x30000u,0x80000u,0x20000u,0x10000u};
    for (lib_u8 opcode=0xa0u; opcode<=0xa3u; ++opcode)
        for (lib_u8 prefix=0u; prefix<sizeof(prefixes); ++prefix)
            for (lib_u8 offset=0u; offset<sizeof(offsets)/sizeof(offsets[0]); ++offset)
                for (lib_u8 value=0u; value<sizeof(values)/sizeof(values[0]); ++value) {
                    const lib_u8 extra=prefix!=0u;
                    v30_transfer_case item={.operation=opcode<0xa2u ? V30_TRANSFER_LOAD : V30_TRANSFER_STORE,
                        .width=(opcode & 1u)+1u,.scenario=value & 1u,.value=values[value],
                        .address=bases[prefix]+offsets[offset],.length=extra+3u};
                    if (extra) item.code[0]=prefixes[prefix];
                    item.code[extra]=opcode;
                    item.code[extra+1u]=(lib_u8)offsets[offset];
                    item.code[extra+2u]=(lib_u8)(offsets[offset]>>8u);
                    if (v30_transfer_check(&item)) return 1;
                }
    static const lib_u16 tables[]={0u,1u,0xff00u,0xffffu};
    for (lib_u8 prefix=0u; prefix<sizeof(prefixes); ++prefix)
        for (lib_u8 table=0u; table<sizeof(tables)/sizeof(tables[0]); ++table)
            for (lib_u16 al=0u; al<=255u; ++al) {
                const lib_u8 extra=prefix!=0u;
                v30_transfer_case item={.operation=V30_TRANSFER_TRANSLATE,.width=1u,
                    .bx=tables[table],.al=(lib_u8)al,.value=(lib_u16)(al ^ (tables[table]>>8u) ^ 0xa5u),
                    .scenario=al & 1u,.address=bases[prefix]+(lib_u16)(tables[table]+al),.length=extra+1u};
                if (extra) item.code[0]=prefixes[prefix];
                item.code[extra]=0xd7u;
                if (v30_transfer_check(&item)) return 1;
            }
    return 0;
}

lib_i32 main(void)
{
    if (v30_transfer_modrm()) return 1;
    if (v30_transfer_immediates()) return 1;
    if (v30_transfer_direct_and_translate()) return 1;
    lib_c_printf("V30:TRANSFER:OK\n");
    return 0;
}
