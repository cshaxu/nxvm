#include "lib/types/file.h"
#include "lib/types/types_interface.h"
#include "x86-core/machine/machine.h"
#include "support/memory_alias_fixture.h"

typedef struct scheduler_provider_probe {
    lib_u32 advances;
    lib_u64 advanced_ticks;
} scheduler_provider_probe;

static void scheduler_provider_advance(void *owner, lib_u64 ticks)
{
    scheduler_provider_probe *probe = owner;
    ++probe->advances;
    probe->advanced_ticks += ticks;
}

static const x86_core_machine_execution_provider scheduler_provider = {
    LIB_NULL, scheduler_provider_advance
};

typedef struct scheduler_attachment_probe {
    x86_core_machine_attachment_deadline_observation deadline;
    lib_u32 deadline_calls;
    lib_u32 before_calls;
    lib_u32 after_calls;
    lib_u32 finalized;
    lib_u64 before_due_tick;
    lib_u64 before_ticks;
    lib_u64 after_ticks;
    lib_u64 expire_at_tick;
} scheduler_attachment_probe;

static void scheduler_deadline(void *owner, lib_u64 now,
    x86_core_machine_attachment_deadline_observation *out)
{
    scheduler_attachment_probe *probe = owner;
    (void)now;
    ++probe->deadline_calls;
    *out = probe->deadline;
}

static void scheduler_before(void *owner, lib_u64 ticks, lib_u64 due)
{
    scheduler_attachment_probe *probe = owner;
    ++probe->before_calls;
    probe->before_ticks += ticks;
    probe->before_due_tick = due;
}

static void scheduler_after(void *owner, lib_u64 ticks, lib_u64 due)
{
    scheduler_attachment_probe *probe = owner;
    ++probe->after_calls;
    probe->after_ticks += ticks;
    if (probe->expire_at_tick != 0u && due >= probe->expire_at_tick) {
        probe->deadline.source_ticks = 0u;
        probe->expire_at_tick = 0u;
    }
}

static void scheduler_finalize(void *owner)
{
    ++((scheduler_attachment_probe *)owner)->finalized;
}

static void scheduler_count_event(void *context, lib_u64 due_tick)
{
    if (due_tick == 4u) ++*(lib_u32 *)context;
}

static lib_i32 scheduler_source_before_timeline(void)
{
    const x86_core_machine_executor_config config = {
        .memory_bytes = 1048576u, .cpu_profile = X86_CORE_CPU_PROFILE_8086
    };
    scheduler_attachment_probe probe = {
        .deadline.source_ticks = 1u, .expire_at_tick = 1u
    };
    const x86_core_machine_attachment attachment = {
        .deadline = scheduler_deadline, .after_internal_advance = scheduler_after,
        .finalize_devices = scheduler_finalize, .context = &probe
    };
    x86_core_machine *machine = LIB_NULL;
    x86_core_machine_timeline_token token;
    x86_core_machine_time_observation time = {0};
    lib_u32 count = 0u;
    lib_u8 advanced = LIB_FALSE;
    lib_i32 failed = x86_core_machine_neutral_create(&config, &machine) != LIB_STATUS_OK;

    if (!failed) failed |= x86_core_machine_bind_attachment(machine, &attachment) !=
            LIB_STATUS_OK ||
        x86_core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK ||
        x86_core_machine_reset(machine) != LIB_STATUS_OK ||
        x86_core_machine_timeline_schedule(&machine->timeline, 4u,
            scheduler_count_event, &count, &token) != LIB_STATUS_OK ||
        x86_core_machine_capture_time_observation(machine, &time) != LIB_STATUS_OK ||
        !time.next_deadline_valid || time.next_deadline_tick != 1u ||
        x86_core_machine_advance_to_next_deadline(machine, &advanced) != LIB_STATUS_OK ||
        !advanced || machine->elapsed_ticks != 1u || count != 0u ||
        x86_core_machine_capture_time_observation(machine, &time) != LIB_STATUS_OK ||
        !time.next_deadline_valid || time.next_deadline_tick != 4u ||
        x86_core_machine_advance_to_next_deadline(machine, &advanced) != LIB_STATUS_OK ||
        !advanced || machine->elapsed_ticks != 4u || count != 1u;
    x86_core_machine_destroy(machine);
    return failed || probe.finalized != 1u;
}

