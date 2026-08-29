#include "type.h"

#include "core/machine/machine_interface.h"
#include "../support/core_machine_cpu_fixture.h"

typedef struct arbitration_trace_probe {
    core_machine_trace_event events[64];
    type_unsigned_32 count;
} arbitration_trace_probe;

static C_VOID arbitration_trace(C_VOID *opaque,
    const core_machine_trace_event *event)
{
    arbitration_trace_probe *probe = (arbitration_trace_probe *)opaque;

    if (probe != STD_NULL && probe->count < 64u) {
        probe->events[probe->count++] = *event;
    }
}

static C_INT arbitration_expect_chain(const arbitration_trace_probe *probe)
{
    type_unsigned_32 index;
    type_unsigned_32 selected_tick = 3u;
    type_unsigned_32 phase = 0u;
    type_unsigned_32 groups = 0u;

    for (index = 0u; index < probe->count; ++index) {
        const core_machine_trace_event *event = &probe->events[index];

        if (event->type == CORE_MACHINE_TRACE_DMA_ADVANCE ||
            event->type == CORE_MACHINE_TRACE_PIT_ADVANCE ||
            event->type == CORE_MACHINE_TRACE_PIC_REFRESH) {
            core_machine_trace_event_type expected = phase == 0u ?
                CORE_MACHINE_TRACE_DMA_ADVANCE : phase == 1u ?
                CORE_MACHINE_TRACE_PIT_ADVANCE : CORE_MACHINE_TRACE_PIC_REFRESH;

            if (event->type != expected || event->timeline_ticks != selected_tick) {
                return 1;
            }
            ++phase;
            if (phase == 3u) {
                phase = 0u;
                ++groups;
            }
        }
    }
    return groups != 1u || phase != 0u;
}

static C_INT arbitration_has_cpu_retire(const arbitration_trace_probe *probe)
{
    type_unsigned_32 index;

    for (index = 0u; index < probe->count; ++index) {
        if (probe->events[index].type == CORE_MACHINE_TRACE_CPU_RETIRE) return 1;
    }
    return 0;
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
        0x000ffff0u, 16u) != TYPE_STATUS_OK;
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
    failed |= observation.now != 3u || observation.pending_events != 0u ||
        observation.next_sequence != 0u;
    failed |= probe.count < 5u || !arbitration_has_cpu_retire(&probe) ||
        arbitration_expect_chain(&probe) ||
        probe.events[probe.count - 1u].type != CORE_MACHINE_TRACE_RUN_BOUNDARY;
    failed |= core_machine_reset(machine) != TYPE_STATUS_OK;
    failed |= core_machine_get_timeline_observation(machine, &observation) !=
        TYPE_STATUS_OK;
    failed |= observation.now != 0u || observation.pending_events != 0u ||
        observation.next_sequence != 0u;

    core_machine_destroy(machine);
    if (failed) return 1;
    STD_PRINTF("M5:T346:S3:ARBITRATION:OK\n");
    return 0;
}
