#include "lib/types/file.h"
#include "lib/types/types_interface.h"
#include "ibmpc/board-common/machine_board_interface.h"
#include "x86/core/machine.h"
#include "x86/core/machine_interface.h"
#include "x86/core/timeline.h"
#include "../board-common/core_machine_board_fixture.h"


#if CORE_MACHINE_RUNTIME_TRACE_ENABLED
typedef struct timeline_trace_probe {
    core_machine_trace_event events[40];
    lib_u32 count;
} timeline_trace_probe;

static void timeline_trace(void *opaque,
    const core_machine_trace_event *event)
{
    timeline_trace_probe *probe = (timeline_trace_probe *)opaque;

    if (probe != LIB_NULL && probe->count <
        sizeof(probe->events) / sizeof(probe->events[0])) {
        probe->events[probe->count++] = *event;
    }
}

static const core_machine_trace_event *timeline_find_trace_event(
    const timeline_trace_probe *probe, core_machine_trace_event_type type)
{
    lib_u32 index;

    for (index = 0u; index < probe->count; ++index) {
        if (probe->events[index].type == type) return &probe->events[index];
    }
    return LIB_NULL;
}
#endif

static void timeline_a(void *opaque, lib_u64 due_tick)
{ (void)opaque; (void)due_tick; }

static lib_i32 timeline_machine_contract(void)
{
    core_machine *machine = LIB_NULL;
    core_machine_config config = { 0 };
    core_machine_run_budget budget = { 1u, 0u };
    core_machine_run_result result;
    core_machine_timeline_observation observation;
    core_machine_time_observation time_observation;
    core_machine_timeline_token immediate;
#if CORE_MACHINE_RUNTIME_TRACE_ENABLED
    core_machine_trace_provider trace = { timeline_trace, LIB_NULL };
    timeline_trace_probe trace_probe = { { { 0 } }, 0u };
#endif
    const lib_u8 nop = 0x90u;
    lib_i32 failed = 0;

    config.cpu_profile = CORE_MACHINE_CPU_PROFILE_80286;
#if CORE_MACHINE_RUNTIME_TRACE_ENABLED
    trace.context = &trace_probe;
#endif
    if (core_machine_create(&config, &machine, LIB_NULL) != LIB_STATUS_OK) return 1;
    failed |= test_core_machine_fixture_register_reset_mapping(machine, 0x00fffff0u,
        0x000ffff0u, 16u) != LIB_STATUS_OK;
    failed |= core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK;
    failed |= core_machine_reset(machine) != LIB_STATUS_OK;
    failed |= core_machine_memory_write(machine, 0x00fffff0u, &nop, 1u) !=
        LIB_STATUS_OK;
    failed |= core_machine_timeline_schedule(&machine->timeline, 0u, timeline_a,
        LIB_NULL, &immediate) != LIB_STATUS_OK;
    failed |= core_machine_capture_time_observation(machine, &time_observation) !=
        LIB_STATUS_OK || time_observation.next_deadline_valid ||
        time_observation.progress_disposition != CORE_MACHINE_TIME_PROGRESS_IMMEDIATE;
#if CORE_MACHINE_RUNTIME_TRACE_ENABLED
    failed |= core_machine_set_trace_provider(machine, &trace) != LIB_STATUS_OK;
#endif
    failed |= core_machine_run(machine, budget, &result) != LIB_STATUS_OK;
    if (!failed) failed |= result.reason != CORE_MACHINE_STOP_BUDGET || result.elapsed_ticks != 3u;
    failed |= core_machine_get_timeline_observation(machine, &observation) !=
        LIB_STATUS_OK;
    if (!failed) failed |= observation.now != 3u || observation.pending_events != 0u ||
        observation.next_sequence != 1u;
    /* Release intentionally omits development trace recording; the state and
     * ordering contract below remains a Debug-only observation. */
#if CORE_MACHINE_RUNTIME_TRACE_ENABLED
    {
        const core_machine_trace_event *retire = timeline_find_trace_event(
            &trace_probe, CORE_MACHINE_TRACE_CPU_RETIRE);
        const core_machine_trace_event *dma = timeline_find_trace_event(
            &trace_probe, CORE_MACHINE_TRACE_DMA_ADVANCE);
        const core_machine_trace_event *pit = timeline_find_trace_event(
            &trace_probe, CORE_MACHINE_TRACE_PIT_ADVANCE);
        const core_machine_trace_event *pic = timeline_find_trace_event(
            &trace_probe, CORE_MACHINE_TRACE_PIC_REFRESH);
        const core_machine_trace_event *boundary = timeline_find_trace_event(
            &trace_probe, CORE_MACHINE_TRACE_RUN_BOUNDARY);

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
    core_machine_destroy(machine);
    return failed;
}

lib_i32 main(void)
{
    if (timeline_machine_contract()) return 1;
    lib_c_printf("TIMELINE:OK\n");
    return 0;
}
