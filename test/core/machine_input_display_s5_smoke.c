#include "type.h"

#include "core/machine/machine.h"
#include "support/core_machine_cpu_fixture.h"

typedef struct input_display_trace_probe {
    core_machine_trace_event events[40];
    type_unsigned_32 count;
} input_display_trace_probe;

static C_VOID input_display_trace(C_VOID *opaque,
    const core_machine_trace_event *event)
{
    input_display_trace_probe *probe = (input_display_trace_probe *)opaque;

    if (probe != STD_NULL && probe->count <
        sizeof(probe->events) / sizeof(probe->events[0])) {
        probe->events[probe->count++] = *event;
    }
}

static const core_machine_trace_event *input_display_find_event(
    const input_display_trace_probe *probe, core_machine_trace_event_type type)
{
    type_unsigned_32 index;

    for (index = 0u; index < probe->count; ++index) {
        if (probe->events[index].type == type) return &probe->events[index];
    }
    return STD_NULL;
}

C_INT main(C_VOID)
{
    core_machine *machine = STD_NULL;
    core_machine_config config = { 0 };
    core_machine_run_budget budget = { 1u, 0u };
    core_machine_run_result result;
    core_machine_timeline_observation observation;
    input_display_trace_probe probe = { { { 0 } }, 0u };
    core_machine_trace_provider trace = { input_display_trace, &probe };
    const type_unsigned_8 nop = 0x90u;
    C_INT failed = 0;

    config.ticks_per_instruction = 1u;
    config.cpu_profile = CORE_MACHINE_CPU_PROFILE_80286;
    failed |= core_machine_create(&config, &machine) != TYPE_STATUS_OK;
    failed |= !failed && test_core_machine_fixture_register_reset_mapping(machine,
        0x00fffff0u, 0x000ffff0u, 16u) != TYPE_STATUS_OK;
    failed |= !failed && core_machine_freeze_execution_providers(machine) !=
        TYPE_STATUS_OK;
    failed |= !failed && core_machine_reset(machine) != TYPE_STATUS_OK;
    failed |= !failed && core_machine_memory_write(machine, 0x00fffff0u, &nop,
        sizeof(nop)) != TYPE_STATUS_OK;
    failed |= !failed && core_machine_set_trace_provider(machine, &trace) !=
        TYPE_STATUS_OK;
    failed |= !failed && core_machine_run(machine, budget, &result) != TYPE_STATUS_OK;
    failed |= !failed && (result.reason != CORE_MACHINE_STOP_BUDGET ||
        result.elapsed_ticks != 3u);
    failed |= !failed && core_machine_get_timeline_observation(machine,
        &observation) != TYPE_STATUS_OK;
    failed |= !failed && (observation.now != 3u || observation.pending_events != 0u ||
        observation.next_sequence != 0u);
    {
        const core_machine_trace_event *retire = input_display_find_event(&probe,
            CORE_MACHINE_TRACE_CPU_RETIRE);
        const core_machine_trace_event *fdc = input_display_find_event(&probe,
            CORE_MACHINE_TRACE_FDC_ADVANCE);
        const core_machine_trace_event *hdc = input_display_find_event(&probe,
            CORE_MACHINE_TRACE_HDC_ADVANCE);
        const core_machine_trace_event *kbc = input_display_find_event(&probe,
            CORE_MACHINE_TRACE_KBC_ADVANCE);
        const core_machine_trace_event *vadp = input_display_find_event(&probe,
            CORE_MACHINE_TRACE_VADP_ADVANCE);
        const core_machine_trace_event *boundary = input_display_find_event(&probe,
            CORE_MACHINE_TRACE_RUN_BOUNDARY);

        failed |= !failed && (retire == STD_NULL || fdc == STD_NULL ||
            hdc == STD_NULL || kbc == STD_NULL || vadp == STD_NULL ||
            boundary == STD_NULL || fdc->timeline_ticks != 3u ||
            hdc->timeline_ticks != 3u ||
            kbc->timeline_ticks != 3u || vadp->timeline_ticks != 3u ||
            retire->sequence >= fdc->sequence || fdc->sequence >= hdc->sequence ||
            hdc->sequence >= kbc->sequence ||
            kbc->sequence >= vadp->sequence || vadp->sequence >= boundary->sequence);
    }
    failed |= !failed && core_machine_reset(machine) != TYPE_STATUS_OK;
    failed |= !failed && core_machine_get_timeline_observation(machine,
        &observation) != TYPE_STATUS_OK;
    failed |= !failed && (observation.now != 0u || observation.pending_events != 0u ||
        observation.next_sequence != 0u);

    core_machine_destroy(machine);
    if (failed) return 1;
    STD_PRINTF("M5:T346:S5:INPUT-DISPLAY-TIMELINE:OK\n");
    return 0;
}
