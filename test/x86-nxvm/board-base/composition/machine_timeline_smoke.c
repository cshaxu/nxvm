#include "lib/types/file.h"
#include "lib/types/types_interface.h"
#include "x86-nxvm/board-base/machine_board_interface.h"
#include "x86-core/machine/machine_interface.h"
#include "x86-nxvm/board-base/trace_interface.h"


#if X86_CORE_MACHINE_RUNTIME_TRACE_ENABLED
typedef struct timeline_trace_probe {
    x86_core_machine_trace_event events[40];
    lib_u32 count;
} timeline_trace_probe;

static void timeline_trace(void *opaque,
    const x86_core_machine_trace_event *event)
{
    timeline_trace_probe *probe = (timeline_trace_probe *)opaque;

    if (probe != LIB_NULL && probe->count <
        sizeof(probe->events) / sizeof(probe->events[0])) {
        probe->events[probe->count++] = *event;
    }
}

static const x86_core_machine_trace_event *timeline_find_trace_event(
    const timeline_trace_probe *probe, lib_u32 type)
{
    lib_u32 index;

    for (index = 0u; index < probe->count; ++index) {
        if (probe->events[index].type == type) return &probe->events[index];
    }
    return LIB_NULL;
}
#endif

static lib_i32 timeline_machine_contract(void)
{
    x86_core_machine *machine = LIB_NULL;
    x86_nxvm_board_config config = { 0 };
    x86_core_machine_run_budget budget = { 1u, 0u };
    x86_core_machine_run_result result;
    x86_core_machine_timeline_observation observation;
    const x86_core_machine_memory_alias_config reset_alias = {
        0x00fffff0u, 0x000ffff0u, 16u
    };
#if X86_CORE_MACHINE_RUNTIME_TRACE_ENABLED
    x86_core_machine_trace_provider trace = { timeline_trace, LIB_NULL };
    timeline_trace_probe trace_probe = { { { 0 } }, 0u };
#endif
    const lib_u8 nop = 0x90u;
    lib_i32 failed = 0;

    config.cpu_profile = X86_CORE_CPU_PROFILE_80286;
#if X86_CORE_MACHINE_RUNTIME_TRACE_ENABLED
    trace.context = &trace_probe;
#endif
    if (x86_nxvm_board_create(&config, &machine, LIB_NULL) != LIB_STATUS_OK) return 1;
    failed |= x86_core_machine_install_memory_aliases(machine, &reset_alias, 1u,
        LIB_FALSE) != LIB_STATUS_OK;
    failed |= x86_core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK;
    failed |= x86_core_machine_reset(machine) != LIB_STATUS_OK;
    failed |= x86_core_machine_memory_write(machine, 0x00fffff0u, &nop, 1u) !=
        LIB_STATUS_OK;
#if X86_CORE_MACHINE_RUNTIME_TRACE_ENABLED
    failed |= x86_core_machine_set_trace_provider(machine, &trace) != LIB_STATUS_OK;
#endif
    failed |= x86_core_machine_run(machine, budget, &result) != LIB_STATUS_OK;
    if (!failed) failed |= result.reason != X86_CORE_MACHINE_STOP_BUDGET || result.elapsed_ticks != 3u;
    failed |= x86_core_machine_get_timeline_observation(machine, &observation) !=
        LIB_STATUS_OK;
    if (!failed) failed |= observation.now != 3u || observation.pending_events != 0u ||
        observation.next_sequence != 0u;
    /* Release intentionally omits development trace recording; the state and
     * ordering contract below remains a Debug-only observation. */
#if X86_CORE_MACHINE_RUNTIME_TRACE_ENABLED
    {
        const x86_core_machine_trace_event *retire = timeline_find_trace_event(
            &trace_probe, X86_CORE_MACHINE_TRACE_CPU_RETIRE);
        const x86_core_machine_trace_event *dma = timeline_find_trace_event(
            &trace_probe, X86_CORE_MACHINE_TRACE_DMA_ADVANCE);
        const x86_core_machine_trace_event *pit = timeline_find_trace_event(
            &trace_probe, X86_NXVM_BOARD_TRACE_PIT_ADVANCE);
        const x86_core_machine_trace_event *pic = timeline_find_trace_event(
            &trace_probe, X86_NXVM_BOARD_TRACE_PIC_REFRESH);
        const x86_core_machine_trace_event *boundary = timeline_find_trace_event(
            &trace_probe, X86_CORE_MACHINE_TRACE_RUN_BOUNDARY);

        failed |= retire == LIB_NULL || dma == LIB_NULL || pit == LIB_NULL ||
            pic == LIB_NULL || boundary == LIB_NULL || retire->elapsed_ticks != 0u ||
            retire->timeline_ticks != 0u || retire->value != 3u ||
            dma->elapsed_ticks != 3u || dma->timeline_ticks != 3u ||
            dma->value != 3u || pit->elapsed_ticks != 3u ||
            pit->timeline_ticks != 3u || pit->value != 3u ||
            pic->elapsed_ticks != 3u || pic->timeline_ticks != 3u ||
            boundary->elapsed_ticks != 3u || boundary->timeline_ticks != 3u ||
            retire->sequence >= dma->sequence || dma->sequence >= pit->sequence ||
            pit->sequence >= pic->sequence || pic->sequence >= boundary->sequence;
    }
#endif
    x86_core_machine_destroy(machine);
    return failed;
}

lib_i32 main(void)
{
    if (timeline_machine_contract()) return 1;
    lib_c_printf("TIMELINE:OK\n");
    return 0;
}
