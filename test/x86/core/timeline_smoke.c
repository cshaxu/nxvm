#include "lib/types/file.h"
#include "x86/core/timeline.h"

typedef struct timeline_probe {
    core_machine_timeline *timeline;
    lib_u8 order[5];
    lib_u32 count;
    core_machine_timeline_token nested;
} timeline_probe;

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

static void timeline_count(void *opaque, lib_u64 due_tick)
{ (void)due_tick; ++((timeline_probe *)opaque)->count; }

static lib_i32 timeline_failure_contract(void)
{
    core_machine_timeline timeline;
    core_machine_timeline_token tokens[CORE_MACHINE_TIMELINE_EVENT_CAPACITY];
    core_machine_timeline_token rejected = { LIB_UINT32_MAX, LIB_UINT64_MAX };
    timeline_probe probe = {0};
    lib_u64 due = 123u;
    lib_i32 failed = 0;
    failed |= core_machine_timeline_initialize(LIB_NULL) != LIB_STATUS_INVALID_ARGUMENT;
    core_machine_timeline_reset(LIB_NULL);
    failed |= core_machine_timeline_pending_count(LIB_NULL) != 0u;
    failed |= core_machine_timeline_initialize(&timeline) != LIB_STATUS_OK;
    failed |= core_machine_timeline_next_due(&timeline, &due) != LIB_STATUS_INVALID_STATE || due != 123u;
    failed |= core_machine_timeline_schedule(&timeline, 0u, LIB_NULL, &probe, &rejected) != LIB_STATUS_INVALID_ARGUMENT;
    failed |= core_machine_timeline_schedule(&timeline, 0u, timeline_count, &probe, LIB_NULL) != LIB_STATUS_INVALID_ARGUMENT;
    failed |= core_machine_timeline_cancel(&timeline, LIB_NULL) != LIB_STATUS_INVALID_ARGUMENT;
    failed |= core_machine_timeline_cancel(&timeline, &rejected) != LIB_STATUS_INVALID_ARGUMENT;
    for (lib_u32 i = 0u; i < CORE_MACHINE_TIMELINE_EVENT_CAPACITY; ++i)
        failed |= core_machine_timeline_schedule(&timeline, CORE_MACHINE_TIMELINE_EVENT_CAPACITY - i,
            timeline_count, &probe, &tokens[i]) != LIB_STATUS_OK;
    failed |= core_machine_timeline_schedule(&timeline, 0u, timeline_count, &probe, &rejected) != LIB_STATUS_NO_MEMORY;
    failed |= rejected.slot != LIB_UINT32_MAX || rejected.sequence != LIB_UINT64_MAX ||
        core_machine_timeline_pending_count(&timeline) != CORE_MACHINE_TIMELINE_EVENT_CAPACITY;
    failed |= core_machine_timeline_cancel(&timeline, &tokens[0]) != LIB_STATUS_OK;
    failed |= core_machine_timeline_cancel(&timeline, &tokens[0]) != LIB_STATUS_INVALID_STATE;
    failed |= core_machine_timeline_schedule(&timeline, 1u, timeline_count, &probe, &rejected) != LIB_STATUS_OK;
    failed |= core_machine_timeline_cancel(&timeline, &tokens[0]) != LIB_STATUS_INVALID_STATE;
    failed |= core_machine_timeline_advance(&timeline, CORE_MACHINE_TIMELINE_EVENT_CAPACITY) != LIB_STATUS_OK ||
        probe.count != CORE_MACHINE_TIMELINE_EVENT_CAPACITY || core_machine_timeline_pending_count(&timeline) != 0u;
    failed |= core_machine_timeline_advance(&timeline, 1u) != LIB_STATUS_INVALID_ARGUMENT ||
        timeline.now != CORE_MACHINE_TIMELINE_EVENT_CAPACITY;
    timeline.next_sequence = LIB_UINT64_MAX;
    rejected = (core_machine_timeline_token){ LIB_UINT32_MAX, LIB_UINT64_MAX };
    failed |= core_machine_timeline_schedule(&timeline, timeline.now, timeline_count, &probe, &rejected) != LIB_STATUS_INVALID_ARGUMENT;
    failed |= rejected.slot != LIB_UINT32_MAX || core_machine_timeline_pending_count(&timeline) != 0u;
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
    lib_i32 failed = timeline_failure_contract();

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

    if (failed) return 1;
    lib_c_printf("X86:TIMELINE:OK\n");
    return 0;
}
