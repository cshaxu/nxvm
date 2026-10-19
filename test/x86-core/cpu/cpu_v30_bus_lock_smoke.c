#include "support/cpu_v30_memory_fixture.h"

typedef struct v30_lock_fixture {
    v30_memory_fixture memory;
    lib_u32 begins, completions, commits, ports, port_completions;
    lib_bool active, expected, bad, fail_begin, fail_port, pending, entry;
} v30_lock_fixture;

static lib_status v30_lock_begin(void *opaque)
{
    v30_lock_fixture *fixture = opaque;
    ++fixture->begins;
    fixture->bad |= fixture->active;
    if (fixture->fail_begin) return LIB_STATUS_IO_ERROR;
    fixture->active = LIB_TRUE;
    return LIB_STATUS_OK;
}

static void v30_lock_complete(void *opaque, lib_bool commit)
{
    v30_lock_fixture *fixture = opaque;
    fixture->bad |= !fixture->active;
    fixture->active = LIB_FALSE;
    ++fixture->completions;
    fixture->commits += commit != LIB_FALSE;
}

static lib_status v30_lock_read(void *opaque, lib_u32 address, void *destination,
    lib_u8 bytes, x86_core_cpu_memory_access_provenance provenance,
    lib_bool observe, lib_bool reset, lib_u8 cache)
{
    v30_lock_fixture *fixture = opaque;
    if (!observe && provenance == X86_CORE_CPU_MEMORY_ACCESS_DATA)
        fixture->bad |= fixture->active != (fixture->expected &&
            (!fixture->entry || fixture->memory.bus.execution.instruction_in_progress));
    return v30_memory_read(&fixture->memory,address,destination,bytes,provenance,
        observe,reset,cache);
}

static lib_status v30_lock_write(void *opaque, lib_u32 address, const void *source,
    lib_u8 bytes, x86_core_cpu_memory_access_provenance provenance, lib_u8 cache)
{
    v30_lock_fixture *fixture = opaque;
    fixture->bad |= fixture->active != (fixture->expected &&
        (!fixture->entry || fixture->memory.bus.execution.instruction_in_progress));
    return v30_memory_write(&fixture->memory,address,source,bytes,provenance,cache);
}

static lib_u64 v30_lock_cycle(void *opaque, x86_core_cpu_external_cycle_phase phase,
    x86_core_cpu_external_cycle_space space, lib_u32 address, lib_u8 bytes,
    lib_u8 write, x86_core_cpu_memory_access_provenance provenance)
{
    v30_lock_fixture *fixture = opaque;
    return space == X86_CORE_CPU_EXTERNAL_CYCLE_SPACE_MEMORY ?
        v30_memory_cycle(&fixture->memory,phase,space,address,bytes,write,provenance) : 0u;
}

static lib_status v30_lock_port(void *opaque, lib_u16 port, lib_u8 bytes,
    lib_bool write, lib_u32 *value)
{
    v30_lock_fixture *fixture = opaque;
    ++fixture->ports;
    fixture->bad |= fixture->active != fixture->expected || port != 0x1234u ||
        (bytes != 1u && bytes != 2u);
    if (fixture->fail_port) return LIB_STATUS_IO_ERROR;
    if (!write) *value = 0x5678u;
    return LIB_STATUS_OK;
}

static void v30_lock_port_complete(void *opaque, lib_u16 port, lib_u8 bytes,
    lib_bool write)
{
    v30_lock_fixture *fixture = opaque;
    (void)port; (void)bytes; (void)write;
    fixture->bad |= fixture->active != fixture->expected;
    ++fixture->port_completions;
}

static lib_bool v30_lock_pending(void *opaque)
{
    return ((v30_lock_fixture *)opaque)->pending;
}

static lib_status v30_lock_acknowledge(void *opaque, lib_u8 *vector)
{
    v30_lock_fixture *fixture = opaque;
    fixture->bad |= fixture->active;
    fixture->pending = LIB_FALSE;
    fixture->expected = LIB_FALSE;
    *vector = 0x30u;
    return LIB_STATUS_OK;
}

