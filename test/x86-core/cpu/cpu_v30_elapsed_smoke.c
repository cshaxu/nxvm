#include "support/cpu_v30_memory_fixture.h"

typedef struct v30_elapsed_fixture {
    v30_memory_fixture memory;
    x86_core_cpu_bus_provider provider;
    lib_u64 wait, accepted_waits;
    lib_u32 samples, commits, cancels;
    lib_bool blocked, overflow_fetch;
} v30_elapsed_fixture;

static lib_u64 v30_elapsed_cycle(void *opaque, x86_core_cpu_external_cycle_phase phase,
    x86_core_cpu_external_cycle_space space, lib_u32 address, lib_u8 bytes,
    lib_u8 write, x86_core_cpu_memory_access_provenance provenance)
{
    v30_elapsed_fixture *fixture=opaque;
    (void)space; (void)address; (void)bytes; (void)write; (void)provenance;
    if (phase == X86_CORE_CPU_EXTERNAL_CYCLE_PHASE_CANCEL) ++fixture->cancels;
    if (phase != X86_CORE_CPU_EXTERNAL_CYCLE_PHASE_COMMIT) return 0u;
    ++fixture->commits;
    if (fixture->overflow_fetch && fixture->commits==1u)
        fixture->memory.bus.instructions.data.timing_fetch_bus_ticks=LIB_UINT32_MAX-3u;
    fixture->accepted_waits+=fixture->wait;
    return fixture->wait;
}

static lib_bool v30_elapsed_poll(void *opaque)
{
    v30_elapsed_fixture *fixture=opaque;
    ++fixture->samples;
    return fixture->blocked;
}

static void v30_elapsed_prepare(v30_elapsed_fixture *fixture)
{
    lib_memory_set(fixture,0,sizeof(*fixture));
    v30_memory_prepare(&fixture->memory);
    fixture->provider=*fixture->memory.bus.execution.bus;
    fixture->provider.wait_test_asserted=v30_elapsed_poll;
    fixture->provider.acknowledge_interrupt=cpu_bus_acknowledge;
    fixture->memory.bus.execution.bus=&fixture->provider;
    fixture->memory.bus.execution.bus_context=fixture;
    x86_core_cpu_execution_context_bind_external_cycle_provider(
        &fixture->memory.bus.execution,v30_elapsed_cycle,fixture);
}

/* Independent bus oracle: count accepted provider records, never the CPU
 * aggregate/counter. Both byte and aligned-word cycles cost four clocks. */
static lib_u64 v30_elapsed_service(const v30_elapsed_fixture *fixture,
    lib_u32 first, lib_bool fetch_only)
{
    lib_u64 clocks=0u;
    for (lib_u32 index=first; index < fixture->memory.record_count; ++index) {
        const v30_memory_record *record=&fixture->memory.records[index];
        if (record->accepted && !record->observe &&
            (!fetch_only || record->provenance != X86_CORE_CPU_MEMORY_ACCESS_DATA))
            clocks+=4u;
    }
    return clocks;
}

static lib_i32 v30_elapsed_expect(const v30_elapsed_fixture *fixture,
    lib_u64 ticks, lib_u64 events)
{
    const x86_core_cpu_timing_result actual=x86_core_cpu_capture_timing(
        &fixture->memory.bus.execution);
    if (actual.ticks != ticks || actual.event_ticks != events ||
        (ticks && (actual.source_timing_unallocated || actual.retirement_origin !=
            ((actual.formula_inputs & X86_CORE_CPU_TIMING_INPUT_FETCH) ?
                X86_CORE_CPU_RETIREMENT_TIMING_ORIGIN_V30_ELAPSED_L2 :
                X86_CORE_CPU_RETIREMENT_TIMING_ORIGIN_V30_NOMINAL_SOURCE)))) {
        lib_c_printf("V30:ELAPSED:TIME ticks=%llu/%llu events=%llu/%llu origin=%u\n",
            actual.ticks,ticks,actual.event_ticks,events,(lib_u32)actual.retirement_origin);
        return 1;
    }
    return 0;
}

