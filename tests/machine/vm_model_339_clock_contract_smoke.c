#include "type.h"

#include "core/machine/clock.h"
#include "core/machine/machine.h"
#include "core/machine/machine_interface.h"
#include "vm/composition/session/session_private.h"
#include "vm/composition/session/session_interface.h"
#include "vm/composition/session/waiting.h"
#include "vm/profile/default_profile/pc_at_profile.h"

static C_INT vm_model_339_clock_contract_is_selected(C_VOID)
{
    const vm_profile_default_pc_at_descriptor *model_339 =
        vm_profile_ibm_5170_model_339_descriptor_get();
    const vm_profile_default_pc_at_descriptor *generic =
        vm_profile_default_pc_at_descriptor_get();
    const vm_session_config config = {
        .profile_kind = VM_SESSION_PROFILE_IBM_5170_MODEL_339
    };
    vm_session *session = STD_NULL;
    vm_session *fallback = STD_NULL;
    core_machine_time_observation time_observation;
    const core_machine_run_result waiting = {
        .reason = CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT
    };
    type_bool advanced;
    C_INT session_advanced;
    C_INT failed = 0;

    if (model_339 == STD_NULL || generic == STD_NULL ||
        vm_session_create(&config, &session) != TYPE_STATUS_OK ||
        session == STD_NULL) {
        vm_session_destroy(session);
        return 1;
    }

    failed |= model_339->clock_plan.dma.numerator != 3u ||
        model_339->clock_plan.dma.denominator != 8u ||
        model_339->clock_plan.pit.numerator != 596591u ||
        model_339->clock_plan.pit.denominator != 4000000u ||
        model_339->clock_plan.pit.reset_phase != 0u ||
        model_339->clock_plan.rtc.numerator != 64u ||
        model_339->clock_plan.rtc.denominator != 15625u ||
        model_339->clock_plan.rtc.reset_phase != 0u ||
        model_339->clock_plan.vadp.numerator != 315u ||
        model_339->clock_plan.vadp.denominator != 1408u ||
        model_339->clock_plan.vadp.reset_phase != 0u ||
        model_339->rtc_ticks_per_second != 32768u ||
        model_339->kbc_typematic_initial_ticks != 4000000u ||
        model_339->kbc_typematic_repeat_ticks != 800000u ||
        model_339->kbc_command_response_ticks != 0u ||
        generic->clock_plan.pit.numerator != 1u ||
        generic->clock_plan.pit.denominator != 4u ||
        generic->clock_plan.rtc.numerator != 1u ||
        generic->clock_plan.rtc.denominator != 1u ||
        generic->rtc_ticks_per_second != 50000u;
    failed |= session->profile == model_339 ||
        STD_STRCMP(session->profile->identity, "pc-at-5170") != 0 ||
        session->core_machine_config.memory_bytes !=
            session->ibm_5170_root.resolved.values.core.configuration.memory_bytes ||
        session->core_machine_config.time_axis.kind !=
            session->ibm_5170_root.resolved.values.core.configuration.time_axis.kind ||
        session->controller_timing_rules.dma_service !=
            session->ibm_5170_root.resolved.values.core.controller_timing_rules.dma_service ||
        session->core_machine->dma_clock.numerator != 3u ||
        session->core_machine->dma_clock.denominator != 8u ||
        session->core_machine->pit_clock.numerator != 596591u ||
        session->core_machine->pit_clock.denominator != 4000000u ||
        session->core_machine->pit_clock.reset_phase != 0u ||
        session->core_machine->rtc_clock.numerator != 64u ||
        session->core_machine->rtc_clock.denominator != 15625u ||
        session->core_machine->rtc_clock.reset_phase != 0u ||
        session->core_machine->vadp_clock.numerator != 315u ||
        session->core_machine->vadp_clock.denominator != 1408u ||
        session->core_machine->vadp_clock.reset_phase != 0u ||
        session->core_machine->shared_rtc.ticks_per_second != 32768u ||
        session->core_machine->rtc_cmos_config.timing.provenance !=
            CORE_MACHINE_RTC_TIMING_L3_SOURCE;
    failed |= session->core_machine->timing_plan.controller_timing.dma_clock !=
        CORE_MACHINE_CONTROLLER_TIMING_RULE_SOURCE_RATIONAL_CLOCK ||
        session->core_machine->timing_plan.controller_timing.dma_service !=
        CORE_MACHINE_CONTROLLER_TIMING_RULE_SOURCE_DMA_SERVICE_PHASES ||
        session->core_machine->timing_plan.controller_timing.pit_clock !=
        CORE_MACHINE_CONTROLLER_TIMING_RULE_SOURCE_RATIONAL_CLOCK ||
        session->core_machine->timing_plan.controller_timing.rtc_clock !=
        CORE_MACHINE_CONTROLLER_TIMING_RULE_SOURCE_RATIONAL_CLOCK;
    {
        core_machine_timing_disposition disposition;

        failed |= core_machine_get_timing_disposition(session->core_machine,
            CORE_MACHINE_TIMING_CAPABILITY_CTRL_PIC, &disposition) != TYPE_STATUS_OK ||
            disposition != CORE_MACHINE_TIMING_DISPOSITION_L2_FALLBACK;
        failed |= core_machine_get_timing_disposition(session->core_machine,
            CORE_MACHINE_TIMING_CAPABILITY_CTRL_RTC_CMOS, &disposition) !=
            TYPE_STATUS_OK || disposition != CORE_MACHINE_TIMING_DISPOSITION_L3_REQUIRED;
        failed |= core_machine_get_timing_disposition(session->core_machine,
            CORE_MACHINE_TIMING_CAPABILITY_CTRL_DMA, &disposition) != TYPE_STATUS_OK ||
            disposition != CORE_MACHINE_TIMING_DISPOSITION_L3_REQUIRED;
        failed |= core_machine_get_timing_disposition(session->core_machine,
            CORE_MACHINE_TIMING_CAPABILITY_CTRL_PIT, &disposition) != TYPE_STATUS_OK ||
            disposition != CORE_MACHINE_TIMING_DISPOSITION_L3_REQUIRED;
    }
    failed |= session->core_machine->kbc_typematic_initial_ticks != 4000000u ||
        session->core_machine->kbc_typematic_repeat_ticks != 800000u ||
        session->core_machine->kbc_command_response_ticks != 0u;
    failed |= session->core_machine->shared_kbc.data.typematic != 0x2cu ||
        session->core_machine->shared_kbc.data.typematic_initial_ticks != 4000000u ||
        session->core_machine->shared_kbc.data.typematic_repeat_ticks != 800000u;
    failed |= core_machine_capture_time_observation(session->core_machine,
        &time_observation) != TYPE_STATUS_OK || !time_observation.pacing_time_available ||
        time_observation.pacing_ticks_per_second != 8000000u ||
        time_observation.physical_time_available ||
        time_observation.physical_ticks_per_second != 0u;
    core_machine_port_write(&session->core_machine->executor_port, 0x0060u, 0xf3u);
    failed |= core_machine_port_read(&session->core_machine->executor_port, 0x0060u) != 0xfau;
    core_machine_port_write(&session->core_machine->executor_port, 0x0060u, 0x7fu);
    failed |= core_machine_port_read(&session->core_machine->executor_port, 0x0060u) != 0xfau ||
        session->core_machine->shared_kbc.data.typematic_initial_ticks != 8000000u ||
        session->core_machine->shared_kbc.data.typematic_repeat_ticks != 4000000u;
    failed |= core_machine_clock_domain_advance(&session->core_machine->pit_clock,
        4000000u) != 596591u ||
        core_machine_clock_domain_advance(&session->core_machine->rtc_clock,
        15625u) != 64u ||
        core_machine_clock_domain_advance(&session->core_machine->vadp_clock,
        1408u) != 315u;
    failed |= core_machine_reset(session->core_machine) != TYPE_STATUS_OK ||
        session->core_machine->shared_kbc.data.typematic != 0x2cu ||
        session->core_machine->shared_kbc.data.typematic_initial_ticks != 4000000u ||
        session->core_machine->shared_kbc.data.typematic_repeat_ticks != 800000u ||
        core_machine_clock_domain_advance(&session->core_machine->pit_clock,
        4000000u) != 596591u ||
        core_machine_clock_domain_advance(&session->core_machine->rtc_clock,
        15625u) != 64u ||
        core_machine_clock_domain_advance(&session->core_machine->vadp_clock,
        1408u) != 315u;
    core_machine_port_write(&session->core_machine->executor_port, 0x0070u, 0x0bu);
    core_machine_port_write(&session->core_machine->executor_port, 0x0071u, 0x42u);
    failed |= core_machine_capture_time_observation(session->core_machine,
        &time_observation) != TYPE_STATUS_OK || !time_observation.next_deadline_valid ||
        time_observation.elapsed_ticks != 0u || time_observation.next_deadline_tick != 7u;
    STD_ATOMIC_STORE(&session->control.flagRun, TYPE_TRUE);
    session->speed = VM_SESSION_SPEED_STANDARD;
    vm_session_pacing_reset(session);
    session_advanced = 0;
    failed |= vm_session_waiting_advance(session, &waiting, &session_advanced) !=
        TYPE_STATUS_OK || !session_advanced || !session->pacing_origin_valid ||
        core_machine_capture_time_observation(session->core_machine,
        &time_observation) != TYPE_STATUS_OK || time_observation.elapsed_ticks != 7u;
    failed |= core_machine_reset(session->core_machine) != TYPE_STATUS_OK;
    core_machine_port_write(&session->core_machine->executor_port, 0x0043u, 0x34u);
    core_machine_port_write(&session->core_machine->executor_port, 0x0040u, 4u);
    core_machine_port_write(&session->core_machine->executor_port, 0x0040u, 0u);
    session->core_machine->time_axis = (core_machine_time_axis) {
        CORE_MACHINE_TIME_AXIS_MACRO_PROPORTIONAL, 8000000u };
    session->core_machine->retirement_time_contract =
        CORE_MACHINE_RETIREMENT_TIME_DETERMINISTIC;
    session->speed = VM_SESSION_SPEED_TURBO;
    vm_session_pacing_reset(session);
    session_advanced = 0;
    failed |= vm_session_waiting_advance(session, &waiting, &session_advanced) !=
        TYPE_STATUS_OK || !session_advanced || session->pacing_origin_valid ||
        core_machine_capture_time_observation(session->core_machine,
        &time_observation) != TYPE_STATUS_OK || time_observation.elapsed_ticks != 7u;
    STD_ATOMIC_STORE(&session->control.flagRun, TYPE_FALSE);
    core_machine_pit_set_gate(&session->core_machine->shared_pit, 1u, TYPE_FALSE);
    failed |= core_machine_capture_time_observation(session->core_machine,
        &time_observation) != TYPE_STATUS_OK || !time_observation.next_deadline_valid ||
        time_observation.next_deadline_tick != 27u;
    advanced = TYPE_FALSE;
    failed |= core_machine_advance_to_next_deadline(session->core_machine,
        &advanced) != TYPE_STATUS_OK || !advanced ||
        core_machine_capture_time_observation(session->core_machine,
        &time_observation) != TYPE_STATUS_OK || time_observation.elapsed_ticks != 27u;
    failed |= core_machine_reset(session->core_machine) != TYPE_STATUS_OK;
    core_machine_port_write(&session->core_machine->executor_port, 0x0043u, 0x34u);
    core_machine_port_write(&session->core_machine->executor_port, 0x0040u, 4u);
    core_machine_port_write(&session->core_machine->executor_port, 0x0040u, 0u);
    failed |= core_machine_capture_time_observation(session->core_machine,
        &time_observation) != TYPE_STATUS_OK || !time_observation.next_deadline_valid ||
        time_observation.elapsed_ticks != 0u || time_observation.next_deadline_tick != 7u;
    session->core_machine->shared_kbc.data.response_remaining_ticks = 1u;
    failed |= core_machine_capture_time_observation(session->core_machine,
        &time_observation) != TYPE_STATUS_OK || time_observation.next_deadline_valid;
    session->core_machine->shared_kbc.data.response_remaining_ticks = 0u;
    session->core_machine->fdc.data.seek_pending[0] = TYPE_TRUE;
    failed |= core_machine_capture_time_observation(session->core_machine,
        &time_observation) != TYPE_STATUS_OK || time_observation.next_deadline_valid;
    session->core_machine->fdc.data.seek_pending[0] = TYPE_FALSE;
    session->core_machine->fdc.data.phase = core_machine_fdc_PHASE_RESULT;
    failed |= core_machine_capture_time_observation(session->core_machine,
        &time_observation) != TYPE_STATUS_OK || time_observation.next_deadline_valid;
    session->core_machine->fdc.data.phase = core_machine_fdc_PHASE_COMMAND;
    session->core_machine->hdc.data.phase = CORE_MACHINE_HDC_PHASE_PENDING_COMMAND;
    failed |= core_machine_capture_time_observation(session->core_machine,
        &time_observation) != TYPE_STATUS_OK || time_observation.next_deadline_valid;
    session->core_machine->hdc.data.phase = CORE_MACHINE_HDC_PHASE_IDLE;
    session->core_machine->shared_dma_primary.data.isr = 1u;
    failed |= core_machine_capture_time_observation(session->core_machine,
        &time_observation) != TYPE_STATUS_OK || time_observation.next_deadline_valid;
    session->core_machine->shared_dma_primary.data.isr = 0u;
    advanced = TYPE_FALSE;
    failed |= core_machine_advance_to_next_deadline(session->core_machine,
        &advanced) != TYPE_STATUS_OK || !advanced ||
        core_machine_capture_time_observation(session->core_machine,
        &time_observation) != TYPE_STATUS_OK || time_observation.elapsed_ticks != 7u;

    /* A source-qualified PIT deadline uses the selected macro axis; it stays
     * distinct from a verified physical Core axis. */
    failed |= core_machine_reset(session->core_machine) != TYPE_STATUS_OK;
    core_machine_port_write(&session->core_machine->executor_port, 0x0043u, 0x34u);
    core_machine_port_write(&session->core_machine->executor_port, 0x0040u, 4u);
    core_machine_port_write(&session->core_machine->executor_port, 0x0040u, 0u);
    session->core_machine->time_axis = (core_machine_time_axis) {
        CORE_MACHINE_TIME_AXIS_MACRO_PROPORTIONAL, 8000000u };
    session->core_machine->retirement_time_contract =
        CORE_MACHINE_RETIREMENT_TIME_DETERMINISTIC;
    advanced = TYPE_FALSE;
    failed |= core_machine_advance_to_next_deadline(session->core_machine,
        &advanced) != TYPE_STATUS_OK || !advanced ||
        core_machine_capture_time_observation(session->core_machine,
        &time_observation) != TYPE_STATUS_OK || time_observation.elapsed_ticks != 7u;

    vm_session_destroy(session);
    if (vm_session_create(STD_NULL, &fallback) != TYPE_STATUS_OK) {
        return 1;
    }
    core_machine_port_write(&fallback->core_machine->executor_port, 0x0043u, 0x34u);
    core_machine_port_write(&fallback->core_machine->executor_port, 0x0040u, 4u);
    core_machine_port_write(&fallback->core_machine->executor_port, 0x0040u, 0u);
    failed |= core_machine_capture_time_observation(fallback->core_machine,
        &time_observation) != TYPE_STATUS_OK || time_observation.next_deadline_valid ||
        time_observation.pacing_time_available || time_observation.pacing_ticks_per_second != 0u ||
        time_observation.physical_time_available ||
        time_observation.physical_ticks_per_second != 0u;
    advanced = TYPE_FALSE;
    failed |= core_machine_advance_to_next_deadline(fallback->core_machine,
        &advanced) != TYPE_STATUS_OK || advanced;
    vm_session_destroy(fallback);
    return failed;
}

C_INT main(C_VOID)
{
    if (vm_model_339_clock_contract_is_selected()) return 1;
    STD_PRINTF("M5:T375:S2:MODEL339-CLOCK-CONTRACT:OK\n");
    STD_PRINTF("M5:T375:S13:MODEL339-CGA-REFERENCE-CONTRACT:OK\n");
    STD_PRINTF("M5:T375:S22:MODEL339-TYPEMATIC:OK\n");
    STD_PRINTF("M5:T375:S23:KBC-F3-CADENCE:OK\n");
    STD_PRINTF("M5:T462:S3:CONTROLLER-PROFILE-SELECTION:OK\n");
    STD_PRINTF("M5:T462:S3:CONTROLLER-OWNER-CONSUMPTION:OK\n");
    STD_PRINTF("M5:T469:S3:CORE-DEADLINE-SELECTION:OK\n");
    STD_PRINTF("M5:T476:S3:IBM5170-ROOT-CUTOVER:OK\n");
    return 0;
}
