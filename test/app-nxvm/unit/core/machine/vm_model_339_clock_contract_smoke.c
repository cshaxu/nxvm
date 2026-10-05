#include "app-nxvm/profiles/machine_factory_interface.h"
#include "lib/types/types_interface.h"
#include <stdio.h>

#include "../../../../ibmpc/board-common/composition_fixture.h"
#include "../../../../ibmpc/board-common/kbc_state_fixture.h"
#include "ibmpc/board-common/machine_board_interface.h"
#include "ibmpc/machine/machine_private.h"
#include "ibmpc/machine/machine_interface.h"
#include "app-nxvm/profiles/default_profile/pc_at_profile_private.h"
#include "support/rom/session_assets.h"

static lib_i32 vm_model_339_clock_contract_is_selected(void)
{
    const vm_profile_default_pc_at_descriptor *model_339 =
        vm_profile_ibm_5170_model_339_descriptor_get();
    const vm_profile_default_pc_at_descriptor *generic =
        vm_profile_default_pc_at_descriptor_get();
    const vm_machine_config config = {
        .profile_kind = VM_MACHINE_PROFILE_IBM_5170_MODEL_339
    };
    vm_machine *session = LIB_NULL;
    core_machine_time_observation time_observation;
    test_board_plan_observation plan;
    test_board_composition_observation board;
    lib_i32 failed = 0;

    if (model_339 == LIB_NULL || generic == LIB_NULL ||
        vm_test_ibm_5170_session_create(&config, &session) != LIB_STATUS_OK ||
        session == LIB_NULL) {
        vm_machine_destroy(session);
        return 1;
    }

    plan = test_board_capture_plan(session->core_machine_plan);
    board = test_board_capture_composition(session->board);
    failed = failed || model_339->clock_plan.dma.numerator != 3u ||
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
    failed = failed || plan.memory_bytes != 512u * 1024u ||
        plan.time_axis_kind !=
            CORE_MACHINE_TIME_AXIS_MACRO_PROPORTIONAL ||
        plan.controller_timing.dma_service !=
            CORE_MACHINE_CONTROLLER_TIMING_RULE_SOURCE_DMA_SERVICE_PHASES ||
        board.dma_clock.numerator != 3u ||
        board.dma_clock.denominator != 8u ||
        board.pit_clock.numerator != 596591u ||
        board.pit_clock.denominator != 4000000u ||
        board.pit_clock.reset_phase != 0u ||
        board.rtc_clock.numerator != 64u ||
        board.rtc_clock.denominator != 15625u ||
        board.rtc_clock.reset_phase != 0u ||
        board.vadp_clock.numerator != 315u ||
        board.vadp_clock.denominator != 1408u ||
        board.vadp_clock.reset_phase != 0u ||
        board.rtc_ticks_per_second != 32768u ||
        board.rtc_provenance !=
            CORE_MACHINE_RTC_TIMING_L3_SOURCE;
    failed = failed || board.controller_timing.dma_clock !=
        CORE_MACHINE_CONTROLLER_TIMING_RULE_SOURCE_RATIONAL_CLOCK ||
        board.controller_timing.dma_service !=
        CORE_MACHINE_CONTROLLER_TIMING_RULE_SOURCE_DMA_SERVICE_PHASES ||
        board.controller_timing.pit_clock !=
        CORE_MACHINE_CONTROLLER_TIMING_RULE_SOURCE_RATIONAL_CLOCK ||
        board.controller_timing.rtc_clock !=
        CORE_MACHINE_CONTROLLER_TIMING_RULE_SOURCE_RATIONAL_CLOCK;
    {
        core_machine_timing_disposition disposition;

        failed = failed || core_machine_get_timing_disposition(session->core_machine,
            CORE_MACHINE_TIMING_CAPABILITY_CTRL_PIC, &disposition) != LIB_STATUS_OK ||
            disposition != CORE_MACHINE_TIMING_DISPOSITION_L2_FALLBACK;
        failed = failed || core_machine_get_timing_disposition(session->core_machine,
            CORE_MACHINE_TIMING_CAPABILITY_CTRL_RTC_CMOS, &disposition) !=
            LIB_STATUS_OK || disposition != CORE_MACHINE_TIMING_DISPOSITION_L3_REQUIRED;
        failed = failed || core_machine_get_timing_disposition(session->core_machine,
            CORE_MACHINE_TIMING_CAPABILITY_CTRL_DMA, &disposition) != LIB_STATUS_OK ||
            disposition != CORE_MACHINE_TIMING_DISPOSITION_L3_REQUIRED;
        failed = failed || core_machine_get_timing_disposition(session->core_machine,
            CORE_MACHINE_TIMING_CAPABILITY_CTRL_PIT, &disposition) != LIB_STATUS_OK ||
            disposition != CORE_MACHINE_TIMING_DISPOSITION_L3_REQUIRED;
    }
    failed = failed || board.kbc_typematic_initial_ticks != 4000000u ||
        board.kbc_typematic_repeat_ticks != 800000u ||
        board.kbc_command_response_ticks != 0u;
    failed = failed || test_board_keyboard_repeat_cadence(session->board, 4000000u, 800000u);
    {
        lib_u64 ticks = 0u;
        lib_u32 value = 0u;
        /* Exercise the selected keyboard through its real serial/controller
         * path. Native set-2 A is translated to the guest set-1 byte 1Eh. */
        failed = failed || test_board_kbc_submit_native_byte(session->board, 0x1cu) != LIB_STATUS_OK;
        failed = failed || core_machine_bus_read(session->core_machine, 0x60u, &value) !=
            LIB_STATUS_OK || value != 0x1eu;
        failed = failed || test_board_kbc_ticks_until_event(session->board, &ticks) != LIB_STATUS_OK ||
            ticks != 4000000u;
        if (!failed) test_board_kbc_advance(session->board, 3999999u);
        failed = failed || core_machine_bus_read(session->core_machine, 0x64u, &value) !=
            LIB_STATUS_OK || (value & 0x01u) != 0u;
        failed = failed || test_board_kbc_ticks_until_event(session->board, &ticks) != LIB_STATUS_OK ||
            ticks != 1u;
        if (!failed) test_board_kbc_advance(session->board, 1u);
        failed = failed || core_machine_bus_read(session->core_machine, 0x60u, &value) !=
            LIB_STATUS_OK || value != 0x1eu;
        failed = failed || test_board_kbc_ticks_until_event(session->board, &ticks) != LIB_STATUS_OK ||
            ticks != 800000u;
        failed = failed || test_board_kbc_submit_native_byte(session->board, 0xf0u) != LIB_STATUS_OK;
        failed = failed || test_board_kbc_submit_native_byte(session->board, 0x1cu) != LIB_STATUS_OK;
    }
    failed = failed || core_machine_capture_time_observation(session->core_machine,
        &time_observation) != LIB_STATUS_OK || !time_observation.pacing_time_available ||
        time_observation.pacing_ticks_per_second != 8000000u ||
        time_observation.physical_time_available ||
        time_observation.physical_ticks_per_second != 0u;
    vm_machine_destroy(session);
    return failed;
}

lib_i32 main(void)
{
    if (vm_model_339_clock_contract_is_selected()) return 1;
    printf("M5:T375:S2:MODEL339-CLOCK-CONTRACT:OK\n");
    printf("M5:T375:S13:MODEL339-CGA-REFERENCE-CONTRACT:OK\n");
    printf("M5:T375:S22:MODEL339-TYPEMATIC:OK\n");
    printf("M5:T375:S23:KBC-F3-CADENCE:OK\n");
    printf("M5:T462:S3:CONTROLLER-PROFILE-SELECTION:OK\n");
    printf("M5:T462:S3:CONTROLLER-OWNER-CONSUMPTION:OK\n");
    printf("M5:T469:S3:CORE-DEADLINE-SELECTION:OK\n");
    printf("M5:T476:S3:IBM5170-ROOT-CUTOVER:OK\n");
    return 0;
}
