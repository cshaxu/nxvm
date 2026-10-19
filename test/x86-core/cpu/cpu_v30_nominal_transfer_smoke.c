#include "support/cpu_v30_memory_fixture.h"

typedef struct v30_nominal_case {
    lib_u8 opcode, register_ticks, memory_ticks, odd_ticks, width;
} v30_nominal_case;

/* Independent NEC 12-4..12-13 columns: register, aligned memory, odd word.
 * A zero register column has no documented nominal row for that encoding. */
static const v30_nominal_case v30_nominal_cases[] = {
    {0x88u,2u,9u,0u,1u}, {0x89u,2u,9u,4u,2u},
    {0x8au,2u,11u,0u,1u}, {0x8bu,2u,11u,4u,2u},
    {0x8cu,2u,10u,4u,2u}, {0x8eu,2u,11u,4u,2u},
    {0xc6u,0u,11u,0u,1u}, {0xc7u,0u,11u,4u,2u},
    {0xc4u,0u,18u,8u,4u}, {0xc5u,0u,18u,8u,4u},
    {0x8du,0u,4u,0u,0u},
    {0x86u,3u,16u,0u,1u}, {0x87u,3u,16u,8u,2u}
};

static void v30_nominal_prepare(v30_memory_fixture *fixture)
{
    v30_memory_restart(fixture);
    fixture->bus.cpu.data.bx=0x300u;
    fixture->bus.cpu.data.bp=0x500u;
    fixture->bus.cpu.data.si=0x40u;
    fixture->bus.cpu.data.di=0x80u;
}

static lib_i32 v30_nominal_result(const v30_memory_fixture *fixture,
    lib_u64 ticks, lib_u32 form, lib_bool odd)
{
    const x86_core_cpu_timing_result result=x86_core_cpu_capture_timing(&fixture->bus.execution);
    const lib_bool allocated=ticks != 0u;
    const lib_u32 expected_form=allocated ? form : X86_CORE_CPU_RETIREMENT_SOURCE_FORM_UNATTRIBUTED;
    if (!result.source_timing_unallocated || result.ticks != ticks || result.event_ticks ||
        result.key_id != expected_form || result.form_id != expected_form ||
        result.retirement_origin != (allocated ?
            X86_CORE_CPU_RETIREMENT_TIMING_ORIGIN_V30_NOMINAL_SOURCE :
            X86_CORE_CPU_RETIREMENT_TIMING_ORIGIN_UNATTRIBUTED) ||
        ((result.formula_inputs & X86_CORE_CPU_TIMING_INPUT_ODD_WORD) != 0u) != odd ||
        ((result.formula_inputs & X86_CORE_CPU_TIMING_INPUT_MODRM) != 0u) !=
            (allocated && (form & 0x100u) != 0u) ||
        x86_core_cpu_timing_maximum_ticks(X86_CORE_CPU_PROFILE_V30,LIB_NULL)) {
        lib_c_printf("V30:NOMINAL-TRANSFER:FAIL opcode=%02x ticks=%llu/%llu origin=%u form=%x/%x inputs=%x\n",
            fixture->bus.instructions.data.opcodes[0],result.ticks,ticks,
            (lib_u32)result.retirement_origin,result.form_id,expected_form,result.formula_inputs);
        return 1;
    }
    return 0;
}

