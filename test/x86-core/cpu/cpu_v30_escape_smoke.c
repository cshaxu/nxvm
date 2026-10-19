#include "support/cpu_v30_memory_fixture.h"

typedef struct v30_escape_case {
    lib_u8 opcode, modrm, prefix;
    lib_bool odd, wrap;
} v30_escape_case;

static lib_u8 v30_escape_prepare(v30_memory_fixture *fixture,
    const v30_escape_case *item, v30_memory_plan *plan)
{
    static const lib_u16 offsets[]={0x340u,0x380u,0x540u,0x580u,0x40u,0x80u,0x500u,0x300u};
    lib_u8 code[6], size=0u;
    const lib_u8 mod=item->modrm>>6u, rm=item->modrm&7u;
    lib_u16 offset=offsets[rm];
    v30_memory_restart(fixture);
    fixture->bus.cpu.data.bx=0x300u;
    fixture->bus.cpu.data.bp=0x500u;
    fixture->bus.cpu.data.si=0x40u;
    fixture->bus.cpu.data.di=0x80u;
    fixture->bus.cpu.data.eip=item->wrap ? 0xfffdu : 0x100u;
    fixture->bus.cpu.data.eflags &= ~(VCPU_EFLAGS_IF|VCPU_EFLAGS_TF);
    if (item->prefix) code[size++]=item->prefix;
    code[size++]=item->opcode;
    code[size++]=item->modrm;
    if (mod==0u && rm==6u) {
        offset=item->odd ? 0xffffu : 0x400u;
        code[size++]=(lib_u8)offset;
        code[size++]=(lib_u8)(offset>>8u);
    } else if (mod==1u) {
        code[size++]=item->odd ? 0xffu : 0u;
        offset=(lib_u16)(offset-(item->odd ? 1u : 0u));
    } else if (mod==2u) {
        const lib_u16 displacement=item->odd ? 0xff01u : 0x100u;
        code[size++]=(lib_u8)displacement;
        code[size++]=(lib_u8)(displacement>>8u);
        offset=(lib_u16)(offset+displacement);
    }
    for (lib_u8 byte=0u; byte<size; ++byte)
        fixture->memory[0x80000u+(lib_u16)(fixture->bus.cpu.data.eip+byte)]=code[byte];
    *plan=(v30_memory_plan){0};
    if (mod!=3u) {
        const lib_u32 base=item->prefix==0x26u ? 0x30000u :
            item->prefix==0x2eu ? 0x80000u : item->prefix==0x36u ? 0x20000u :
            item->prefix==0x3eu ? 0x10000u :
            (rm==2u || rm==3u || (rm==6u && mod)) ? 0x20000u : 0x10000u;
        v30_memory_plan_transfer(plan,fixture,base+offset,2u,LIB_FALSE,0u);
    }
    return size;
}