static lib_i32 v30_elapsed_fetch(void)
{
    static v30_elapsed_fixture fixture;
    static const lib_u16 offsets[]={0x100u,0x101u,0xfffu,0xfffeu,0xffffu};
    for (lib_size layout=0u; layout < sizeof(offsets)/sizeof(offsets[0]); ++layout) {
        v30_elapsed_prepare(&fixture);
        fixture.memory.bus.cpu.data.eip=offsets[layout];
        for (lib_u8 index=0u; index < 40u; ++index)
            fixture.memory.memory[0x80000u+(lib_u16)(offsets[layout]+index)]=0x90u;
        lib_u64 total=0u;
        for (lib_u8 step=0u; step < 12u; ++step) {
            const lib_u32 first=fixture.memory.record_count;
            x86_core_cpu_execution_refresh(&fixture.memory.bus.execution);
            const lib_u64 fetch=v30_elapsed_service(&fixture,first,LIB_TRUE);
            if (fixture.memory.bus.execution.stop_requested || fixture.memory.invalid ||
                fixture.memory.bus.instructions.data.timing_fetch_bus_ticks!=fetch ||
                ((fixture.memory.bus.execution.timing_result.formula_inputs &
                    X86_CORE_CPU_TIMING_INPUT_FETCH)!=0u)!=(fetch!=0u) ||
                v30_elapsed_expect(&fixture,3u+fetch,0u)) return 1;
            total+=3u+fetch;
        }
        if (layout==0u && total!=60u) return 1; /* Two six-byte fills +12 NOPs. */
    }
    for (lib_u8 padding=0u; padding <= 30u; padding+=15u) {
        v30_elapsed_prepare(&fixture);
        for (lib_u8 index=0u; index < padding; ++index)
            fixture.memory.memory[0x80100u+index]=0x26u;
        fixture.memory.memory[0x80100u+padding]=0x90u;
        x86_core_cpu_execution_refresh(&fixture.memory.bus.execution);
        if (fixture.memory.bus.execution.stop_requested || v30_elapsed_expect(&fixture,
            3u+2u*padding+v30_elapsed_service(&fixture,0u,LIB_TRUE),0u)) return 1;
    }
    return 0;
}

static lib_i32 v30_elapsed_operands_and_failure(void)
{
    static v30_elapsed_fixture fixture;
    for (lib_u8 odd=0u; odd < 2u; ++odd)
        for (lib_u8 fail=0u; fail <= 2u+odd; ++fail) {
            v30_elapsed_prepare(&fixture);
            const lib_u16 address=0x400u+odd;
            lib_memory_copy(fixture.memory.memory+0x80100u,
                (const lib_u8[]){0x8bu,0x06u,(lib_u8)address,(lib_u8)(address>>8u)},4u);
            fixture.wait=3u;
            if (fail==1u) fixture.memory.fail_fetch=2u;
            else if (fail) fixture.memory.fail_read=fail-1u;
            x86_core_cpu_execution_refresh(&fixture.memory.bus.execution);
            if (fixture.accepted_waits != 3u*fixture.commits || fixture.memory.invalid) return 1;
            if (fail) {
                if (!fixture.memory.bus.execution.stop_requested || fixture.cancels!=1u ||
                    v30_elapsed_expect(&fixture,0u,v30_elapsed_service(&fixture,0u,LIB_FALSE))) return 1;
            } else if (fixture.memory.bus.execution.stop_requested || fixture.cancels ||
                v30_elapsed_expect(&fixture,11u+4u*odd+
                    v30_elapsed_service(&fixture,0u,LIB_TRUE),0u)) return 1;
        }
    for (lib_u8 padding=0u; padding<=30u; padding+=15u) {
        v30_elapsed_prepare(&fixture);
        for (lib_u8 index=0u; index<padding; ++index)
            fixture.memory.memory[0x80100u+index]=0x3eu;
        lib_memory_copy(fixture.memory.memory+0x80100u+padding,
            (const lib_u8[]){0x8bu,0x06u,0u,4u},4u);
        fixture.memory.fail_read=1u;
        x86_core_cpu_execution_refresh(&fixture.memory.bus.execution);
        if (!fixture.memory.bus.execution.stop_requested || fixture.cancels!=1u ||
            v30_elapsed_expect(&fixture,0u,
                v30_elapsed_service(&fixture,0u,LIB_FALSE)+2u*padding)) return 1;
    }
    return 0;
}