static void v30_lock_prepare(v30_lock_fixture *fixture, const lib_u8 *code,
    lib_u8 bytes, lib_bool locked)
{
    static const x86_core_cpu_bus_provider provider = {
        .read_memory=v30_lock_read, .write_memory=v30_lock_write,
        .transfer_port=v30_lock_port, .complete_port=v30_lock_port_complete,
        .interrupt_pending=v30_lock_pending, .acknowledge_interrupt=v30_lock_acknowledge,
        .begin_memory_lock=v30_lock_begin, .complete_memory_lock=v30_lock_complete
    };
    v30_memory_restart(&fixture->memory);
    lib_memory_set(&fixture->begins,0,sizeof(*fixture)-lib_offsetof(v30_lock_fixture,begins));
    fixture->expected = locked;
    fixture->memory.bus.execution.bus = &provider;
    fixture->memory.bus.execution.bus_context = fixture;
    x86_core_cpu_execution_context_bind_external_cycle_provider(&fixture->memory.bus.execution,
        v30_lock_cycle,fixture);
    fixture->memory.bus.cpu.data.eip = 0x100u;
    fixture->memory.bus.cpu.data.dx = 0x1234u;
    fixture->memory.bus.cpu.data.si = 0x400u;
    fixture->memory.bus.cpu.data.di = 0x600u;
    fixture->memory.bus.cpu.data.sp = 0x900u;
    fixture->memory.bus.cpu.data.eflags &= ~(VCPU_EFLAGS_IF | VCPU_EFLAGS_TF | VCPU_EFLAGS_DF);
    lib_memory_copy(fixture->memory.memory+0x80100u,code,bytes);
    lib_memory_set(fixture->memory.memory+0x10400u,0x12,16u);
    lib_memory_set(fixture->memory.memory+0x30600u,0x34,16u);
}

static lib_i32 v30_lock_scalar(void)
{
    static const struct {lib_u8 code[6], bytes;} cases[] = {
        {{0x90u},1u}, {{0x01u,0xc0u},2u},
        {{0x01u,0x06u,0x00u,0x04u},4u}, {{0x89u,0x06u,0x00u,0x04u},4u},
        {{0x87u,0x06u,0x00u,0x04u},4u}, {{0x68u,0x34u,0x12u},3u},
        {{0xecu},1u}, {{0xedu},1u}, {{0xeeu},1u}, {{0xefu},1u},
        {{0x0fu,0x12u,0x06u,0x00u,0x04u},5u}
    };
    static v30_lock_fixture fixture;
    for (lib_u8 row=0u; row<sizeof(cases)/sizeof(cases[0]); ++row)
        for (lib_u8 locked=0u; locked<2u; ++locked) {
            lib_u8 code[8] = {0xf0u,0x3eu};
            lib_memory_copy(code+2u,cases[row].code,cases[row].bytes);
            v30_lock_prepare(&fixture,code+(locked ? 0u : 2u),
                cases[row].bytes+(locked ? 2u : 0u),locked != 0u);
            const t_cpu before=fixture.memory.bus.cpu;
            x86_core_cpu_instruction_lexeme lexeme;
            const lib_bool preview=x86_core_cpu_execution_preview_lexeme(
                &fixture.memory.bus.execution,&lexeme);
            if (preview==locked || fixture.begins || fixture.completions || fixture.ports ||
                fixture.memory.reads || fixture.memory.writes || fixture.memory.fetches ||
                lib_memory_compare(&before,&fixture.memory.bus.cpu,sizeof(before))) return 1;
            x86_core_cpu_execution_refresh(&fixture.memory.bus.execution);
            x86_core_cpu_instruction_observation observation;
            x86_core_cpu_execution_copy_observation(&fixture.memory.bus.execution,&observation);
            if (fixture.bad || fixture.active || fixture.begins != locked ||
                fixture.completions != locked || fixture.commits != locked ||
                fixture.memory.bus.faults || fixture.memory.bus.execution.stop_requested ||
                observation.lock_prefix != locked) {
                lib_c_printf("V30:BUSLOCK:SCALAR row=%u lock=%u begin=%u complete=%u active=%u bad=%u observed=%u\n",
                    row,locked,fixture.begins,fixture.completions,fixture.active,fixture.bad,
                    observation.lock_prefix);
                return 1;
            }
        }
    return 0;
}