lib_i32 main(void)
{
    const x86_core_machine_executor_config config = {
        .memory_bytes = 2u * 1024u * 1024u,
        .cpu_profile = X86_CORE_CPU_PROFILE_80286,
        .ticks_per_instruction = 2u
    };
    x86_core_machine_run_budget budget = {0u, 1u};
    x86_core_machine_run_result result;
    x86_core_machine *machine = LIB_NULL;
    scheduler_provider_probe provider = {0};
    scheduler_attachment_probe probe = {0};
    const x86_core_machine_attachment attachment = {
        .deadline = scheduler_deadline,
        .before_internal_advance = scheduler_before,
        .after_internal_advance = scheduler_after,
        .finalize_devices = scheduler_finalize,
        .context = &probe
    };
    x86_core_machine_time_observation observation;
    const lib_u8 nop = 0x90u;
    lib_i32 failed = 0;

    if (x86_core_machine_neutral_create(&config, &machine) != LIB_STATUS_OK)
        return 1;
    failed |= test_core_machine_fixture_register_reset_mapping(machine,
        0xfffffff0u, 0x000ffff0u, 16u) != LIB_STATUS_OK ||
        x86_core_machine_bind_attachment(machine, &attachment) != LIB_STATUS_OK ||
        x86_core_machine_bind_execution_provider(machine, &scheduler_provider,
            &provider) != LIB_STATUS_OK ||
        x86_core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK ||
        x86_core_machine_reset(machine) != LIB_STATUS_OK ||
        x86_core_machine_memory_write(machine, 0xfffffff0u, &nop, 1u) != LIB_STATUS_OK;
    failed |= x86_core_machine_run(machine, budget, &result) != LIB_STATUS_OK ||
        result.reason != X86_CORE_MACHINE_STOP_BUDGET || result.executed != 0u ||
        result.ticks != 0u || result.elapsed_ticks != 0u ||
        provider.advances != 0u || provider.advanced_ticks != 0u ||
        probe.before_calls != 0u || probe.after_calls != 0u;
    budget.instructions = 1u;
    budget.ticks = 0u;
    failed |= x86_core_machine_run(machine, budget, &result) != LIB_STATUS_OK ||
        result.reason != X86_CORE_MACHINE_STOP_BUDGET || result.executed != 1u ||
        result.ticks != 3u || result.elapsed_ticks != 3u ||
        provider.advances != 1u || provider.advanced_ticks != 3u ||
        probe.before_calls == 0u || probe.before_ticks != 3u ||
        probe.before_due_tick != machine->elapsed_ticks ||
        probe.after_calls == 0u || probe.after_ticks != 3u;

    probe.deadline_calls = 0u;
    probe.deadline.source_ticks = 5u;
    x86_core_machine_capture_time_observation_private(machine, &observation);
    failed |= probe.deadline_calls != 1u || !observation.next_deadline_valid ||
        observation.next_deadline_tick != machine->elapsed_ticks + 5u ||
        observation.progress_disposition != X86_CORE_MACHINE_TIME_PROGRESS_DEADLINE;
    probe.deadline.immediate_due = LIB_TRUE;
    x86_core_machine_capture_time_observation_private(machine, &observation);
    failed |= probe.deadline_calls != 2u || observation.next_deadline_valid ||
        observation.progress_disposition != X86_CORE_MACHINE_TIME_PROGRESS_IMMEDIATE;
    probe.deadline.immediate_due = LIB_FALSE;
    probe.deadline.l1_compatibility = LIB_TRUE;
    probe.deadline.fast_advance_blocked = LIB_TRUE;
    x86_core_machine_capture_time_observation_private(machine, &observation);
    failed |= observation.next_deadline_valid ||
        observation.progress_disposition != X86_CORE_MACHINE_TIME_PROGRESS_L1_COMPATIBILITY;
    probe.deadline.l1_compatibility = LIB_FALSE;
    x86_core_machine_capture_time_observation_private(machine, &observation);
    failed |= observation.next_deadline_valid ||
        observation.progress_disposition != X86_CORE_MACHINE_TIME_PROGRESS_DEADLINE ||
        probe.deadline_calls != 4u;

    /* Clock-limit injection stays at the timeline owner; normal publication
     * must still deliver the registered attachment and execution callbacks. */
    probe.deadline.fast_advance_blocked = LIB_FALSE;
    machine->elapsed_ticks = LIB_UINT64_MAX - 5u;
    x86_core_machine_capture_time_observation_private(machine, &observation);
    failed |= !observation.next_deadline_valid ||
        observation.next_deadline_tick != LIB_UINT64_MAX ||
        observation.progress_disposition != X86_CORE_MACHINE_TIME_PROGRESS_DEADLINE;
    {
        lib_u8 advanced = LIB_FALSE;
        const lib_u64 before = provider.advanced_ticks;
        const lib_u32 before_calls = probe.before_calls;

        failed |= x86_core_machine_advance_to_next_deadline(machine, &advanced) !=
                LIB_STATUS_OK || !advanced || machine->elapsed_ticks != LIB_UINT64_MAX ||
            provider.advanced_ticks != before + 5u ||
            probe.before_calls != before_calls + 1u ||
            probe.before_due_tick != LIB_UINT64_MAX;
        x86_core_machine_capture_time_observation_private(machine, &observation);
        failed |= observation.next_deadline_valid ||
            observation.progress_disposition != X86_CORE_MACHINE_TIME_PROGRESS_DEADLINE;
        failed |= x86_core_machine_publish_elapsed_ticks(machine, 1u,
                X86_CORE_MACHINE_TIME_PUBLICATION_DEADLINE) != LIB_STATUS_INVALID_ARGUMENT ||
            machine->elapsed_ticks != LIB_UINT64_MAX ||
            provider.advanced_ticks != before + 5u;
    }
    x86_core_machine_destroy(machine);
    if (failed || probe.finalized != 1u || scheduler_source_before_timeline()) return 1;
    lib_c_printf("SCHEDULER:OK\n");
    return 0;
}
