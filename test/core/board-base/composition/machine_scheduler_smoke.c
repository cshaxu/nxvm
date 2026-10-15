#include "lib/types/file.h"
#include "lib/types/types_interface.h"
#include "core/board-base/machine_board_interface.h"
#include "core/x86/machine.h"
#include "../core_machine_board_fixture.h"

typedef struct scheduler_provider_probe {
    lib_u32 advances;
    lib_u64 advanced_ticks;
} scheduler_provider_probe;

static void scheduler_provider_advance(void *opaque,
    lib_u64 elapsed_ticks)
{
    scheduler_provider_probe *probe = (scheduler_provider_probe *)opaque;

    if (probe != LIB_NULL) {
        ++probe->advances;
        probe->advanced_ticks += elapsed_ticks;
    }
}

static const core_machine_execution_provider scheduler_provider = {
    LIB_NULL,
    scheduler_provider_advance
};

typedef struct scheduler_deadline_probe {
    core_machine_attachment_deadline_observation value;
    lib_u32 calls;
} scheduler_deadline_probe;

static void scheduler_board_deadline(void *owner, lib_u64 now,
    core_machine_attachment_deadline_observation *out_observation);

typedef struct scheduler_board_probe {
    core_machine *machine;
    core_machine_attachment production;
    scheduler_deadline_probe *deadline_probe;
    lib_u32 before_internal_calls;
    lib_u32 dma_calls;
    lib_u32 calls;
    lib_u64 before_internal_due_tick;
    lib_u64 before_internal_ticks;
    lib_u64 dma_ticks;
    lib_u64 advanced_ticks;
} scheduler_board_probe;

static void scheduler_board_deadline_forward(void *owner, lib_u64 now,
    core_machine_attachment_deadline_observation *out_observation)
{
    scheduler_board_probe *probe = owner;
    if (probe->deadline_probe != LIB_NULL) {
        scheduler_board_deadline(probe->deadline_probe, now,
            out_observation);
        return;
    }
    probe->production.deadline(probe->production.context, now,
        out_observation);
}

static void scheduler_board_after_internal(void *owner, lib_u64 source_ticks,
    lib_u64 due_tick)
{
    scheduler_board_probe *probe = owner;
    ++probe->calls;
    probe->advanced_ticks += source_ticks;
    probe->production.after_internal_advance(probe->production.context,
        source_ticks, due_tick);
}

static void scheduler_board_before_internal(void *owner, lib_u64 source_ticks,
    lib_u64 due_tick)
{
    scheduler_board_probe *probe = owner;
    ++probe->before_internal_calls;
    probe->before_internal_ticks += source_ticks;
    probe->before_internal_due_tick = due_tick;
    probe->production.before_internal_advance(probe->production.context,
        source_ticks, due_tick);
}

static lib_bool scheduler_board_refresh_request(void *owner, lib_u8 *out_address)
{
    scheduler_board_probe *probe = owner;
    return probe->production.refresh_request(probe->production.context, out_address);
}

static void scheduler_board_refresh_complete(void *owner)
{
    scheduler_board_probe *probe = owner;
    probe->production.refresh_complete(probe->production.context);
}

static lib_u64 scheduler_board_dma_ticks(void *owner, lib_u64 source_ticks)
{
    scheduler_board_probe *probe = owner;
    return probe->production.dma_ticks(probe->production.context, source_ticks);
}

static lib_bool scheduler_board_dma_request(void *owner)
{
    scheduler_board_probe *probe = owner;
    return probe->production.dma_request(probe->production.context);
}

static void scheduler_board_dma_advance(void *owner, lib_u64 dma_ticks)
{
    scheduler_board_probe *probe = owner;
    ++probe->dma_calls;
    probe->dma_ticks += dma_ticks;
    probe->production.dma_advance(probe->production.context, dma_ticks);
}

static lib_bool scheduler_board_pic_pending(void *owner)
{
    scheduler_board_probe *probe = owner;
    return probe->production.interrupt_pending(probe->production.context);
}

static lib_u8 scheduler_board_pic_acknowledge(void *owner)
{
    scheduler_board_probe *probe = owner;
    return probe->production.interrupt_acknowledge(probe->production.context);
}

