#include "type.h"

#include "core/machine/machine.h"
#include "core/machine/machine_interface.h"
#include "vm/composition/session/session_private.h"
#include "vm/composition/session/session_interface.h"
#include "vm/profile/default_profile/pc_at_profile.h"
#include "../support/rom/session_assets.h"

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
    core_machine_time_observation time_observation;
    C_INT failed = 0;

    if (model_339 == STD_NULL || generic == STD_NULL ||
        vm_test_ibm_5170_session_create(&config, &session) != TYPE_STATUS_OK ||
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
        generic->clock_plan.pit.numerator != 596591u ||
        generic->clock_plan.pit.denominator != 4000000u ||
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
    vm_session_destroy(session);
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
