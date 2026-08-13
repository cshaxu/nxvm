#include "type.h"

#include "core/machine/machine.h"
#include "../support/core_machine_cpu_fixture.h"

typedef struct readiness_trace_probe {
    core_machine_trace_event events[24];
    type_unsigned_32 count;
} readiness_trace_probe;

static C_VOID readiness_trace(C_VOID *opaque,
    const core_machine_trace_event *event)
{
    readiness_trace_probe *probe = (readiness_trace_probe *)opaque;

    if (probe != STD_NULL && probe->count < 24u) {
        probe->events[probe->count++] = *event;
    }
}

static C_INT readiness_expect_chain(const readiness_trace_probe *probe)
{
    type_unsigned_32 index;
    type_unsigned_64 due_tick = 1u;
    type_unsigned_32 phase = 0u;

    for (index = 0u; index < probe->count; ++index) {
        const core_machine_trace_event *event = &probe->events[index];

        if (event->type == CORE_MACHINE_TRACE_FDC_REFRESH ||
            event->type == CORE_MACHINE_TRACE_HDC_REFRESH ||
            event->type == CORE_MACHINE_TRACE_RTC_ADVANCE) {
            core_machine_trace_event_type expected = phase == 0u ?
                CORE_MACHINE_TRACE_FDC_REFRESH : phase == 1u ?
                CORE_MACHINE_TRACE_HDC_REFRESH : CORE_MACHINE_TRACE_RTC_ADVANCE;

            if (event->type != expected || event->timeline_ticks != due_tick) {
                return 1;
            }
            ++phase;
            if (phase == 3u) {
                phase = 0u;
                ++due_tick;
            }
        }
    }
    return due_tick != 3u || phase != 0u;
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
    failed |= !failed && (observation.now != 2u || observation.pending_events != 3u ||
        observation.next_sequence != 9u);
    failed |= !failed && (probe.count < 5u ||
        probe.events[0].type != CORE_MACHINE_TRACE_CPU_RETIRE ||
        readiness_expect_chain(&probe) ||
        probe.events[probe.count - 1u].type != CORE_MACHINE_TRACE_RUN_BOUNDARY);
    failed |= !failed && core_machine_reset(machine) != TYPE_STATUS_OK;
    failed |= !failed && core_machine_get_timeline_observation(machine,
        &observation) != TYPE_STATUS_OK;
    failed |= !failed && (observation.now != 0u || observation.pending_events != 3u ||
        observation.next_sequence != 3u);

    core_machine_destroy(machine);
    if (failed) return 1;
    STD_PRINTF("M5:T346:S4:RTC-STORAGE-READINESS:OK\n");
    return 0;
}