static lib_i32 v30_escape_execute(const v30_escape_case *item, lib_bool deep)
{
    static v30_memory_fixture fixture;
    v30_memory_plan plan;
    const lib_u8 size=v30_escape_prepare(&fixture,item,&plan);
    const t_cpu before=fixture.bus.cpu;
    t_cpu expected=before;
    expected.data.eip=(lib_u16)(expected.data.eip+size);
    lib_u8 code[6];
    for (lib_u8 byte=0u; byte<size; ++byte)
        code[byte]=fixture.memory[0x80000u+(lib_u16)(before.data.eip+byte)];
    x86_core_cpu_instruction_lexeme lexeme;
    const lib_bool available=item->prefix!=0xf0u;
    if (x86_core_cpu_instruction_lexeme_scan(code,size,X86_CORE_CPU_PROFILE_V30,
            LIB_FALSE,&lexeme)!=available ||
        (available && lexeme.byte_count!=size)) return 1;
    for (lib_u8 length=0u; length<size; ++length)
        if (x86_core_cpu_instruction_lexeme_scan(code,length,X86_CORE_CPU_PROFILE_V30,
                LIB_FALSE,&lexeme)) return 1;
    if (x86_core_cpu_execution_preview_lexeme(&fixture.bus.execution,&lexeme)!=available ||
        (available && lexeme.byte_count!=size) || fixture.reads || fixture.writes ||
        fixture.fetches || lib_memory_compare(&before,&fixture.bus.cpu,sizeof(before))) return 1;
    const lib_u32 observations=fixture.observations;
    fixture.record_count=fixture.observations=0u;
    x86_core_cpu_execution_refresh(&fixture.bus.execution);
    x86_core_cpu_instruction_observation observation;
    x86_core_cpu_execution_copy_observation(&fixture.bus.execution,&observation);
    if (observation.nec_emulation_mode) return 1;
    const lib_bool memory=item->modrm<0xc0u;
    const lib_bool odd=memory && (plan.transfers[0].address&1u);
    const lib_u64 ticks=(memory ? odd ? 15u : 11u : 2u)+(item->prefix ? 2u : 0u);
    const lib_u32 inputs=(memory ? X86_CORE_CPU_TIMING_INPUT_MODRM |
        X86_CORE_CPU_TIMING_INPUT_EFFECTIVE_ADDRESS : 0u) |
        (odd ? X86_CORE_CPU_TIMING_INPUT_ODD_WORD : 0u) |
        (item->prefix==0xf0u ? X86_CORE_CPU_TIMING_INPUT_LOCK :
            item->prefix ? X86_CORE_CPU_TIMING_INPUT_SEGMENT_OVERRIDE : 0u);
    if (fixture.bus.execution.stop_requested || fixture.bus.faults || fixture.bus.delivered_exceptions ||
        !fixture.bus.execution.completed_observation_valid || fixture.observations ||
        lib_memory_compare(&fixture.bus.cpu,&expected,sizeof(expected)) ||
        v30_memory_check_plan(&fixture,&plan,LIB_FALSE) ||
        v30_memory_check_nominal(&fixture,ticks,item->opcode|(memory ? 0x100u : 0u),
            inputs,X86_CORE_CPU_RETIREMENT_TIMING_ORIGIN_V30_NOMINAL_SOURCE)) {
        lib_c_printf("V30:ESCAPE:RESULT op=%02x modrm=%02x prefix=%02x odd=%u wrap=%u reads=%u\n",
            item->opcode,item->modrm,item->prefix,item->odd,item->wrap,fixture.reads);
        return 1;
    }
    const lib_u32 fetches=fixture.fetches, reads=fixture.reads;
    if (deep) for (lib_u8 kind=0u; kind<3u; ++kind) {
        const lib_u32 parts=kind==0u ? observations : kind==1u ? fetches : reads;
        for (lib_u32 part=1u; part<=parts; ++part) {
            v30_escape_prepare(&fixture,item,&plan);
            if (kind==0u) {
                fixture.fail_observe=part;
                if (x86_core_cpu_execution_preview_lexeme(&fixture.bus.execution,&lexeme) ||
                    fixture.bus.execution.stop_requested || fixture.bus.faults ||
                    fixture.reads || fixture.writes || fixture.fetches ||
                    lib_memory_compare(&before,&fixture.bus.cpu,sizeof(before))) return 1;
            } else {
                if (kind==1u) fixture.fail_fetch=part;
                else fixture.fail_read=part;
                x86_core_cpu_execution_refresh(&fixture.bus.execution);
                if (!fixture.bus.execution.stop_requested || !fixture.bus.faults ||
                    fixture.bus.execution.completed_observation_valid ||
                    fixture.bus.execution.timing_result.ticks || fixture.writes ||
                    fixture.bus.instructions.data.memory_lock_active ||
                    lib_memory_compare(&before,&fixture.bus.cpu,sizeof(before)) ||
                    (kind==2u && v30_memory_check_plan(&fixture,&plan,LIB_TRUE))) return 1;
            }
        }
    }
    return 0;
}

