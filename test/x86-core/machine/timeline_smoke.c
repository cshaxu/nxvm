#include "lib/types/file.h"
#include "x86-core/machine/timeline.h"
#include "x86-core/machine/machine.h"

typedef struct timeline_probe {
    x86_core_machine_timeline *timeline;
    lib_u8 order[5];
    lib_u32 count;
    x86_core_machine_timeline_token nested;
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
        (void)x86_core_machine_timeline_schedule(probe->timeline, due_tick,
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
    x86_core_machine_timeline timeline;
    x86_core_machine_timeline_token tokens[X86_CORE_MACHINE_TIMELINE_EVENT_CAPACITY];
    x86_core_machine_timeline_token rejected = { LIB_UINT32_MAX, LIB_UINT64_MAX };
    timeline_probe probe = {0};
    lib_u64 due = 123u;
    lib_i32 failed = 0;
    failed |= x86_core_machine_timeline_initialize(LIB_NULL) != LIB_STATUS_INVALID_ARGUMENT;
    x86_core_machine_timeline_reset(LIB_NULL);
    failed |= x86_core_machine_timeline_pending_count(LIB_NULL) != 0u;
    failed |= x86_core_machine_timeline_initialize(&timeline) != LIB_STATUS_OK;
    failed |= x86_core_machine_timeline_next_due(&timeline, &due) != LIB_STATUS_INVALID_STATE || due != 123u;
    failed |= x86_core_machine_timeline_schedule(&timeline, 0u, LIB_NULL, &probe, &rejected) != LIB_STATUS_INVALID_ARGUMENT;
    failed |= x86_core_machine_timeline_schedule(&timeline, 0u, timeline_count, &probe, LIB_NULL) != LIB_STATUS_INVALID_ARGUMENT;
    failed |= x86_core_machine_timeline_cancel(&timeline, LIB_NULL) != LIB_STATUS_INVALID_ARGUMENT;
    failed |= x86_core_machine_timeline_cancel(&timeline, &rejected) != LIB_STATUS_INVALID_ARGUMENT;
    for (lib_u32 i = 0u; i < X86_CORE_MACHINE_TIMELINE_EVENT_CAPACITY; ++i)
        failed |= x86_core_machine_timeline_schedule(&timeline, X86_CORE_MACHINE_TIMELINE_EVENT_CAPACITY - i,
            timeline_count, &probe, &tokens[i]) != LIB_STATUS_OK;
    failed |= x86_core_machine_timeline_schedule(&timeline, 0u, timeline_count, &probe, &rejected) != LIB_STATUS_NO_MEMORY;
    failed |= rejected.slot != LIB_UINT32_MAX || rejected.sequence != LIB_UINT64_MAX ||
        x86_core_machine_timeline_pending_count(&timeline) != X86_CORE_MACHINE_TIMELINE_EVENT_CAPACITY;
    failed |= x86_core_machine_timeline_cancel(&timeline, &tokens[0]) != LIB_STATUS_OK;
    failed |= x86_core_machine_timeline_cancel(&timeline, &tokens[0]) != LIB_STATUS_INVALID_STATE;
    failed |= x86_core_machine_timeline_schedule(&timeline, 1u, timeline_count, &probe, &rejected) != LIB_STATUS_OK;
    failed |= x86_core_machine_timeline_cancel(&timeline, &tokens[0]) != LIB_STATUS_INVALID_STATE;
    failed |= x86_core_machine_timeline_advance(&timeline, X86_CORE_MACHINE_TIMELINE_EVENT_CAPACITY) != LIB_STATUS_OK ||
        probe.count != X86_CORE_MACHINE_TIMELINE_EVENT_CAPACITY || x86_core_machine_timeline_pending_count(&timeline) != 0u;
    failed |= x86_core_machine_timeline_advance(&timeline, 1u) != LIB_STATUS_INVALID_ARGUMENT ||
        timeline.now != X86_CORE_MACHINE_TIMELINE_EVENT_CAPACITY;
    timeline.next_sequence = LIB_UINT64_MAX;
    rejected = (x86_core_machine_timeline_token){ LIB_UINT32_MAX, LIB_UINT64_MAX };
    failed |= x86_core_machine_timeline_schedule(&timeline, timeline.now, timeline_count, &probe, &rejected) != LIB_STATUS_INVALID_ARGUMENT;
    failed |= rejected.slot != LIB_UINT32_MAX || x86_core_machine_timeline_pending_count(&timeline) != 0u;
    return failed;
}

static lib_i32 timeline_execution_boundary(void)
{
    const x86_core_machine_executor_config config = {
        .memory_bytes = 2u * 1024u * 1024u,
        .cpu_profile = X86_CORE_CPU_PROFILE_80286
    };
    const x86_core_machine_memory_alias_config reset_alias = {
        0x00fffff0u, 0x000ffff0u, 16u
    };
    const lib_u8 nop = 0x90u;
    x86_core_machine *machine = LIB_NULL;
    x86_core_machine_timeline_token immediate;
    x86_core_machine_time_observation time;
    x86_core_machine_timeline_observation observation;
    x86_core_machine_run_result run;
    timeline_probe probe = {0};
    lib_i32 failed = 1;

    if (x86_core_machine_neutral_create(&config, &machine) != LIB_STATUS_OK)
        return 1;
    if (x86_core_machine_install_memory_aliases(machine, &reset_alias, 1u,
            LIB_FALSE) != LIB_STATUS_OK ||
        x86_core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK ||
        x86_core_machine_reset(machine) != LIB_STATUS_OK ||
        x86_core_machine_memory_write(machine, 0x00fffff0u, &nop, 1u) != LIB_STATUS_OK ||
        x86_core_machine_timeline_schedule(&machine->timeline, 0u, timeline_count,
            &probe, &immediate) != LIB_STATUS_OK ||
        x86_core_machine_capture_time_observation(machine, &time) != LIB_STATUS_OK ||
        time.next_deadline_valid ||
        time.progress_disposition != X86_CORE_MACHINE_TIME_PROGRESS_IMMEDIATE ||
        x86_core_machine_run(machine, (x86_core_machine_run_budget){1u, 0u},
            &run) != LIB_STATUS_OK ||
        run.reason != X86_CORE_MACHINE_STOP_BUDGET || run.elapsed_ticks != 3u ||
        x86_core_machine_get_timeline_observation(machine, &observation) != LIB_STATUS_OK)
        goto done;
    failed = observation.now != 3u || observation.pending_events != 0u ||
        observation.next_sequence != 1u || probe.count != 1u;
done:
    x86_core_machine_destroy(machine);
    return failed;
}

lib_i32 main(void)
{
    x86_core_machine_timeline timeline;
    x86_core_machine_timeline_token first;
    x86_core_machine_timeline_token second;
    x86_core_machine_timeline_token cancelled;
    timeline_probe probe = { 0 };
    lib_u64 due_tick = 0u;
    lib_i32 failed = timeline_failure_contract() || timeline_execution_boundary();

    probe.timeline = &timeline;
    failed |= x86_core_machine_timeline_initialize(&timeline) != LIB_STATUS_OK;
    failed |= x86_core_machine_timeline_schedule(&timeline, 10u, timeline_a, &probe,
        &first) != LIB_STATUS_OK;
    failed |= x86_core_machine_timeline_next_due(&timeline, &due_tick) != LIB_STATUS_OK ||
        due_tick != 10u;
    failed |= x86_core_machine_timeline_schedule(&timeline, 10u, timeline_c, &probe,
        &second) != LIB_STATUS_OK;
    failed |= x86_core_machine_timeline_schedule(&timeline, 5u, timeline_b, &probe,
        &probe.nested) != LIB_STATUS_OK;
    failed |= x86_core_machine_timeline_next_due(&timeline, &due_tick) != LIB_STATUS_OK ||
        due_tick != 5u;
    failed |= x86_core_machine_timeline_schedule(&timeline, 8u, timeline_cancelled,
        &probe, &cancelled) != LIB_STATUS_OK;
    failed |= x86_core_machine_timeline_cancel(&timeline, &cancelled) != LIB_STATUS_OK;
    failed |= x86_core_machine_timeline_advance(&timeline, 10u) != LIB_STATUS_OK;
    failed |= timeline.now != 10u || x86_core_machine_timeline_pending_count(&timeline) != 0u ||
        probe.count != 4u || probe.order[0] != 2u || probe.order[1] != 1u ||
        probe.order[2] != 1u || probe.order[3] != 3u;
    failed |= x86_core_machine_timeline_next_due(&timeline, &due_tick) != LIB_STATUS_INVALID_STATE;
    failed |= x86_core_machine_timeline_schedule(&timeline, 9u, timeline_a, &probe,
        &first) != LIB_STATUS_INVALID_ARGUMENT;
    failed |= x86_core_machine_timeline_schedule(&timeline, 12u, timeline_cancelled,
        &probe, &cancelled) != LIB_STATUS_OK;
    x86_core_machine_timeline_reset(&timeline);
    failed |= x86_core_machine_timeline_advance(&timeline, 20u) != LIB_STATUS_OK ||
        probe.count != 4u || timeline.now != 20u;

    if (failed) return 1;
    lib_c_printf("X86:TIMELINE:OK\n");
    return 0;
}