static lib_i32 v30_elapsed_entry(void)
{
    static v30_elapsed_fixture fixture;
    for (lib_u8 nmi=0u; nmi < 2u; ++nmi)
        for (lib_u8 odd=0u; odd < 2u; ++odd)
            for (lib_u8 fail=0u; fail <= 6u+3u*odd; ++fail) {
                v30_elapsed_prepare(&fixture);
                fixture.memory.memory[0x80100u]=0x90u;
                fixture.memory.bus.cpu.data.sp+=odd;
                fixture.memory.bus.cpu.data.eflags|=VCPU_EFLAGS_IF;
                const lib_u8 vector=nmi ? 2u : 0x30u;
                fixture.memory.memory[4u*vector+1u]=2u;
                if (fail==1u) fixture.memory.bus.fail_acknowledge=LIB_TRUE;
                else if (fail<=4u+3u*odd && fail) fixture.memory.fail_write=fail-1u;
                else if (fail) fixture.memory.fail_read=fail-4u-3u*odd;
                if (nmi) {
                    if (!x86_core_cpu_request_nmi(&fixture.memory.bus.execution)) return 1;
                } else fixture.memory.bus.interrupt=LIB_TRUE;
                x86_core_cpu_execution_refresh(&fixture.memory.bus.execution);
                const lib_bool failed=fail!=0u && !(nmi && fail==1u);
                const lib_u64 fetch=v30_elapsed_service(&fixture,0u,LIB_TRUE);
                const lib_u64 accepted=v30_elapsed_service(&fixture,0u,LIB_FALSE)-fetch;
                const lib_u64 events=failed ? accepted : (nmi ? 38u : 49u)+12u*odd;
                if (fixture.memory.bus.execution.stop_requested != failed ||
                    fixture.memory.fetches!=3u ||
                    !fixture.memory.bus.execution.completed_observation_valid ||
                    fixture.memory.bus.execution.completed_observation.point.linear_pc!=0x80100u ||
                    v30_elapsed_expect(&fixture,15u,events)) return 1;
            }
    return 0;
}

static lib_i32 v30_elapsed_poll_and_idle(void)
{
    static v30_elapsed_fixture fixture;
    v30_elapsed_prepare(&fixture);
    fixture.memory.memory[0x80100u]=0x9bu;
    fixture.blocked=LIB_TRUE;
    x86_core_cpu_execution_refresh(&fixture.memory.bus.execution);
    lib_u64 remaining=0u;
    if (fixture.samples!=1u || v30_elapsed_expect(&fixture,19u,0u) ||
        x86_core_cpu_execution_wait_poll_ticks(&fixture.memory.bus.execution,&remaining)!=LIB_STATUS_OK ||
        remaining!=5u) return 1;
    x86_core_cpu_execution_refresh(&fixture.memory.bus.execution);
    if (fixture.samples!=1u || v30_elapsed_expect(&fixture,0u,0u)) return 1;
    x86_core_cpu_execution_advance_wait(&fixture.memory.bus.execution,4u);
    if (fixture.samples!=1u) return 1;
    fixture.blocked=LIB_FALSE;
    x86_core_cpu_execution_advance_wait(&fixture.memory.bus.execution,1u);
    if (fixture.samples!=2u || fixture.memory.bus.cpu.data.flagHalt) return 1;
    v30_elapsed_prepare(&fixture);
    fixture.memory.memory[0x80100u]=0xf4u;
    x86_core_cpu_execution_refresh(&fixture.memory.bus.execution);
    if (v30_elapsed_expect(&fixture,14u,0u)) return 1;
    const lib_u32 records=fixture.memory.record_count;
    x86_core_cpu_execution_refresh(&fixture.memory.bus.execution);
    if (fixture.memory.record_count!=records || v30_elapsed_expect(&fixture,0u,0u)) return 1;
    x86_core_cpu_state_reset(&fixture.memory.bus.execution);
    if (fixture.memory.bus.execution.wait_state!=CPU_WAIT_STATE_NONE ||
        fixture.memory.bus.execution.wait_poll_remaining_ticks) return 1;
    return 0;
}

