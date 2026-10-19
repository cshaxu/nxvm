#include "x86-core/machine/machine.h"
#include "x86-core/machine/debug_interface.h"
#include "lib/types/file.h"

typedef struct v30_time_probe {
    lib_u64 ticks, source_ticks;
    lib_u32 samples, finalized, retired;
    lib_bool blocked;
} v30_time_probe;

static lib_bool v30_time_poll(void *opaque)
{
    v30_time_probe *probe=opaque;
    ++probe->samples;
    return probe->blocked;
}

static void v30_time_advance(void *opaque, lib_u64 ticks, lib_u64 due)
{
    v30_time_probe *probe=opaque;
    (void)ticks;
    probe->ticks=due;
}

static void v30_time_finalize(void *opaque)
{
    v30_time_probe *probe=opaque;
    ++probe->finalized;
}

static void v30_time_retired(void *opaque,
    const x86_core_machine_retirement_observation *observation)
{
    v30_time_probe *probe=opaque;
    ++probe->retired;
    probe->source_ticks=observation->source_ticks;
}

/* Actual configured construction/run, with no CPU pointer or profile rebinding.
 * One-tick slices must transport completion without resampling POLL early. */
static lib_i32 v30_time_poll_receiver(lib_u8 stage, lib_u8 wait)
{
    x86_core_machine_executor_config config={
        .memory_bytes=0x100000u,.cpu_profile=X86_CORE_CPU_PROFILE_V30};
    const x86_core_cpu_debug_register_patch entry={
        .mask=X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_CS) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_EIP),
        .values={[X86_CORE_CPU_DEBUG_EIP]=0x100u}};
    const lib_u8 code[]={0x9bu,0x90u,0xf4u,0x90u,0x90u,0x90u};
    v30_time_probe probe={.blocked=LIB_TRUE};
    const x86_core_machine_attachment attachment={.context=&probe,
        .wait_test_asserted=v30_time_poll,.after_internal_advance=v30_time_advance,
        .finalize_devices=v30_time_finalize};
    x86_core_machine *machine=LIB_NULL;
    x86_core_machine_run_result result={0};
    x86_core_machine_time_observation observation;
    x86_core_cpu_state cpu;
    lib_u64 total=0u, retired=0u;
    lib_i32 status=1;

    config.transaction_contract.external_access_wait_windows[0]=
        (x86_core_machine_external_access_wait_window){
            X86_CORE_CPU_EXTERNAL_CYCLE_SPACE_MEMORY,0x100u,0x105u,wait};
    if (x86_core_machine_neutral_create(&config,&machine)!=LIB_STATUS_OK) return 1;
    if (x86_core_machine_bind_attachment(machine,&attachment)!=LIB_STATUS_OK ||
        x86_core_machine_freeze_execution_providers(machine)!=LIB_STATUS_OK ||
        x86_core_machine_reset(machine)!=LIB_STATUS_OK ||
        x86_core_machine_set_retirement_observation_provider(machine,
            &(x86_core_machine_retirement_observation_provider){v30_time_retired,&probe})!=LIB_STATUS_OK ||
        x86_core_machine_debug_patch_registers(machine,&entry)!=LIB_STATUS_OK ||
        x86_core_machine_memory_write(machine,0x100u,code,sizeof(code))!=LIB_STATUS_OK)
        goto cleanup;
    for (lib_u8 part=0u; part<32u && retired==0u; ++part) {
        const lib_u64 budget=stage==1u ? 1u : stage==2u && part==0u ? 7u : 0u;
        if (x86_core_machine_run(machine,(x86_core_machine_run_budget){1u,budget},
                &result)!=LIB_STATUS_OK || (budget && result.ticks>budget)) goto cleanup;
        total+=result.ticks;
        retired+=result.executed;
        if (probe.samples!=1u || probe.ticks!=total || result.elapsed_ticks!=total ||
            x86_core_machine_capture_time_observation(machine,&observation)!=LIB_STATUS_OK ||
            (retired==0u && observation.next_deadline_valid)) goto cleanup;
    }
    if (retired!=1u || total!=19u+3u*wait || probe.retired!=1u || probe.source_ticks!=19u ||
        result.reason!=X86_CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT ||
        !observation.next_deadline_valid || observation.next_deadline_tick!=total+5u ||
        x86_core_machine_get_cpu_state(machine,&cpu)!=LIB_STATUS_OK || !cpu.halted ||
        x86_core_machine_advance_time(machine,4u)!=LIB_STATUS_OK || probe.samples!=1u)
        goto cleanup;
    probe.blocked=LIB_FALSE;
    if (x86_core_machine_advance_time(machine,1u)!=LIB_STATUS_OK || probe.samples!=2u ||
        probe.ticks!=total+5u ||
        x86_core_machine_get_cpu_state(machine,&cpu)!=LIB_STATUS_OK || cpu.halted ||
        x86_core_machine_capture_time_observation(machine,&observation)!=LIB_STATUS_OK ||
        observation.next_deadline_valid) goto cleanup;
    if (x86_core_machine_run(machine,(x86_core_machine_run_budget){1u,0u},&result)!=LIB_STATUS_OK ||
        result.executed!=1u || result.ticks!=3u || result.elapsed_ticks!=total+8u ||
        result.linear_pc!=0x102u || probe.samples!=2u || probe.retired!=2u ||
        probe.source_ticks!=3u) goto cleanup;
    if (x86_core_machine_run(machine,(x86_core_machine_run_budget){1u,0u},&result)!=LIB_STATUS_OK ||
        result.executed!=1u || result.ticks!=2u || result.elapsed_ticks!=total+10u ||
        result.reason!=X86_CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT || probe.retired!=3u ||
        probe.source_ticks!=2u ||
        x86_core_machine_run(machine,(x86_core_machine_run_budget){1u,0u},&result)!=LIB_STATUS_OK ||
        result.executed || result.ticks || result.elapsed_ticks!=total+10u || probe.retired!=3u ||
        x86_core_machine_advance_time(machine,11u)!=LIB_STATUS_OK ||
        probe.ticks!=total+21u || probe.samples!=2u || probe.retired!=3u) goto cleanup;
    status=0;
