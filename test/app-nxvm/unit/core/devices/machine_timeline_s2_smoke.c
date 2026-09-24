#include "lib/types/types_interface.h"
#include <stdio.h>

#include "app-nxvm/devices/machine.h"
#include "app-nxvm/devices/machine_interface.h"
#include "app-nxvm/devices/timeline.h"
#include "support/core_machine_cpu_fixture.h"

typedef struct timeline_probe {
    core_machine_timeline *timeline;
    lib_u8 order[5];
    lib_u32 count;
    core_machine_timeline_token nested;
} timeline_probe;

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

static void timeline_record(timeline_probe *probe, lib_u8 value)
{
    if (probe != LIB_NULL && probe->count < sizeof(probe->order)) {
        probe->order[probe->count++] = value;
    }
}

static void timeline_a(void *opaque, lib_u64 due_tick)
{
    (void)due_tick;
    timeline_record((timeline_probe *)opaque, 1u);
}

static void timeline_b(void *opaque, lib_u64 due_tick)
{
    timeline_probe *probe = (timeline_probe *)opaque;

    timeline_record(probe, 2u);
    if (probe != LIB_NULL) {
        (void)core_machine_timeline_schedule(probe->timeline, due_tick,
            timeline_a, probe, &probe->nested);
    }
}

static void timeline_c(void *opaque, lib_u64 due_tick)
{
    (void)due_tick;
    timeline_record((timeline_probe *)opaque, 3u);
}

static void timeline_cancelled(void *opaque, lib_u64 due_tick)
{
    (void)due_tick;
    timeline_record((timeline_probe *)opaque, 4u);
}

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
    failed |= core_machine_create(&config, &machine) != LIB_STATUS_OK;
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
    failed |= result.reason != CORE_MACHINE_STOP_BUDGET || result.elapsed_ticks != 3u;
    failed |= core_machine_get_timeline_observation(machine, &observation) !=
        LIB_STATUS_OK;
    failed |= observation.now != 3u || observation.pending_events != 0u ||
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
    core_machine_timeline timeline;
    core_machine_timeline_token first;
    core_machine_timeline_token second;
    core_machine_timeline_token cancelled;
    timeline_probe probe = { 0 };
    lib_u64 due_tick = 0u;
    lib_i32 failed = 0;

    probe.timeline = &timeline;
    failed |= core_machine_timeline_initialize(&timeline) != LIB_STATUS_OK;
    failed |= core_machine_timeline_schedule(&timeline, 10u, timeline_a, &probe,
        &first) != LIB_STATUS_OK;
    failed |= core_machine_timeline_next_due(&timeline, &due_tick) != LIB_STATUS_OK ||
        due_tick != 10u;
    failed |= core_machine_timeline_schedule(&timeline, 10u, timeline_c, &probe,
        &second) != LIB_STATUS_OK;
    failed |= core_machine_timeline_schedule(&timeline, 5u, timeline_b, &probe,
        &probe.nested) != LIB_STATUS_OK;
    failed |= core_machine_timeline_next_due(&timeline, &due_tick) != LIB_STATUS_OK ||
        due_tick != 5u;
    failed |= core_machine_timeline_schedule(&timeline, 8u, timeline_cancelled,
        &probe, &cancelled) != LIB_STATUS_OK;
    failed |= core_machine_timeline_cancel(&timeline, &cancelled) != LIB_STATUS_OK;
    failed |= core_machine_timeline_advance(&timeline, 10u) != LIB_STATUS_OK;
    failed |= timeline.now != 10u || core_machine_timeline_pending_count(&timeline) != 0u ||
        probe.count != 4u || probe.order[0] != 2u || probe.order[1] != 1u ||
        probe.order[2] != 1u || probe.order[3] != 3u;
    failed |= core_machine_timeline_next_due(&timeline, &due_tick) != LIB_STATUS_INVALID_STATE;
    failed |= core_machine_timeline_schedule(&timeline, 9u, timeline_a, &probe,
        &first) != LIB_STATUS_INVALID_ARGUMENT;
    failed |= core_machine_timeline_schedule(&timeline, 12u, timeline_cancelled,
        &probe, &cancelled) != LIB_STATUS_OK;
    core_machine_timeline_reset(&timeline);
    failed |= core_machine_timeline_advance(&timeline, 20u) != LIB_STATUS_OK ||
        probe.count != 4u || timeline.now != 20u;
    failed |= timeline_machine_contract();

    if (failed) return 1;
    printf("M5:T346:S2:TIMELINE:OK\n");
    return 0;
}