static lib_i32 v30_lock_repeat(void)
{
    static const lib_u8 operations[] = {0xa4u,0xa5u,0xa6u,0xa7u,0xaau,0xabu,
        0xacu,0xadu,0xaeu,0xafu,0x6cu,0x6du,0x6eu,0x6fu};
    static const lib_u8 prefixes[][4] = {
        {0xf0u,0xf3u},
        {0xf0u,0xf3u,0x3eu}, {0xf0u,0x3eu,0xf3u},
        {0xf3u,0xf0u,0x3eu}, {0xf3u,0x3eu,0xf0u},
        {0x3eu,0xf0u,0xf3u}, {0x3eu,0xf3u,0xf0u},
        {0xf0u,0xf0u,0xf3u,0x3eu}, {0x3eu,0x3eu,0xf3u,0xf0u},
        {0xf3u,0xf0u,0xf3u,0x3eu}
    };
    static const lib_u8 prefix_bytes[]={2u,3u,3u,3u,3u,3u,3u,4u,4u,4u};
    static v30_lock_fixture fixture;
    for (lib_u8 row=0u; row<sizeof(operations); ++row)
      for (lib_size prefix=0u; prefix<sizeof(prefix_bytes); ++prefix)
        for (lib_u8 count=0u; count<4u; ++count)
        for (lib_u8 layout=0u; layout<4u; ++layout) {
            lib_u8 code[5]={0};
            lib_memory_copy(code,prefixes[prefix],prefix_bytes[prefix]);
            code[prefix_bytes[prefix]]=operations[row];
            v30_lock_prepare(&fixture,code,prefix_bytes[prefix]+1u,LIB_TRUE);
            fixture.memory.bus.cpu.data.cx=count;
            fixture.memory.bus.cpu.data.si += (layout&1u) + (layout&2u ? 8u : 0u);
            fixture.memory.bus.cpu.data.di += (layout&1u) + (layout&2u ? 8u : 0u);
            if (layout&2u) fixture.memory.bus.cpu.data.eflags |= VCPU_EFLAGS_DF;
            /* Equal comparison bytes/accumulator keep REP through the count. */
            fixture.memory.bus.cpu.data.ax=0x1212u;
            lib_memory_set(fixture.memory.memory+0x30600u,0x12,16u);
            for (lib_u8 part=0u; part<(count ? count : 1u); ++part) {
                x86_core_cpu_execution_refresh(&fixture.memory.bus.execution);
                const lib_bool more=part+1u<count;
                if (fixture.bad || fixture.memory.invalid || fixture.memory.bus.faults ||
                    fixture.memory.bus.execution.stop_requested || fixture.begins != 1u ||
                    fixture.active != more || fixture.completions != !more ||
                    fixture.commits != !more || fixture.memory.bus.cpu.data.cx !=
                        (count ? count-part-1u : 0u)) {
                    lib_c_printf("V30:BUSLOCK:REPEAT op=%02x prefix=%u count=%u part=%u begin=%u complete=%u active=%u bad=%u\n",
                        operations[row],(lib_u32)prefix,count,part,fixture.begins,
                        fixture.completions,fixture.active,fixture.bad);
                    return 1;
                }
                if (more) {
                    const t_cpu before=fixture.memory.bus.cpu;
                    x86_core_cpu_instruction_lexeme lexeme;
                    if (x86_core_cpu_execution_preview_lexeme(&fixture.memory.bus.execution,&lexeme) ||
                        !fixture.active || fixture.begins!=1u || fixture.completions ||
                        lib_memory_compare(&before,&fixture.memory.bus.cpu,sizeof(before))) {
                        lib_c_printf("V30:BUSLOCK:PREVIEW op=%02x count=%u part=%u active=%u complete=%u\n",
                            operations[row],count,part,fixture.active,fixture.completions);
                        return 1;
                    }
                }
            }
        }
    return 0;
}

