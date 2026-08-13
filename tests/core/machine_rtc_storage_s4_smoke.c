#include "type.h"

#include "core/machine/machine.h"
#include "../support/core_machine_cpu_fixture.h"

typedef struct readiness_trace_probe {
    core_machine_trace_event events[16];
    type_unsigned_32 count;
} readiness_trace_probe;

static C_VOID readiness_trace(C_VOID *opaque,
    const core_machine_trace_event *event)
{
    readiness_trace_probe *probe = (readiness_trace_probe *)opaque;

    if (probe != STD_NULL && probe->count < 16u) {
        probe->events[probe->count++] = *event;
    }
}

static C_INT readiness_expect_event(const readiness_trace_probe *probe,
    type_unsigned_32 index, core_machine_trace_event_type type,
    type_unsigned_64 due_tick)
{
    return probe->events[index].type != type ||
        probe->events[index].timeline_ticks != due_tick;
}

C_INT main(C_VOID)
{
    core_machine *machine = STD_NULL;
    core_machine_config config = { 0 };
    core_machine_rtc_cmos_config rtc_config = { 0 };
    core_machine_run_budget budget = { 1u, 0u };
    core_machine_run_result result;
    core_machine_timeline_observation observation;
    readiness_trace_probe probe = { { { 0 } }, 0u };
    core_machine_trace_provider trace = { readiness_trace, &probe };
    const type_unsigned_8 nop = 0x90u;
    C_INT failed = 0;

    config.ticks_per_instruction = 2u;
    config.clock_plan.rtc.numerator = 1u;
    config.clock_plan.rtc.denominator = 1u;
    rtc_config.index_port = 0x0070u;
    rtc_config.data_port = 0x0071u;
    rtc_config.irq = 8u;
    rtc_config.nmi_mask_bit = 0x80u;
    rtc_config.ticks_per_second = 1u;

    failed |= core_machine_create(&config, &machine) != TYPE_STATUS_OK;
    failed |= !failed && core_machine_configure_rtc_cmos(machine, &rtc_config) !=
        TYPE_STATUS_OK;
    failed |= !failed && test_core_machine_fixture_register_reset_mapping(machine,
        0xfffffff0u, 0x000ffff0u, 1u) != TYPE_STATUS_OK;
    failed |= !failed && core_machine_freeze_execution_providers(machine) !=
        TYPE_STATUS_OK;
    failed |= !failed && core_machine_reset(machine) != TYPE_STATUS_OK;
    failed |= !failed && core_machine_memory_write(machine, 0xfffffff0u, &nop,
        sizeof(nop)) != TYPE_STATUS_OK;
    failed |= !failed && core_machine_bus_write(machine, 0x0070u,
        CORE_MACHINE_RTC_REG_B) != TYPE_STATUS_OK;
    failed |= !failed && core_machine_bus_write(machine, 0x0071u,
        CORE_MACHINE_RTC_REG_B_24H | CORE_MACHINE_RTC_REG_B_UIE) !=
        TYPE_STATUS_OK;
    failed |= !failed && core_machine_set_trace_provider(machine, &trace) !=
        TYPE_STATUS_OK;
    failed |= !failed && core_machine_run(machine, budget, &result) != TYPE_STATUS_OK;
    failed |= !failed && (result.reason != CORE_MACHINE_STOP_BUDGET ||
        result.elapsed_ticks != 2u ||
        machine->shared_rtc.calendar.second != 2u);
    failed |= !failed && core_machine_get_timeline_observation(machine,
        &observation) != TYPE_STATUS_OK;
    failed |= !failed && (observation.now != 2u || observation.pending_events != 2u ||
        observation.next_sequence != 6u);
    failed |= !failed && (probe.count != 14u ||
        probe.events[0].type != CORE_MACHINE_TRACE_CPU_RETIRE ||
        readiness_expect_event(&probe, 1u, CORE_MACHINE_TRACE_DMA_ADVANCE, 1u) ||
        readiness_expect_event(&probe, 2u, CORE_MACHINE_TRACE_PIT_ADVANCE, 1u) ||
        readiness_expect_event(&probe, 3u, CORE_MACHINE_TRACE_PIC_REFRESH, 1u) ||
        readiness_expect_event(&probe, 4u, CORE_MACHINE_TRACE_FDC_REFRESH, 1u) ||
        readiness_expect_event(&probe, 5u, CORE_MACHINE_TRACE_HDC_REFRESH, 1u) ||
        readiness_expect_event(&probe, 6u, CORE_MACHINE_TRACE_RTC_ADVANCE, 1u) ||
        readiness_expect_event(&probe, 7u, CORE_MACHINE_TRACE_DMA_ADVANCE, 2u) ||
        readiness_expect_event(&probe, 8u, CORE_MACHINE_TRACE_PIT_ADVANCE, 2u) ||
        readiness_expect_event(&probe, 9u, CORE_MACHINE_TRACE_PIC_REFRESH, 2u) ||
        readiness_expect_event(&probe, 10u, CORE_MACHINE_TRACE_FDC_REFRESH, 2u) ||
        readiness_expect_event(&probe, 11u, CORE_MACHINE_TRACE_HDC_REFRESH, 2u) ||
        readiness_expect_event(&probe, 12u, CORE_MACHINE_TRACE_RTC_ADVANCE, 2u) ||
        probe.events[13].type != CORE_MACHINE_TRACE_RUN_BOUNDARY);
    failed |= !failed && core_machine_reset(machine) != TYPE_STATUS_OK;
    failed |= !failed && core_machine_get_timeline_observation(machine,
        &observation) != TYPE_STATUS_OK;
    failed |= !failed && (observation.now != 0u || observation.pending_events != 2u ||
        observation.next_sequence != 2u);

    core_machine_destroy(machine);
    if (failed) return 1;
    STD_PRINTF("M5:T346:S4:RTC-STORAGE-READINESS:OK\n");
    return 0;
}
