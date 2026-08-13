#include "type.h"

#include "core/machine/machine_interface.h"
#include "core/machine/timeline.h"
#include "../support/core_machine_cpu_fixture.h"

typedef struct timeline_probe {
    core_machine_timeline *timeline;
    type_unsigned_8 order[5];
    type_unsigned_32 count;
    core_machine_timeline_token nested;
} timeline_probe;

typedef struct timeline_trace_probe {
    core_machine_trace_event events[8];
    type_unsigned_32 count;
} timeline_trace_probe;

static C_VOID timeline_trace(C_VOID *opaque,
    const core_machine_trace_event *event)
{
    timeline_trace_probe *probe = (timeline_trace_probe *)opaque;

    if (probe != STD_NULL && probe->count < 8u) {
        probe->events[probe->count++] = *event;
    }
}

static C_VOID timeline_record(timeline_probe *probe, type_unsigned_8 value)
{
    if (probe != STD_NULL && probe->count < sizeof(probe->order)) {
        probe->order[probe->count++] = value;
    }
}

static C_VOID timeline_a(C_VOID *opaque, type_unsigned_64 due_tick)
{
    (C_VOID)due_tick;
    timeline_record((timeline_probe *)opaque, 1u);
}

static C_VOID timeline_b(C_VOID *opaque, type_unsigned_64 due_tick)
{
    timeline_probe *probe = (timeline_probe *)opaque;

    timeline_record(probe, 2u);
    if (probe != STD_NULL) {
        (C_VOID)core_machine_timeline_schedule(probe->timeline, due_tick,
            timeline_a, probe, &probe->nested);
    }
}

static C_VOID timeline_c(C_VOID *opaque, type_unsigned_64 due_tick)
{
    (C_VOID)due_tick;
    timeline_record((timeline_probe *)opaque, 3u);
}

static C_VOID timeline_cancelled(C_VOID *opaque, type_unsigned_64 due_tick)
{
    (C_VOID)due_tick;
    timeline_record((timeline_probe *)opaque, 4u);
}

static C_INT timeline_machine_contract(C_VOID)
{
    core_machine *machine = STD_NULL;
    core_machine_config config = { 0 };
    core_machine_run_budget budget = { 1u, 0u };
    core_machine_run_result result;
    core_machine_timeline_observation observation;
    core_machine_trace_provider trace = { timeline_trace, STD_NULL };
    timeline_trace_probe trace_probe = { { { 0 } }, 0u };
    const type_unsigned_8 nop = 0x90u;
    C_INT failed = 0;

    trace.context = &trace_probe;
    failed |= core_machine_create(&config, &machine) != TYPE_STATUS_OK;
    failed |= test_core_machine_fixture_register_reset_mapping(machine, 0xfffffff0u,
        0x000ffff0u, 1u) != TYPE_STATUS_OK;
    failed |= core_machine_freeze_execution_providers(machine) != TYPE_STATUS_OK;
    failed |= core_machine_reset(machine) != TYPE_STATUS_OK;
    failed |= core_machine_memory_write(machine, 0xfffffff0u, &nop, 1u) !=
        TYPE_STATUS_OK;
    failed |= core_machine_set_trace_provider(machine, &trace) != TYPE_STATUS_OK;
    failed |= core_machine_run(machine, budget, &result) != TYPE_STATUS_OK;
    failed |= result.reason != CORE_MACHINE_STOP_BUDGET || result.elapsed_ticks != 1u;
    failed |= core_machine_get_timeline_observation(machine, &observation) !=
        TYPE_STATUS_OK;
    failed |= observation.now != 1u || observation.pending_events != 1u ||
        observation.next_sequence != 2u;
    failed |= trace_probe.count != 5u ||
        trace_probe.events[0].type != CORE_MACHINE_TRACE_CPU_RETIRE ||
        trace_probe.events[0].elapsed_ticks != 1u ||
        trace_probe.events[0].timeline_ticks != 0u ||
        trace_probe.events[0].value != 1u ||
        trace_probe.events[1].type != CORE_MACHINE_TRACE_DMA_ADVANCE ||
        trace_probe.events[1].elapsed_ticks != 1u ||
        trace_probe.events[1].timeline_ticks != 1u ||
        trace_probe.events[1].value != 1u ||
        trace_probe.events[2].type != CORE_MACHINE_TRACE_PIT_ADVANCE ||
        trace_probe.events[2].elapsed_ticks != 1u ||
        trace_probe.events[2].timeline_ticks != 1u ||
        trace_probe.events[2].value != 1u ||
        trace_probe.events[3].type != CORE_MACHINE_TRACE_PIC_REFRESH ||
        trace_probe.events[3].elapsed_ticks != 1u ||
        trace_probe.events[3].timeline_ticks != 1u ||
        trace_probe.events[4].type != CORE_MACHINE_TRACE_RUN_BOUNDARY ||
        trace_probe.events[4].elapsed_ticks != 1u ||
        trace_probe.events[4].timeline_ticks != 1u ||
        trace_probe.events[0].sequence >= trace_probe.events[1].sequence ||
        trace_probe.events[1].sequence >= trace_probe.events[2].sequence ||
        trace_probe.events[2].sequence >= trace_probe.events[3].sequence ||
        trace_probe.events[3].sequence >= trace_probe.events[4].sequence;
    core_machine_destroy(machine);
    return failed;
}

C_INT main(C_VOID)
{
    core_machine_timeline timeline;
    core_machine_timeline_token first;
    core_machine_timeline_token second;
    core_machine_timeline_token cancelled;
    timeline_probe probe = { 0 };
    C_INT failed = 0;

    probe.timeline = &timeline;
    failed |= core_machine_timeline_initialize(&timeline) != TYPE_STATUS_OK;
    failed |= core_machine_timeline_schedule(&timeline, 10u, timeline_a, &probe,
        &first) != TYPE_STATUS_OK;
    failed |= core_machine_timeline_schedule(&timeline, 10u, timeline_c, &probe,
        &second) != TYPE_STATUS_OK;
    failed |= core_machine_timeline_schedule(&timeline, 5u, timeline_b, &probe,
        &probe.nested) != TYPE_STATUS_OK;
    failed |= core_machine_timeline_schedule(&timeline, 8u, timeline_cancelled,
        &probe, &cancelled) != TYPE_STATUS_OK;
    failed |= core_machine_timeline_cancel(&timeline, &cancelled) != TYPE_STATUS_OK;
    failed |= core_machine_timeline_advance(&timeline, 10u) != TYPE_STATUS_OK;
    failed |= timeline.now != 10u || core_machine_timeline_pending_count(&timeline) != 0u ||
        probe.count != 4u || probe.order[0] != 2u || probe.order[1] != 1u ||
        probe.order[2] != 1u || probe.order[3] != 3u;
    failed |= core_machine_timeline_schedule(&timeline, 9u, timeline_a, &probe,
        &first) != TYPE_STATUS_INVALID_ARGUMENT;
    failed |= core_machine_timeline_schedule(&timeline, 12u, timeline_cancelled,
        &probe, &cancelled) != TYPE_STATUS_OK;
    core_machine_timeline_reset(&timeline);
    failed |= core_machine_timeline_advance(&timeline, 20u) != TYPE_STATUS_OK ||
        probe.count != 4u || timeline.now != 20u;
    failed |= timeline_machine_contract();

    if (failed) return 1;
    STD_PRINTF("M5:T346:S2:TIMELINE:OK\n");
    return 0;
}