static lib_i32 v30_lock_failures(void)
{
    static v30_lock_fixture fixture;
    /* Odd word has two accepted/refused byte transfers, without RAM rollback. */
    const lib_u8 code[] = {0xf0u,0x01u,0x06u,0x01u,0x04u};
    for (lib_u8 kind=0u; kind<5u; ++kind)
        for (lib_u8 part=1u; part<=(kind==1u || kind==2u ? 2u : 1u); ++part) {
            v30_lock_prepare(&fixture,code,sizeof(code),LIB_TRUE);
            if (kind==0u) fixture.fail_begin=LIB_TRUE;
            else if (kind==1u) fixture.memory.fail_read=part;
            else if (kind==2u) fixture.memory.fail_write=part;
            else if (kind==3u) fixture.memory.fail_fetch=1u;
            else { fixture.memory.memory[0x80101u]=0xecu; fixture.fail_port=LIB_TRUE; }
            const t_cpu before=fixture.memory.bus.cpu;
            x86_core_cpu_execution_refresh(&fixture.memory.bus.execution);
            const lib_bool began=kind!=0u && kind!=3u;
            const lib_u8 low=kind==2u && part==2u ? 0x8au : 0x12u;
            if (fixture.bad || fixture.active || fixture.completions != began || fixture.commits ||
                !fixture.memory.bus.execution.stop_requested || !fixture.memory.bus.faults ||
                fixture.memory.bus.execution.completed_observation_valid ||
                fixture.memory.bus.execution.timing_result.ticks || fixture.port_completions ||
                fixture.memory.memory[0x10401u]!=low || fixture.memory.memory[0x10402u]!=0x12u ||
                lib_memory_compare(&fixture.memory.bus.cpu,&before,sizeof(before))) {
                lib_c_printf("V30:BUSLOCK:FAILURE kind=%u part=%u begin=%u complete=%u bad=%u\n",
                    kind,part,fixture.begins,fixture.completions,fixture.bad);
                return 1;
            }
        }
    /* Refusal after a fetched BUSLOCK, rather than only before its admission. */
    for (lib_u8 prefix=0u; prefix<2u; ++prefix) {
        const lib_u8 stream[] = {0xf0u,0x3eu,0x90u};
        v30_lock_prepare(&fixture,stream,sizeof(stream),LIB_TRUE);
        fixture.memory.bus.cpu.data.eip=prefix ? 0xfffeu : 0xffffu;
        fixture.memory.memory[0x8fffeu]=stream[0];
        fixture.memory.memory[0x8ffffu]=prefix ? stream[1] : stream[0];
        fixture.memory.memory[0x80000u]=stream[2];
        fixture.memory.fail_fetch=2u;
        x86_core_cpu_execution_refresh(&fixture.memory.bus.execution);
        if (fixture.active || fixture.commits || fixture.completions!=fixture.begins ||
            !fixture.memory.bus.execution.stop_requested) return 1;
    }
    for (lib_u8 missing=0u; missing<2u; ++missing) {
        v30_lock_prepare(&fixture,code,sizeof(code),LIB_TRUE);
        x86_core_cpu_bus_provider provider=*fixture.memory.bus.execution.bus;
        if (missing) provider.begin_memory_lock=LIB_NULL;
        else provider.complete_memory_lock=LIB_NULL;
        fixture.memory.bus.execution.bus=&provider;
        x86_core_cpu_execution_refresh(&fixture.memory.bus.execution);
        if (!fixture.memory.bus.execution.stop_requested || fixture.begins || fixture.active ||
            fixture.completions || fixture.memory.reads || fixture.memory.writes) return 1;
    }
    return 0;
}