static lib_i32 v30_nominal_modrm(lib_u32 *count)
{
    static v30_memory_fixture fixture;
    static const lib_u8 prefixes[]={0u,0x26u,0x2eu,0x36u,0x3eu};
    static const lib_u16 offsets[]={0x340u,0x380u,0x540u,0x580u,0x40u,0x80u,0x500u,0x300u};
    v30_memory_prepare(&fixture);
    for (lib_size item=0u; item < sizeof(v30_nominal_cases)/sizeof(v30_nominal_cases[0]); ++item)
        for (lib_u16 modrm=0u; modrm < 256u; ++modrm)
            for (lib_u8 parity=0u; parity < 2u; ++parity)
                for (lib_size prefix=0u; prefix < sizeof(prefixes); ++prefix) {
                    const v30_nominal_case *row=&v30_nominal_cases[item];
                    const lib_u8 mod=(lib_u8)(modrm>>6u), reg=(lib_u8)((modrm>>3u)&7u), rm=modrm&7u;
                    const lib_bool memory=mod != 3u;
                    if ((row->opcode==0xc6u || row->opcode==0xc7u) && reg) continue;
                    if ((row->opcode==0x8cu || row->opcode==0x8eu) &&
                        (reg>3u || (row->opcode==0x8eu && reg==1u))) continue;
                    if (!memory && (row->opcode==0xc4u || row->opcode==0xc5u || row->opcode==0x8du)) continue;
                    v30_nominal_prepare(&fixture);
                    lib_u8 code[8], size=0u;
                    lib_u16 offset=offsets[rm];
                    if (prefixes[prefix]) code[size++]=prefixes[prefix];
                    code[size++]=row->opcode;
                    code[size++]=(lib_u8)modrm;
                    if (mod==0u && rm==6u) {
                        offset=parity ? 0xffffu : 0x400u;
                        code[size++]=(lib_u8)offset;
                        code[size++]=(lib_u8)(offset>>8u);
                    } else if (mod==1u) {
                        code[size++]=parity ? 0xffu : 0u;
                        offset=(lib_u16)(offset-(parity ? 1u : 0u));
                    } else if (mod==2u) {
                        const lib_u16 displacement=parity ? 0xff01u : 0x100u;
                        code[size++]=(lib_u8)displacement;
                        code[size++]=(lib_u8)(displacement>>8u);
                        offset=(lib_u16)(offset+displacement);
                    }
                    const lib_u32 base=prefix==1u ? 0x30000u : prefix==2u ? 0x80000u :
                        prefix==3u ? 0x20000u : prefix==4u ? 0x10000u :
                        (rm==2u || rm==3u || (rm==6u && mod)) ? 0x20000u : 0x10000u;
                    if (memory) lib_memory_copy(fixture.memory+base+offset,
                        (const lib_u8[]){0x78u,0x56u,0x21u,0x43u},4u);
                    if (row->opcode==0xc6u || row->opcode==0xc7u) {
                        code[size++]=0x35u;
                        if (row->width==2u) code[size++]=0x79u;
                    }
                    lib_memory_copy(fixture.memory+0x80100u,code,size);
                    x86_core_cpu_execution_refresh(&fixture.bus.execution);
                    lib_u64 ticks=memory ? row->memory_ticks : row->register_ticks;
                    const lib_bool odd=memory && (offset&1u) && row->odd_ticks;
                    if (odd) ticks+=row->odd_ticks;
                    if (ticks && prefix) ticks+=2u;
                    if (fixture.bus.execution.stop_requested || fixture.bus.faults || fixture.invalid ||
                        !fixture.bus.execution.completed_observation_valid ||
                        v30_nominal_result(&fixture,ticks,row->opcode | (memory ? 0x100u : 0u),
                            odd)) return 1;
                    ++*count;
                }
    return 0;
}

static lib_i32 v30_nominal_direct(lib_u32 *count)
{
    static v30_memory_fixture fixture;
    static const lib_u16 offsets[]={0x400u,0x401u,0xfffeu,0xffffu};
    v30_memory_prepare(&fixture);
    for (lib_u16 opcode=0u; opcode < 256u; ++opcode) {
        if (!((opcode>=0xa0u && opcode<=0xa3u) || (opcode>=0xb0u && opcode<=0xbfu) ||
            (opcode>=0x90u && opcode<=0x97u) || opcode==0x9eu || opcode==0x9fu || opcode==0xd7u)) continue;
        for (lib_size address=0u; address < sizeof(offsets)/sizeof(offsets[0]); ++address) {
            v30_nominal_prepare(&fixture);
            const lib_u8 code[]={(lib_u8)opcode,(lib_u8)offsets[address],(lib_u8)(offsets[address]>>8u)};
            lib_memory_copy(fixture.memory+0x80100u,code,sizeof(code));
            fixture.memory[0x10000u+offsets[address]]=0x56u;
            fixture.memory[0x10001u+offsets[address]]=0x78u;
            x86_core_cpu_instruction_lexeme lexeme;
            const x86_core_cpu_timing_result before=x86_core_cpu_capture_timing(&fixture.bus.execution);
            if (!x86_core_cpu_execution_preview_lexeme(&fixture.bus.execution,&lexeme) || !lexeme.available ||
                lib_memory_compare(&before,&fixture.bus.execution.timing_result,sizeof(before)) ||
                fixture.reads || fixture.writes) return 1;
            x86_core_cpu_execution_refresh(&fixture.bus.execution);
            lib_u64 ticks=opcode>=0xb0u && opcode<=0xbfu ? 4u : opcode==0x9fu ? 2u :
                opcode==0xd7u ? 9u : 3u;
            const lib_bool direct=opcode>=0xa0u && opcode<=0xa3u;
            const lib_bool odd=direct && (opcode&1u) && (offsets[address]&1u);
            if (direct) ticks=(opcode&2u ? 9u : 10u)+(odd ? 4u : 0u);
            if (fixture.bus.execution.stop_requested || fixture.bus.faults || fixture.invalid ||
                !fixture.bus.execution.completed_observation_valid ||
                v30_nominal_result(&fixture,ticks,opcode,odd)) return 1;
            ++*count;
        }
    }
    return 0;
}

