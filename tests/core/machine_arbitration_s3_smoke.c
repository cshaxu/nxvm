#include "type.h"

#include "core/machine/machine_interface.h"
#include "../support/core_machine_cpu_fixture.h"

typedef struct arbitration_trace_probe {
    core_machine_trace_event events[16];
    type_unsigned_32 count;
} arbitration_trace_probe;

static C_VOID arbitration_trace(C_VOID *opaque,
    const core_machine_trace_event *event)
{
    arbitration_trace_probe *probe = (arbitration_trace_probe *)opaque;

    if (probe != STD_NULL && probe->count < 16u) {
        probe->events[probe->count++] = *event;
    }
}

static C_INT arbitration_expect_event(const arbitration_trace_probe *probe,
    type_unsigned_32 index, core_machine_trace_event_type type,
    type_unsigned_32 due_tick)
{
    return probe->events[index].type != type ||
        probe->events[index].timeline_ticks != due_tick;
}

C_INT main(C_VOID)
{
    core_machine *machine = STD_NULL;
    core_machine_config config = { 0 };
    core_machine_run_budget budget = { 1u, 0u };
    core_machine_run_result result;
    core_machine_timeline_observation observation;
    arbitration_trace_probe probe = { { { 0 } }, 0u };
    core_machine_trace_provider trace = { arbitration_trace, &probe };
    const type_unsigned_8 nop = 0x90u;
    C_INT failed = 0;

    config.ticks_per_instruction = 3u;
    failed |= core_machine_create(&config, &machine) != TYPE_STATUS_OK;
    failed |= test_core_machine_fixture_register_reset_mapping(machine, 0xfffffff0u,
        0x000ffff0u, 1u) != TYPE_STATUS_OK;
    failed |= core_machine_freeze_execution_providers(machine) != TYPE_STATUS_OK;
    failed |= core_machine_reset(machine) != TYPE_STATUS_OK;
    failed |= core_machine_memory_write(machine, 0xfffffff0u, &nop, 1u) !=
        TYPE_STATUS_OK;
    failed |= core_machine_set_trace_provider(machine, &trace) != TYPE_STATUS_OK;
    failed |= core_machine_run(machine, budget, &result) != TYPE_STATUS_OK;
    failed |= result.reason != CORE_MACHINE_STOP_BUDGET ||
        result.elapsed_ticks != 3u;
    failed |= core_machine_get_timeline_observation(machine, &observation) !=
        TYPE_STATUS_OK;
    failed |= observation.now != 3u || observation.pending_events != 1u ||
        observation.next_sequence != 4u;
    failed |= probe.count != 11u ||
        probe.events[0].type != CORE_MACHINE_TRACE_CPU_RETIRE ||
        arbitration_expect_event(&probe, 1u, CORE_MACHINE_TRACE_DMA_ADVANCE, 1u) ||
        arbitration_expect_event(&probe, 2u, CORE_MACHINE_TRACE_PIT_ADVANCE, 1u) ||
        arbitration_expect_event(&probe, 3u, CORE_MACHINE_TRACE_PIC_REFRESH, 1u) ||
        arbitration_expect_event(&probe, 4u, CORE_MACHINE_TRACE_DMA_ADVANCE, 2u) ||
        arbitration_expect_event(&probe, 5u, CORE_MACHINE_TRACE_PIT_ADVANCE, 2u) ||
        arbitration_expect_event(&probe, 6u, CORE_MACHINE_TRACE_PIC_REFRESH, 2u) ||
        arbitration_expect_event(&probe, 7u, CORE_MACHINE_TRACE_DMA_ADVANCE, 3u) ||
        arbitration_expect_event(&probe, 8u, CORE_MACHINE_TRACE_PIT_ADVANCE, 3u) ||
        arbitration_expect_event(&probe, 9u, CORE_MACHINE_TRACE_PIC_REFRESH, 3u) ||
        probe.events[10].type != CORE_MACHINE_TRACE_RUN_BOUNDARY;
    failed |= core_machine_reset(machine) != TYPE_STATUS_OK;
    failed |= core_machine_get_timeline_observation(machine, &observation) !=
        TYPE_STATUS_OK;
    failed |= observation.now != 0u || observation.pending_events != 1u ||
        observation.next_sequence != 1u;

    core_machine_destroy(machine);
    if (failed) return 1;
    STD_PRINTF("M5:T346:S3:ARBITRATION:OK\n");
    return 0;
}