static lib_i32 v30_lock_lifetime(void)
{
    static v30_lock_fixture fixture;
    static const lib_u8 padding[] = {0u,14u,30u};
    for (lib_size prefix=0u; prefix<sizeof(padding); ++prefix) {
      lib_u8 code[34];
      lib_memory_set(code,0x3eu,sizeof(code));
      code[0]=0xf0u;
      code[padding[prefix]+1u]=0xf3u;
      code[padding[prefix]+2u]=0xa4u;
      for (lib_u8 ending=0u; ending<8u; ++ending) {
        v30_lock_prepare(&fixture,code,padding[prefix]+3u,LIB_TRUE);
        fixture.memory.bus.cpu.data.cx=3u;
        x86_core_cpu_execution_refresh(&fixture.memory.bus.execution);
        if (!fixture.active || fixture.begins!=1u || fixture.completions) return 1;
        const t_cpu before=fixture.memory.bus.cpu;
        x86_core_cpu_instruction_lexeme lexeme;
        if (x86_core_cpu_execution_preview_lexeme(&fixture.memory.bus.execution,&lexeme) ||
            !fixture.active || fixture.begins!=1u || fixture.completions ||
            lib_memory_compare(&before,&fixture.memory.bus.cpu,sizeof(before))) return 1;
        if (ending==0u) {
            x86_core_cpu_execution_finalize(&fixture.memory.bus.execution);
            x86_core_cpu_execution_finalize(&fixture.memory.bus.execution);
        } else if (ending==1u) x86_core_cpu_state_reset(&fixture.memory.bus.execution);
        else if (ending==2u || ending==3u) {
            fixture.memory.bus.cpu.data.eflags |= VCPU_EFLAGS_IF;
            fixture.entry=LIB_TRUE;
            if (ending==2u) fixture.pending=LIB_TRUE;
            else fixture.memory.bus.execution.nmi_pending=LIB_TRUE;
            const lib_u32 vector=ending==2u ? 0xc0u : 8u;
            fixture.memory.memory[vector]=0x00u;
            fixture.memory.memory[vector+1u]=0x02u;
            fixture.memory.memory[vector+2u]=0x00u;
            fixture.memory.memory[vector+3u]=0x80u;
            fixture.memory.memory[0x80200u]=0xcfu;
            x86_core_cpu_execution_refresh(&fixture.memory.bus.execution);
            if (fixture.active || fixture.completions!=1u || fixture.commits!=1u ||
                fixture.memory.bus.cpu.data.eip!=0x200u || fixture.memory.bus.cpu.data.cx!=1u ||
                fixture.memory.bus.faults || fixture.bad) return 1;
            fixture.expected=LIB_FALSE;
            x86_core_cpu_execution_refresh(&fixture.memory.bus.execution);
            /* A long interrupted stream retains only its last three prefixes. */
            fixture.expected=padding[prefix]==0u;
            fixture.entry=LIB_FALSE;
            x86_core_cpu_execution_refresh(&fixture.memory.bus.execution);
            const lib_u32 locks=padding[prefix]==0u ? 2u : 1u;
            if (fixture.active || fixture.begins!=locks || fixture.completions!=locks ||
                fixture.commits!=locks || fixture.memory.bus.cpu.data.cx || fixture.bad) return 1;
            continue;
        } else {
            /* Public debugger-style invalidation accompanies a code/context edit. */
            if (ending==7u) fixture.memory.memory[0x80102u+padding[prefix]]=0x90u;
            else {
                fixture.memory.memory[0x80100u]=ending==4u ? 0x90u : 0xf0u;
                fixture.memory.memory[0x80101u]=0x90u;
            }
            if (ending==6u) fixture.memory.bus.cpu.data.eip=0x101u;
            fixture.expected=ending==5u || ending==7u;
            x86_core_cpu_execution_invalidate_prefetch(&fixture.memory.bus.execution);
            x86_core_cpu_execution_refresh(&fixture.memory.bus.execution);
        }
        const lib_bool replacement=ending==5u || ending==7u;
        if (fixture.active || fixture.begins!=1u+replacement ||
            fixture.completions!=fixture.begins || fixture.commits!=replacement || fixture.bad) {
            lib_c_printf("V30:BUSLOCK:LIFETIME prefixes=%u ending=%u begin=%u complete=%u commit=%u bad=%u\n",
                padding[prefix]+2u,ending,fixture.begins,fixture.completions,fixture.commits,fixture.bad);
            return 1;
        }
      }
    }
    /* Zero/equal stop conditions do not retain a lock merely because CX remains. */
    for (lib_u8 opcode=0xa6u; opcode<=0xaeu; opcode+=8u) {
        const lib_u8 compare[] = {0xf0u,0xf3u,opcode};
        v30_lock_prepare(&fixture,compare,sizeof(compare),LIB_TRUE);
        fixture.memory.bus.cpu.data.cx=3u;
        x86_core_cpu_execution_refresh(&fixture.memory.bus.execution);
        if (fixture.active || fixture.begins!=1u || fixture.completions!=1u ||
            fixture.commits!=1u || fixture.memory.bus.cpu.data.cx!=2u || fixture.bad) return 1;
    }
    return 0;
}

lib_i32 main(void)
{
    static const struct {const char *name; lib_i32 (*run)(void);} cases[] = {
        {"scalar",v30_lock_scalar}, {"repeat",v30_lock_repeat},
        {"failures",v30_lock_failures}, {"lifetime",v30_lock_lifetime}
    };
    for (lib_u8 row=0u; row<sizeof(cases)/sizeof(cases[0]); ++row)
        if (cases[row].run()) {
            lib_c_printf("V30:BUSLOCK:FAILED %s\n",cases[row].name);
            return 1;
        }
    lib_c_printf("V30:BUSLOCK:OK\n");
    return 0;
}