static lib_i32 v30_nominal_boundaries(lib_u32 *count)
{
    static v30_memory_fixture fixture;
    static const struct {lib_u8 code[5], ticks;} cases[]={
        {{0xf2u,0x89u,0xc0u},0u}, {{0xf3u,0x90u},0u},
        {{0x26u,0x27u},5u}, {{0x26u,0x2fu},9u}
    };
    v30_memory_prepare(&fixture);
    for (lib_size item=0u; item < sizeof(cases)/sizeof(cases[0]); ++item) {
        v30_nominal_prepare(&fixture);
        lib_memory_copy(fixture.memory+0x80100u,cases[item].code,sizeof(cases[item].code));
        x86_core_cpu_execution_refresh(&fixture.bus.execution);
        if (fixture.bus.execution.stop_requested || fixture.bus.faults ||
            v30_nominal_result(&fixture,cases[item].ticks,cases[item].code[1],LIB_FALSE)) {
            lib_c_printf("V30:NOMINAL-TRANSFER:EXCLUDED item=%u stop=%u faults=%u\n",
                (lib_u32)item,fixture.bus.execution.stop_requested,fixture.bus.faults);
            return 1;
        }
        ++*count;
    }
    /* A live mode-entry instruction, not changing an internal mode mirror. */
    v30_nominal_prepare(&fixture);
    lib_memory_copy(fixture.memory+0x80100u,(const lib_u8[]){0x0fu,0xffu,0x40u},3u);
    lib_memory_copy(fixture.memory+0x100u,(const lib_u8[]){0x00u,0x02u,0x00u,0x80u},4u);
    fixture.memory[0x80200u]=0x90u; /* 8080 SUB B, not native XCH AW,AW. */
    x86_core_cpu_execution_refresh(&fixture.bus.execution);
    x86_core_cpu_execution_refresh(&fixture.bus.execution);
    if (fixture.bus.execution.stop_requested || fixture.bus.faults ||
        fixture.bus.cpu.data.eflags & X86_CPU_NEC_PSW_MD ||
        v30_nominal_result(&fixture,0u,0u,LIB_FALSE)) {
        lib_c_printf("V30:NOMINAL-TRANSFER:MODE stop=%u fault=%u pc=%x cs=%x flags=%x\n",
            fixture.bus.execution.stop_requested,fixture.bus.faults,fixture.bus.cpu.data.eip,
            fixture.bus.cpu.data.cs.selector,fixture.bus.cpu.data.eflags);
        return 1;
    }
    ++*count;

    static const lib_u8 transfers[][4]={
        {0x8bu,0x07u}, {0x89u,0x07u}, {0x87u,0x07u}, {0xc5u,0x07u}
    };
    for (lib_size item=0u; item < sizeof(transfers)/sizeof(transfers[0]); ++item) {
        v30_nominal_prepare(&fixture);
        lib_memory_copy(fixture.memory+0x80100u,transfers[item],4u);
        x86_core_cpu_execution_refresh(&fixture.bus.execution);
        if (fixture.bus.execution.stop_requested || !fixture.bus.execution.timing_result.ticks) {
            lib_c_printf("V30:NOMINAL-TRANSFER:BASELINE item=%u stop=%u ticks=%llu\n",
                (lib_u32)item,fixture.bus.execution.stop_requested,fixture.bus.execution.timing_result.ticks);
            return 1;
        }
        const lib_u32 counts[]={fixture.fetches,fixture.reads,fixture.writes};
        for (lib_u8 kind=0u; kind < 3u; ++kind)
            for (lib_u32 part=1u; part <= counts[kind]; ++part) {
                v30_nominal_prepare(&fixture);
                lib_memory_copy(fixture.memory+0x80100u,transfers[item],4u);
                if (!kind) fixture.fail_fetch=part;
                else if (kind==1u) fixture.fail_read=part;
                else fixture.fail_write=part;
                x86_core_cpu_execution_refresh(&fixture.bus.execution);
                if (!fixture.bus.execution.stop_requested || fixture.bus.faults!=1u ||
                    fixture.bus.execution.completed_observation_valid ||
                    fixture.bus.execution.timing_result.ticks ||
                    fixture.bus.execution.timing_result.retirement_origin!=X86_CORE_CPU_RETIREMENT_TIMING_ORIGIN_UNATTRIBUTED)
                {
                    lib_c_printf("V30:NOMINAL-TRANSFER:REFUSAL item=%u kind=%u part=%u stop=%u faults=%u complete=%u ticks=%llu origin=%u\n",
                        (lib_u32)item,kind,part,fixture.bus.execution.stop_requested,fixture.bus.faults,
                        fixture.bus.execution.completed_observation_valid,fixture.bus.execution.timing_result.ticks,
                        (lib_u32)fixture.bus.execution.timing_result.retirement_origin);
                    return 1;
                }
                ++*count;
            }
    }
    /* Reuse one live CPU: neither an unallocated next instruction nor a host
     * failure may inherit the preceding successful nominal publication. */
    v30_nominal_prepare(&fixture);
    lib_memory_copy(fixture.memory+0x80100u,
        (const lib_u8[]){0xb0u,0x55u,0xf3u,0x90u,0x9fu,0x8bu,0x07u},7u);
    const lib_u64 clocks[]={4u,0u,2u};
    const lib_u32 forms[]={0xb0u,0u,0x9fu};
    for (lib_u8 step=0u; step < 3u; ++step) {
        x86_core_cpu_execution_refresh(&fixture.bus.execution);
        if (fixture.bus.execution.stop_requested ||
            v30_nominal_result(&fixture,clocks[step],forms[step],LIB_FALSE)) return 1;
        ++*count;
    }
    fixture.fail_read=fixture.reads+1u;
    x86_core_cpu_execution_refresh(&fixture.bus.execution);
    if (!fixture.bus.execution.stop_requested || fixture.bus.faults!=1u ||
        fixture.bus.execution.completed_observation_valid ||
        fixture.bus.execution.timing_result.ticks ||
        fixture.bus.execution.timing_result.retirement_origin!=X86_CORE_CPU_RETIREMENT_TIMING_ORIGIN_UNATTRIBUTED)
        return 1;
    ++*count;

    static const x86_core_cpu_profile intel[]={X86_CORE_CPU_PROFILE_8086,X86_CORE_CPU_PROFILE_8088,
        X86_CORE_CPU_PROFILE_80186,X86_CORE_CPU_PROFILE_80286,X86_CORE_CPU_PROFILE_80386,
        X86_CORE_CPU_PROFILE_80486SX};
    for (lib_size item=0u; item < sizeof(intel)/sizeof(intel[0]); ++item) {
        cpu_bus_fixture original;
        cpu_bus_prepare(&original,intel[item]);
        original.memory[0x100u]=0x90u;
        x86_core_cpu_execution_refresh(&original.execution);
        const x86_core_cpu_timing_result result=x86_core_cpu_capture_timing(&original.execution);
        if (original.execution.stop_requested || !result.ticks ||
            result.retirement_origin==X86_CORE_CPU_RETIREMENT_TIMING_ORIGIN_V30_NOMINAL_SOURCE) return 1;
        ++*count;
    }
    return 0;
}