static void scheduler_board_deadline(void *owner, lib_u64 now,
    core_machine_attachment_deadline_observation *out_observation)
{
    scheduler_deadline_probe *probe = owner;
    (void)now;
    ++probe->calls;
    *out_observation = probe->value;
}

static void scheduler_board_reset_devices(void *owner)
{
    scheduler_board_probe *probe = owner;
    probe->production.reset_devices(probe->production.context);
}

static void scheduler_board_reset_clocks(void *owner)
{
    scheduler_board_probe *probe = owner;
    probe->production.reset_clocks(probe->production.context);
}

static void scheduler_board_refresh_nmi(void *owner)
{
    scheduler_board_probe *probe = owner;
    probe->production.refresh_nmi(probe->production.context);
}

static void scheduler_board_finalize_devices(void *owner)
{
    scheduler_board_probe *probe = owner;
    probe->production.finalize_devices(probe->production.context);
}

static lib_bool scheduler_board_shutdown_reset(void *owner)
{
    scheduler_board_probe *probe = owner;
    return probe->production.shutdown_reset(probe->production.context);
}

static lib_status scheduler_board_firmware(void *owner)
{
    scheduler_board_probe *probe = owner;
    return probe->production.firmware(probe->production.context);
}

lib_i32 main(void)
{
    core_machine_config config = { 0 };
    core_machine_run_budget budget = { 0u, 1u };
    core_machine_run_result result;
    core_machine *machine = LIB_NULL;
    scheduler_provider_probe provider_probe = { 0u, 0u };
    scheduler_board_probe board_probe = {0};
    scheduler_deadline_probe deadline_probe = {0};
    core_machine_time_observation observation;
    const lib_u8 nop = 0x90u;
    lib_i32 failed = 0;

    config.ticks_per_instruction = 2u;
    config.cpu_profile = X86_CORE_CPU_PROFILE_80286;
    failed |= core_machine_create(&config, &machine, LIB_NULL) != LIB_STATUS_OK;
    failed |= test_core_machine_fixture_register_reset_mapping(machine, 0xfffffff0u,
        0x000ffff0u, 16u) != LIB_STATUS_OK;
    failed |= core_machine_bind_execution_provider(machine, &scheduler_provider,
        &provider_probe) != LIB_STATUS_OK;
    failed |= core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK;
    failed |= core_machine_reset(machine) != LIB_STATUS_OK;
    failed |= core_machine_memory_write(machine, 0xfffffff0u, &nop, sizeof(nop)) !=
        LIB_STATUS_OK;
    board_probe.machine = machine;
    board_probe.production = machine->attachment;
    /* Same-module instrumentation replaces a complete bundle, never the public
     * construction-only binding. Every callback retains its correct context. */
    machine->attachment = (core_machine_attachment) {
        .deadline = scheduler_board_deadline_forward,
        .refresh_request = scheduler_board_refresh_request,
        .refresh_complete = scheduler_board_refresh_complete,
        .dma_ticks = scheduler_board_dma_ticks,
        .dma_request = scheduler_board_dma_request,
        .dma_advance = scheduler_board_dma_advance,
        .interrupt_pending = scheduler_board_pic_pending,
        .interrupt_acknowledge = scheduler_board_pic_acknowledge,
        .shutdown_reset = scheduler_board_shutdown_reset,
        .before_internal_advance = scheduler_board_before_internal,
        .after_internal_advance = scheduler_board_after_internal,
        .reset_devices = scheduler_board_reset_devices,
        .reset_clocks = scheduler_board_reset_clocks,
        .refresh_nmi = scheduler_board_refresh_nmi,
        .finalize_devices = scheduler_board_finalize_devices,
        .firmware = scheduler_board_firmware,
        .context = &board_probe
    };

    failed |= core_machine_run(machine, budget, &result) != LIB_STATUS_OK;
    failed |= result.reason != CORE_MACHINE_STOP_BUDGET || result.executed != 0u ||
        result.ticks != 0u || result.elapsed_ticks != 0u ||
        provider_probe.advances != 0u || provider_probe.advanced_ticks != 0u ||
        board_probe.before_internal_calls != 0u ||
        board_probe.calls != 0u;

    budget.instructions = 1u;
    budget.ticks = 0u;
    failed |= core_machine_run(machine, budget, &result) != LIB_STATUS_OK;
    failed |= result.reason != CORE_MACHINE_STOP_BUDGET || result.executed != 1u ||
        result.ticks != 3u || result.elapsed_ticks != 3u ||
        provider_probe.advances != 1u || provider_probe.advanced_ticks != 3u ||
        board_probe.before_internal_calls == 0u ||
        board_probe.before_internal_ticks != 3u ||
        board_probe.before_internal_due_tick != machine->elapsed_ticks ||
        board_probe.calls == 0u || board_probe.advanced_ticks != 3u;

    board_probe.deadline_probe = &deadline_probe;
    deadline_probe.value.source_ticks = 5u;
    core_machine_capture_time_observation_private(machine, &observation);
    failed |= deadline_probe.calls != 1u ||
        !observation.next_deadline_valid ||
        observation.next_deadline_tick != machine->elapsed_ticks + 5u ||
        observation.progress_disposition != CORE_MACHINE_TIME_PROGRESS_DEADLINE;
    deadline_probe.value.immediate_due = LIB_TRUE;
    core_machine_capture_time_observation_private(machine, &observation);
    failed |= deadline_probe.calls != 2u || observation.next_deadline_valid ||
        observation.progress_disposition != CORE_MACHINE_TIME_PROGRESS_IMMEDIATE;
    deadline_probe.value.immediate_due = LIB_FALSE;
    deadline_probe.value.l1_compatibility = LIB_TRUE;
    deadline_probe.value.fast_advance_blocked = LIB_TRUE;
    core_machine_capture_time_observation_private(machine, &observation);
    failed |= observation.next_deadline_valid ||
        observation.progress_disposition != CORE_MACHINE_TIME_PROGRESS_L1_COMPATIBILITY;
    deadline_probe.value.l1_compatibility = LIB_FALSE;
    core_machine_capture_time_observation_private(machine, &observation);
    failed |= observation.next_deadline_valid ||
        observation.progress_disposition != CORE_MACHINE_TIME_PROGRESS_DEADLINE ||
        deadline_probe.calls != 4u;

    /* Same-owner clock injection isolates the representable time horizon;
     * the normal scheduler still delivers the production attachment bundle. */
    deadline_probe.value.fast_advance_blocked = LIB_FALSE;
    machine->elapsed_ticks = LIB_UINT64_MAX - 5u;
    core_machine_capture_time_observation_private(machine, &observation);
    failed |= !observation.next_deadline_valid ||
        observation.next_deadline_tick != LIB_UINT64_MAX ||
        observation.progress_disposition != CORE_MACHINE_TIME_PROGRESS_DEADLINE;
    {
        lib_u8 advanced = LIB_FALSE;
        const lib_u64 before = provider_probe.advanced_ticks;
        const lib_u32 before_calls = board_probe.before_internal_calls;
        failed |= core_machine_advance_to_next_deadline(machine, &advanced) !=
            LIB_STATUS_OK || !advanced || machine->elapsed_ticks != LIB_UINT64_MAX ||
            provider_probe.advanced_ticks != before + 5u ||
            board_probe.before_internal_calls != before_calls + 1u ||
            board_probe.before_internal_due_tick != LIB_UINT64_MAX;
        core_machine_capture_time_observation_private(machine, &observation);
        failed |= observation.next_deadline_valid ||
            observation.progress_disposition != CORE_MACHINE_TIME_PROGRESS_DEADLINE;
        failed |= core_machine_publish_elapsed_ticks(machine, 1u,
            CORE_MACHINE_TIME_PUBLICATION_DEADLINE) != LIB_STATUS_INVALID_ARGUMENT ||
            machine->elapsed_ticks != LIB_UINT64_MAX ||
            provider_probe.advanced_ticks != before + 5u;
    }

    machine->attachment = board_probe.production;
    core_machine_destroy(machine);
    if (failed) return 1;
    lib_c_printf("SCHEDULER:OK\n");
    return 0;
}
