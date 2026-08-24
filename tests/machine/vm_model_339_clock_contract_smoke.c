#include "type.h"

#include "core/machine/clock.h"
#include "core/machine/machine.h"
#include "core/machine/machine_interface.h"
#include "vm/composition/session/session_private.h"
#include "vm/composition/session/session_interface.h"
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
    C_INT failed = 0;

    if (model_339 == STD_NULL || generic == STD_NULL ||
        vm_session_create(&config, &session) != TYPE_STATUS_OK ||
        session == STD_NULL) {
        vm_session_destroy(session);
        return 1;
    }

    failed |= model_339->clock_plan.pit.numerator != 596591u ||
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
    failed |= session->profile != model_339 ||
        session->core_machine->pit_clock.numerator != 596591u ||
        session->core_machine->pit_clock.denominator != 4000000u ||
        session->core_machine->pit_clock.reset_phase != 0u ||
        session->core_machine->rtc_clock.numerator != 64u ||
        session->core_machine->rtc_clock.denominator != 15625u ||
        session->core_machine->rtc_clock.reset_phase != 0u ||
        session->core_machine->vadp_clock.numerator != 315u ||
        session->core_machine->vadp_clock.denominator != 1408u ||
        session->core_machine->vadp_clock.reset_phase != 0u ||
        session->core_machine->shared_rtc.ticks_per_second != 32768u;
    failed |= session->core_machine->kbc_typematic_initial_ticks != 4000000u ||
        session->core_machine->kbc_typematic_repeat_ticks != 800000u ||
        session->core_machine->kbc_command_response_ticks != 0u;
    failed |= session->core_machine->shared_kbc.data.typematic != 0x2cu ||
        session->core_machine->shared_kbc.data.typematic_initial_ticks != 4000000u ||
        session->core_machine->shared_kbc.data.typematic_repeat_ticks != 800000u;
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
    return 0;
}