static lib_i32 v30_elapsed_preview_and_overflow(void)
{
    static v30_elapsed_fixture fixture;
    for (lib_u8 fail=0u; fail < 2u; ++fail) {
        v30_elapsed_prepare(&fixture);
        fixture.memory.memory[0x80100u]=0x9bu;
        fixture.blocked=LIB_TRUE;
        fixture.memory.fail_observe=fail ? 2u : 0u;
        const x86_core_cpu_execution_context before=fixture.memory.bus.execution;
        const t_cpu cpu=fixture.memory.bus.cpu;
        const t_cpuins instruction=fixture.memory.bus.instructions;
        x86_core_cpu_instruction_lexeme lexeme;
        if (x86_core_cpu_execution_preview_lexeme(&fixture.memory.bus.execution,&lexeme)==fail ||
            fixture.samples || fixture.commits || fixture.cancels || fixture.memory.fetches ||
            lib_memory_compare(&before,&fixture.memory.bus.execution,sizeof(before)) ||
            lib_memory_compare(&cpu,&fixture.memory.bus.cpu,sizeof(cpu)) ||
            lib_memory_compare(&instruction,&fixture.memory.bus.instructions,sizeof(instruction))) return 1;
    }
    v30_elapsed_prepare(&fixture);
    fixture.memory.memory[0x80100u]=0x90u;
    x86_core_cpu_execution_context_bind_external_cycle_provider(
        &fixture.memory.bus.execution,LIB_NULL,LIB_NULL);
    x86_core_cpu_execution_refresh(&fixture.memory.bus.execution);
    if (fixture.commits || fixture.memory.fetches!=3u ||
        fixture.memory.bus.instructions.data.timing_fetch_bus_ticks!=12u ||
        v30_elapsed_expect(&fixture,15u,0u)) return 1;
    const x86_core_cpu_timing_result before=x86_core_cpu_capture_timing(&fixture.memory.bus.execution);
    const lib_u64 service=fixture.memory.bus.instructions.data.timing_transfer_ticks;
    lib_u8 byte=0u;
    if (x86_core_cpu_read_linear(&fixture.memory.bus.execution,0x80100u,&byte,1u) || byte!=0x90u ||
        service!=fixture.memory.bus.instructions.data.timing_transfer_ticks ||
        lib_memory_compare(&before,&fixture.memory.bus.execution.timing_result,sizeof(before))) return 1;
    v30_elapsed_prepare(&fixture);
    fixture.memory.memory[0x80100u]=0x90u;
    fixture.overflow_fetch=LIB_TRUE;
    x86_core_cpu_execution_refresh(&fixture.memory.bus.execution);
    if (!fixture.memory.bus.execution.stop_requested || fixture.memory.bus.faults!=1u ||
        fixture.memory.bus.execution.completed_observation_valid || fixture.commits!=2u ||
        fixture.cancels || v30_elapsed_expect(&fixture,0u,8u)) return 1;
    return 0;
}

lib_i32 main(void)
{
    if (v30_elapsed_fetch()) { lib_c_printf("V30:ELAPSED:FETCH:FAIL\n"); return 1; }
    if (v30_elapsed_operands_and_failure()) { lib_c_printf("V30:ELAPSED:BUS:FAIL\n"); return 1; }
    if (v30_elapsed_entry()) { lib_c_printf("V30:ELAPSED:ENTRY:FAIL\n"); return 1; }
    if (v30_elapsed_poll_and_idle()) { lib_c_printf("V30:ELAPSED:IDLE:FAIL\n"); return 1; }
    if (v30_elapsed_preview_and_overflow()) { lib_c_printf("V30:ELAPSED:PURE-OVERFLOW:FAIL\n"); return 1; }
    lib_c_printf("V30:ELAPSED:OK\n");
    return 0;
}