static lib_i32 v30_nominal_prefixes(void)
{
    static v30_memory_fixture fixture;
    static const struct {
        lib_u8 code[4], bytes;
        lib_u32 clocks, key, inputs;
        x86_core_cpu_retirement_timing_origin origin;
    } rows[] = {
        {{0x8bu,0x07u},2u,11u,0x18bu,X86_CORE_CPU_TIMING_INPUT_MODRM | X86_CORE_CPU_TIMING_INPUT_EFFECTIVE_ADDRESS,X86_CORE_CPU_RETIREMENT_TIMING_ORIGIN_V30_NOMINAL_SOURCE},
        {{0xb8u,0x34u,0x12u},3u,4u,0xb8u,0u,X86_CORE_CPU_RETIREMENT_TIMING_ORIGIN_V30_NOMINAL_SOURCE},
        {{0xa1u,0x01u,0x03u},3u,14u,0xa1u,X86_CORE_CPU_TIMING_INPUT_ODD_WORD,X86_CORE_CPU_RETIREMENT_TIMING_ORIGIN_V30_NOMINAL_SOURCE},
        {{0x83u,0xc0u,0x01u},3u,4u,0x83u,0u,X86_CORE_CPU_RETIREMENT_TIMING_ORIGIN_V30_NOMINAL_SOURCE},
        {{0xf7u,0xc0u,0x34u,0x12u},4u,4u,0xf7u,0u,X86_CORE_CPU_RETIREMENT_TIMING_ORIGIN_V30_NOMINAL_SOURCE},
        {{0xc1u,0xe0u,0x03u},3u,10u,0x8c1u,X86_CORE_CPU_TIMING_INPUT_SIZE,X86_CORE_CPU_RETIREMENT_TIMING_ORIGIN_V30_NOMINAL_SOURCE},
        {{0x87u,0x07u},2u,16u,0x187u,X86_CORE_CPU_TIMING_INPUT_MODRM | X86_CORE_CPU_TIMING_INPUT_EFFECTIVE_ADDRESS,X86_CORE_CPU_RETIREMENT_TIMING_ORIGIN_V30_NOMINAL_SOURCE},
        {{0x99u},1u,5u,0x99u,0u,X86_CORE_CPU_RETIREMENT_TIMING_ORIGIN_L2_DYNAMIC_ARITHMETIC},
        {{0x0fu,0x39u,0xc1u,0u},4u,87u,0x39000fu,X86_CORE_CPU_TIMING_INPUT_MEMORY,X86_CORE_CPU_RETIREMENT_TIMING_ORIGIN_L2_DYNAMIC_ARITHMETIC},
        {{0xf7u,0xe3u},2u,30u,0x8f7u,X86_CORE_CPU_TIMING_INPUT_SIZE,X86_CORE_CPU_RETIREMENT_TIMING_ORIGIN_L2_DYNAMIC_ARITHMETIC},
        {{0xc8u,0u,0u,1u},4u,19u,0xc8u,X86_CORE_CPU_TIMING_INPUT_MEMORY | X86_CORE_CPU_TIMING_INPUT_SIZE,X86_CORE_CPU_RETIREMENT_TIMING_ORIGIN_L2_CONTROL_MODEL}
    };
    static const lib_u8 prefixes[]={0x26u,0x2eu,0x36u,0x3eu,0xf0u};
    lib_u32 cases=0u;
    v30_memory_prepare(&fixture);
    for (lib_size row=0u; row<sizeof(rows)/sizeof(rows[0]); ++row)
        for (lib_u8 length=1u; length<=3u; ++length)
            for (lib_u16 sequence=0u, limit=length==1u ? 5u : length==2u ? 25u : 125u;
                sequence<limit; ++sequence)
                for (lib_u8 wrap=0u; wrap<2u; ++wrap) {
                    lib_u8 code[7], value=(lib_u8)sequence;
                    lib_u32 inputs=rows[row].inputs;
                    for (lib_u8 index=0u; index<length; ++index) {
                        code[index]=prefixes[value%5u];
                        value/=5u;
                        inputs|=code[index]==0xf0u ? X86_CORE_CPU_TIMING_INPUT_LOCK :
                            X86_CORE_CPU_TIMING_INPUT_SEGMENT_OVERRIDE;
                    }
                    lib_memory_copy(code+length,rows[row].code,rows[row].bytes);
                    v30_nominal_prepare(&fixture);
                    fixture.bus.cpu.data.sp=0x700u;
                    fixture.bus.cpu.data.eip=wrap ? 0xfffeu : 0x100u;
                    for (lib_u8 index=0u; index<length+rows[row].bytes; ++index)
                        fixture.memory[0x80000u+(lib_u16)(fixture.bus.cpu.data.eip+index)]=code[index];
                    const t_cpu before=fixture.bus.cpu;
                    x86_core_cpu_execution_refresh(&fixture.bus.execution);
                    const x86_core_cpu_timing_result timing=x86_core_cpu_capture_timing(&fixture.bus.execution);
                    if (fixture.bus.execution.stop_requested || fixture.bus.faults || fixture.invalid ||
                        fixture.observations || !fixture.bus.execution.completed_observation_valid ||
                        timing.ticks!=rows[row].clocks+2u*length || timing.key_id!=rows[row].key ||
                        timing.form_id!=rows[row].key || timing.formula_inputs!=inputs ||
                        timing.retirement_origin!=rows[row].origin || !timing.source_timing_unallocated ||
                        timing.event_ticks || x86_core_cpu_timing_maximum_ticks(X86_CORE_CPU_PROFILE_V30,LIB_NULL)) {
                        lib_c_printf("V30:NOMINAL-PREFIX:FAIL row=%u length=%u sequence=%u wrap=%u ticks=%llu inputs=%x\n",
                            (lib_u32)row,length,sequence,wrap,timing.ticks,timing.formula_inputs);
                        return 1;
                    }
                    ++cases;
                    if (length!=3u || (sequence!=0u && sequence!=124u)) continue;
                    const lib_u32 parts[]={fixture.fetches,fixture.reads,fixture.writes};
                    for (lib_u8 kind=0u; kind<3u; ++kind)
                        for (lib_u32 ordinal=1u; ordinal<=parts[kind]; ++ordinal) {
                            v30_nominal_prepare(&fixture);
                            fixture.bus.cpu=before;
                            for (lib_u8 index=0u; index<length+rows[row].bytes; ++index)
                                fixture.memory[0x80000u+(lib_u16)(before.data.eip+index)]=code[index];
                            if (kind==0u) fixture.fail_fetch=ordinal;
                            else if (kind==1u) fixture.fail_read=ordinal;
                            else fixture.fail_write=ordinal;
                            x86_core_cpu_execution_refresh(&fixture.bus.execution);
                            if (!fixture.bus.execution.stop_requested || fixture.bus.faults!=1u ||
                                fixture.bus.execution.completed_observation_valid ||
                                fixture.bus.execution.timing_result.ticks ||
                                fixture.bus.execution.timing_result.retirement_origin!=
                                    X86_CORE_CPU_RETIREMENT_TIMING_ORIGIN_UNATTRIBUTED ||
                                lib_memory_compare(&fixture.bus.cpu,&before,sizeof(before))) return 1;
                        }
                }
    lib_c_printf("V30:NOMINAL-PREFIX:OK cases=%u\n",cases);
    return 0;
}

lib_i32 main(void)
{
    _Static_assert(X86_CORE_CPU_RETIREMENT_TIMING_ORIGIN_L2_80486_MODEL==14 &&
        X86_CORE_CPU_RETIREMENT_TIMING_ORIGIN_V30_NOMINAL_SOURCE==15,
        "Existing timing origins must not move");
    lib_u32 cases=0u;
    if (v30_nominal_modrm(&cases)) {
        lib_c_printf("V30:NOMINAL-TRANSFER:MODRM cases=%u\n",cases);
        return 1;
    }
    if (v30_nominal_direct(&cases)) {
        lib_c_printf("V30:NOMINAL-TRANSFER:DIRECT cases=%u\n",cases);
        return 1;
    }
    if (v30_nominal_boundaries(&cases)) {
        lib_c_printf("V30:NOMINAL-TRANSFER:BOUNDARY cases=%u\n",cases);
        return 1;
    }
    if (v30_nominal_prefixes()) return 1;
    lib_c_printf("V30:NOMINAL-TRANSFER:OK cases=%u\n",cases);
    return 0;
}