static lib_i32 v30_escape_matrix(void)
{
    static const lib_u8 opcodes[]={0x66u,0x67u,0xd8u,0xd9u,0xdau,0xdbu,0xdcu,0xddu,0xdeu,0xdfu};
    static const lib_u8 prefixes[]={0u,0x26u,0x2eu,0x36u,0x3eu,0xf0u};
    lib_u32 count=0u;
    for (lib_size opcode=0u; opcode<sizeof(opcodes); ++opcode)
    for (lib_u16 modrm=0u; modrm<256u; ++modrm)
    for (lib_u8 layout=0u; layout<4u; ++layout)
    for (lib_size prefix=0u; prefix<sizeof(prefixes); ++prefix) {
        const v30_escape_case item={opcodes[opcode],(lib_u8)modrm,prefixes[prefix],
            (layout&1u)!=0u,(layout&2u)!=0u};
        const lib_bool deep=opcode<=2u && (prefix==0u || prefix==5u) &&
            (modrm==0x06u || modrm==0x46u || modrm==0x86u || modrm==0xc0u);
        if (v30_escape_execute(&item,deep)) return 1;
        ++count;
    }
    lib_c_printf("V30:ESCAPE:MATRIX:%u\n",count);
    return 0;
}

static lib_i32 v30_escape_profile_contrast(void)
{
    static const x86_core_cpu_profile profiles[]={X86_CORE_CPU_PROFILE_8086,
        X86_CORE_CPU_PROFILE_8088,X86_CORE_CPU_PROFILE_80186,X86_CORE_CPU_PROFILE_80286,
        X86_CORE_CPU_PROFILE_80386,X86_CORE_CPU_PROFILE_80486SX};
    for (lib_size profile=0u; profile<sizeof(profiles)/sizeof(profiles[0]); ++profile)
    for (lib_u8 opcode=0x66u; opcode<=0x67u; ++opcode) {
        const lib_u8 code[]={opcode,0x90u,0u,0u};
        x86_core_cpu_instruction_lexeme lexeme;
        const lib_bool intel_size=profile>=4u;
        if (x86_core_cpu_instruction_lexeme_scan(code,2u,profiles[profile],LIB_FALSE,&lexeme)!=intel_size ||
            (intel_size && lexeme.byte_count!=2u) ||
            x86_core_cpu_instruction_lexeme_scan(code,3u,X86_CORE_CPU_PROFILE_V30,
                LIB_FALSE,&lexeme) ||
            !x86_core_cpu_instruction_lexeme_scan(code,4u,X86_CORE_CPU_PROFILE_V30,
                LIB_FALSE,&lexeme) || lexeme.byte_count!=4u) return 1;
    }
    /* A native HALT has a known initial cost, not an emulation or idle clock. */
    for (lib_u8 prefix=0u; prefix<3u; ++prefix) {
        static v30_memory_fixture fixture;
        const lib_u8 code[]={prefix==1u ? 0x26u : 0xf0u,0xf4u};
        v30_memory_restart(&fixture);
        fixture.bus.cpu.data.eflags &= ~(VCPU_EFLAGS_IF|VCPU_EFLAGS_TF);
        lib_memory_copy(fixture.memory+0x80100u,code+(prefix ? 0u : 1u),prefix ? 2u : 1u);
        const t_cpu before=fixture.bus.cpu;
        t_cpu expected=before;
        expected.data.eip+=prefix ? 2u : 1u;
        expected.data.flagHalt=LIB_TRUE;
        x86_core_cpu_execution_refresh(&fixture.bus.execution);
        if (fixture.bus.execution.stop_requested || !fixture.bus.execution.completed_observation_valid ||
            lib_memory_compare(&expected,&fixture.bus.cpu,sizeof(expected)) || fixture.reads || fixture.writes ||
            v30_memory_check_nominal(&fixture,prefix ? 4u : 2u,0xf4u,prefix==1u ?
                X86_CORE_CPU_TIMING_INPUT_SEGMENT_OVERRIDE : prefix==2u ? X86_CORE_CPU_TIMING_INPUT_LOCK : 0u,
                X86_CORE_CPU_RETIREMENT_TIMING_ORIGIN_V30_NOMINAL_SOURCE)) return 1;
    }
    return 0;
}

lib_i32 main(void)
{
    if (v30_escape_matrix() || v30_escape_profile_contrast()) return 1;
    lib_c_printf("V30:ESCAPE:OK\n");
    return 0;
}
