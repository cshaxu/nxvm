#include "type.h"

#include "core/machine/machine.h"
#include "support/core_machine_cpu_fixture.h"

typedef struct readiness_trace_probe {
    core_machine_trace_event events[40];
    type_unsigned_32 count;
} readiness_trace_probe;

static C_VOID readiness_trace(C_VOID *opaque,
    const core_machine_trace_event *event)
{
    readiness_trace_probe *probe = (readiness_trace_probe *)opaque;

    if (probe != STD_NULL && probe->count <
        sizeof(probe->events) / sizeof(probe->events[0])) {
        probe->events[probe->count++] = *event;
    }
}

static C_INT readiness_expect_chain(const readiness_trace_probe *probe)
{
    type_unsigned_32 index;
    type_unsigned_64 selected_tick = 3u;
    type_unsigned_32 phase = 0u;
    type_unsigned_32 groups = 0u;

    for (index = 0u; index < probe->count; ++index) {
        const core_machine_trace_event *event = &probe->events[index];

        if (event->type == CORE_MACHINE_TRACE_FDC_ADVANCE ||
            event->type == CORE_MACHINE_TRACE_HDC_ADVANCE ||
            event->type == CORE_MACHINE_TRACE_RTC_ADVANCE) {
            core_machine_trace_event_type expected = phase == 0u ?
                CORE_MACHINE_TRACE_FDC_ADVANCE : phase == 1u ?
                CORE_MACHINE_TRACE_HDC_ADVANCE : CORE_MACHINE_TRACE_RTC_ADVANCE;

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

static C_INT readiness_has_event(const readiness_trace_probe *probe,
    core_machine_trace_event_type type)
{
    type_unsigned_32 index;

    for (index = 0u; index < probe->count; ++index) {
        if (probe->events[index].type == type) return 1;
    }
    return 0;
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
    config.cpu_profile = CORE_MACHINE_CPU_PROFILE_80286;
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
        0x00fffff0u, 0x000ffff0u, 16u) != TYPE_STATUS_OK;
    failed |= !failed && core_machine_freeze_execution_providers(machine) !=
        TYPE_STATUS_OK;
    failed |= !failed && core_machine_reset(machine) != TYPE_STATUS_OK;
    failed |= !failed && core_machine_memory_write(machine, 0x00fffff0u, &nop,
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
        result.elapsed_ticks != 3u ||
        machine->shared_rtc.calendar.second != 3u);
    failed |= !failed && core_machine_get_timeline_observation(machine,
        &observation) != TYPE_STATUS_OK;
    failed |= !failed && (observation.now != 3u || observation.pending_events != 0u ||
        observation.next_sequence != 0u);
    failed |= !failed && (probe.count < 5u ||
        !readiness_has_event(&probe, CORE_MACHINE_TRACE_CPU_RETIRE) ||
        readiness_expect_chain(&probe) ||
        probe.events[probe.count - 1u].type != CORE_MACHINE_TRACE_RUN_BOUNDARY);
    failed |= !failed && core_machine_reset(machine) != TYPE_STATUS_OK;
    failed |= !failed && core_machine_get_timeline_observation(machine,
        &observation) != TYPE_STATUS_OK;
    failed |= !failed && (observation.now != 0u || observation.pending_events != 0u ||
        observation.next_sequence != 0u);

    core_machine_destroy(machine);
    if (failed) return 1;
    STD_PRINTF("M5:T346:S4:RTC-STORAGE-READINESS:OK\n");
    return 0;
}