cleanup:
    if (status) lib_c_printf("V30:TIME-RECEIVER:POLL:FAIL stage=%u wait=%u ticks=%llu samples=%u retired=%llu reason=%u\n",
        stage,wait,total,probe.samples,retired,(lib_u32)result.reason);
    x86_core_machine_destroy(machine);
    return status || probe.finalized!=1u;
}

/* Own-component failure injection at the existing cycle receiver, on a
 * genuinely constructed Machine. CANCEL must retain prior accepted waits. */
static lib_i32 v30_time_wait_failures(void)
{
    x86_core_machine_executor_config config={
        .memory_bytes=0x100000u,.cpu_profile=X86_CORE_CPU_PROFILE_V30};
    x86_core_machine *machine=LIB_NULL;
    lib_i32 status=1;
    config.transaction_contract.external_access_wait_windows[0]=
        (x86_core_machine_external_access_wait_window){
            X86_CORE_CPU_EXTERNAL_CYCLE_SPACE_PORT,0x100u,0x1ffu,7u};
    if (x86_core_machine_neutral_create(&config,&machine)!=LIB_STATUS_OK) return 1;
    if (x86_core_machine_freeze_execution_providers(machine)!=LIB_STATUS_OK ||
        x86_core_machine_reset(machine)!=LIB_STATUS_OK) goto cleanup;
    for (lib_u8 cancel=0u; cancel<2u; ++cancel) {
        if (x86_core_machine_cpu_external_cycle_trace(machine,
            X86_CORE_CPU_EXTERNAL_CYCLE_PHASE_BEGIN,X86_CORE_CPU_EXTERNAL_CYCLE_SPACE_PORT,
            0x100u,2u,LIB_FALSE,X86_CORE_CPU_MEMORY_ACCESS_DATA)) goto cleanup;
        const lib_u64 accepted=x86_core_machine_cpu_external_cycle_trace(machine,
            cancel ? X86_CORE_CPU_EXTERNAL_CYCLE_PHASE_CANCEL : X86_CORE_CPU_EXTERNAL_CYCLE_PHASE_COMMIT,
            X86_CORE_CPU_EXTERNAL_CYCLE_SPACE_PORT,0x100u,2u,LIB_FALSE,X86_CORE_CPU_MEMORY_ACCESS_DATA);
        if (accepted!=(cancel ? 0u : 7u) || machine->external_cycle_round_ticks!=7u ||
            machine->external_cycle_pending_valid) goto cleanup;
    }
    machine->external_cycle_round_ticks=LIB_UINT64_MAX-1u;
    (void)x86_core_machine_cpu_external_cycle_trace(machine,
        X86_CORE_CPU_EXTERNAL_CYCLE_PHASE_BEGIN,X86_CORE_CPU_EXTERNAL_CYCLE_SPACE_PORT,
        0x100u,1u,LIB_TRUE,X86_CORE_CPU_MEMORY_ACCESS_DATA);
    if (x86_core_machine_cpu_external_cycle_trace(machine,
        X86_CORE_CPU_EXTERNAL_CYCLE_PHASE_COMMIT,X86_CORE_CPU_EXTERNAL_CYCLE_SPACE_PORT,
        0x100u,1u,LIB_TRUE,X86_CORE_CPU_MEMORY_ACCESS_DATA)!=7u ||
        !machine->external_cycle_round_overflow || machine->external_cycle_round_ticks!=LIB_UINT64_MAX-1u)
        goto cleanup;
    if (x86_core_machine_reset(machine)!=LIB_STATUS_OK || machine->external_cycle_round_ticks ||
        machine->external_cycle_round_overflow || machine->external_cycle_pending_valid) goto cleanup;
    status=0;
cleanup:
    x86_core_machine_destroy(machine);
    return status;
}

lib_i32 main(void)
{
    for (lib_u8 stage=0u; stage<3u; ++stage)
        for (lib_u8 wait=0u; wait<=2u; wait+=2u)
            if (v30_time_poll_receiver(stage,wait)) return 1;
    if (v30_time_wait_failures()) { lib_c_printf("V30:TIME-RECEIVER:WAIT:FAIL\n"); return 1; }
    lib_c_printf("V30:TIME-RECEIVER:OK\n");
    return 0;
}
