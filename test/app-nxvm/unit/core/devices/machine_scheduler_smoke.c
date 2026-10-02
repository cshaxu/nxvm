#include "lib/types/types_interface.h"
#include <stdio.h>

#include "app-nxvm/devices/machine.h"
#include "support/core_machine_board_fixture.h"

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

typedef struct scheduler_board_probe {
    core_machine *machine;
    lib_u32 media_calls;
    lib_u32 rtc_calls;
    lib_u32 dma_calls;
    lib_u32 calls;
    lib_u64 media_due_tick;
    lib_u64 media_ticks;
    lib_u64 rtc_ticks;
    lib_u64 dma_ticks;
    lib_u64 advanced_ticks;
} scheduler_board_probe;

static void scheduler_board_deadline_forward(void *owner, lib_u64 now,
    core_machine_board_deadline_observation *out_observation)
{
    scheduler_board_probe *probe = owner;
    core_machine_board_deadline_observe(probe->machine, now, out_observation);
}

static void scheduler_board_peripheral(void *owner, lib_u64 source_ticks)
{
    scheduler_board_probe *probe = owner;
    ++probe->calls;
    probe->advanced_ticks += source_ticks;
    core_machine_board_peripheral_advance(probe->machine, source_ticks);
}

static void scheduler_board_media(void *owner, lib_u64 source_ticks,
    lib_u64 due_tick)
{
    scheduler_board_probe *probe = owner;
    ++probe->media_calls;
    probe->media_ticks += source_ticks;
    probe->media_due_tick = due_tick;
    core_machine_board_media_advance(probe->machine, source_ticks, due_tick);
}

static void scheduler_board_rtc(void *owner, lib_u64 source_ticks)
{
    scheduler_board_probe *probe = owner;
    ++probe->rtc_calls;
    probe->rtc_ticks += source_ticks;
    core_machine_board_rtc_advance(probe->machine, source_ticks);
}

static lib_bool scheduler_board_refresh_request(void *owner, lib_u8 *out_address)
{
    scheduler_board_probe *probe = owner;
    return core_machine_board_refresh_request(probe->machine, out_address);
}

static void scheduler_board_refresh_complete(void *owner)
{
    scheduler_board_probe *probe = owner;
    core_machine_board_refresh_complete(probe->machine);
}

static lib_u64 scheduler_board_dma_ticks(void *owner, lib_u64 source_ticks)
{
    scheduler_board_probe *probe = owner;
    return core_machine_board_dma_ticks(probe->machine, source_ticks);
}

static lib_bool scheduler_board_dma_request(void *owner)
{
    scheduler_board_probe *probe = owner;
    return core_machine_board_dma_request(probe->machine);
}

static void scheduler_board_dma_advance(void *owner, lib_u64 dma_ticks)
{
    scheduler_board_probe *probe = owner;
    ++probe->dma_calls;
    probe->dma_ticks += dma_ticks;
    core_machine_board_dma_advance(probe->machine, dma_ticks);
}

typedef struct scheduler_deadline_probe {
    core_machine_board_deadline_observation value;
    lib_u32 calls;
} scheduler_deadline_probe;

static void scheduler_board_deadline(void *owner, lib_u64 now,
    core_machine_board_deadline_observation *out_observation)
{
    scheduler_deadline_probe *probe = owner;
    (void)now;
    ++probe->calls;
    *out_observation = probe->value;
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
    config.cpu_profile = CORE_MACHINE_CPU_PROFILE_80286;
    failed |= core_machine_create(&config, &machine) != LIB_STATUS_OK;
    failed |= test_core_machine_fixture_register_reset_mapping(machine, 0xfffffff0u,
        0x000ffff0u, 16u) != LIB_STATUS_OK;
    failed |= core_machine_bind_execution_provider(machine, &scheduler_provider,
        &provider_probe) != LIB_STATUS_OK;
    failed |= core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK;
    failed |= core_machine_reset(machine) != LIB_STATUS_OK;
    failed |= core_machine_memory_write(machine, 0xfffffff0u, &nop, sizeof(nop)) !=
        LIB_STATUS_OK;
    board_probe.machine = machine;
    machine->board_deadline_provider = scheduler_board_deadline_forward;
    machine->board_media_provider = scheduler_board_media;
    machine->board_rtc_provider = scheduler_board_rtc;
    machine->board_peripheral_provider = scheduler_board_peripheral;
    machine->board_refresh_request_provider = scheduler_board_refresh_request;
    machine->board_refresh_complete_provider = scheduler_board_refresh_complete;
    machine->board_dma_ticks_provider = scheduler_board_dma_ticks;
    machine->board_dma_request_provider = scheduler_board_dma_request;
    machine->board_dma_advance_provider = scheduler_board_dma_advance;
    machine->board_owner = &board_probe;

    failed |= core_machine_run(machine, budget, &result) != LIB_STATUS_OK;
    failed |= result.reason != CORE_MACHINE_STOP_BUDGET || result.executed != 0u ||
        result.ticks != 0u || result.elapsed_ticks != 0u ||
        provider_probe.advances != 0u || provider_probe.advanced_ticks != 0u ||
        board_probe.media_calls != 0u || board_probe.rtc_calls != 0u ||
        board_probe.calls != 0u;

    budget.instructions = 1u;
    budget.ticks = 0u;
    failed |= core_machine_run(machine, budget, &result) != LIB_STATUS_OK;
    failed |= result.reason != CORE_MACHINE_STOP_BUDGET || result.executed != 1u ||
        result.ticks != 3u || result.elapsed_ticks != 3u ||
        provider_probe.advances != 1u || provider_probe.advanced_ticks != 3u ||
        board_probe.media_calls == 0u || board_probe.rtc_calls == 0u ||
        board_probe.media_ticks != 3u || board_probe.rtc_ticks != 3u ||
        board_probe.media_due_tick != machine->elapsed_ticks ||
        board_probe.calls == 0u || board_probe.advanced_ticks != 3u;

    machine->board_deadline_provider = scheduler_board_deadline;
    machine->board_owner = &deadline_probe;
    deadline_probe.value.source_ticks = 5u;
    core_machine_capture_time_observation_private(machine, &observation);
    failed |= deadline_probe.calls != 1u || !observation.next_deadline_valid ||
        observation.next_deadline_tick != machine->elapsed_ticks + 5u ||
        observation.progress_disposition != CORE_MACHINE_TIME_PROGRESS_DEADLINE;
    deadline_probe.value.immediate_due = LIB_TRUE;
    core_machine_capture_time_observation_private(machine, &observation);
    failed |= observation.next_deadline_valid ||
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

    machine->board_peripheral_provider = core_machine_board_peripheral_advance;
    machine->board_owner = machine;
    core_machine_destroy(machine);
    if (failed) return 1;
    printf("M5:T219:S2:SCHEDULER:OK\n");
    return 0;
}
