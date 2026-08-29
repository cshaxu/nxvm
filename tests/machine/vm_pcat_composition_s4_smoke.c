#include "type.h"

#include "core/machine/machine.h"
#include "core/machine/machine_interface.h"
#include "core/machine/port.h"
#include "vm/composition/session/lifecycle.h"
#include "vm/composition/session/session_private.h"
#include "vm/composition/session/session_interface.h"
#include "vm/profile/default_profile/pc_at_profile.h"

#include "../support/core_machine_cpu_fixture.h"

static C_INT vm_pcat_s4_topology_matches(
    const vm_session *session,
    const vm_profile_default_pc_at_descriptor *profile)
{
    const vm_profile_default_pc_at_route *pit_route;
    const vm_profile_default_pc_at_route *keyboard_route;
    const vm_profile_default_pc_at_route *aux_route;
    const vm_profile_default_pc_at_route *cmos_route;
    const vm_profile_default_pc_at_route *fdc_route;
    STD_SIZE_T index;
    C_INT failed = 0;

    if (session == STD_NULL || session->core_machine == STD_NULL ||
        profile == STD_NULL) return 1;
    for (index = 0u; index < profile->port_leaf_count; ++index) {
        const vm_profile_default_pc_at_port_leaf *leaf =
            &profile->port_leaves[index];

        failed |= core_machine_port_has_read(
            &session->core_machine->executor_port, leaf->port) != leaf->read ||
            core_machine_port_has_write(
                &session->core_machine->executor_port, leaf->port) != leaf->write;
    }
    pit_route = vm_profile_default_pc_at_route_find(profile,
        VM_PROFILE_DEFAULT_PC_AT_ROUTE_PIT_IRQ0);
    keyboard_route = vm_profile_default_pc_at_route_find(profile,
        VM_PROFILE_DEFAULT_PC_AT_ROUTE_KBC_KEYBOARD_IRQ1);
    aux_route = vm_profile_default_pc_at_route_find(profile,
        VM_PROFILE_DEFAULT_PC_AT_ROUTE_KBC_AUX_IRQ12);
    cmos_route = vm_profile_default_pc_at_route_find(profile,
        VM_PROFILE_DEFAULT_PC_AT_ROUTE_CMOS_IRQ8);
    fdc_route = vm_profile_default_pc_at_route_find(profile,
        VM_PROFILE_DEFAULT_PC_AT_ROUTE_FDC_IRQ6_DMA2);
    failed |= pit_route == STD_NULL || keyboard_route == STD_NULL ||
        aux_route == STD_NULL || cmos_route == STD_NULL || fdc_route == STD_NULL ||
        session->core_machine->shared_pit_irq0_source.irq != pit_route->irq ||
        session->core_machine->shared_kbc.connect.irq1_source.irq !=
            keyboard_route->irq ||
        session->core_machine->shared_kbc.connect.irq12_source.irq !=
            aux_route->irq ||
        session->core_machine->rtc_cmos_config.irq != cmos_route->irq ||
        session->core_machine->rtc_cmos_config.timing.provenance !=
            CORE_MACHINE_RTC_TIMING_L2_RATIO ||
        session->core_machine->fdc_topology.config.irq != fdc_route->irq ||
        session->core_machine->fdc_topology.config.dma_channel !=
            fdc_route->dma_channel ||
        session->core_machine->hdc_topology.config.irq != profile->hdc.irq;
    failed |= !core_machine_port_has_read(&session->core_machine->executor_port,
            0x0061u) ||
        !core_machine_port_has_write(&session->core_machine->executor_port,
            0x0061u) ||
        core_machine_port_has_read(&session->core_machine->executor_port,
            0x0062u) ||
        core_machine_port_has_write(&session->core_machine->executor_port,
            0x0062u) ||
        core_machine_port_has_read(&session->core_machine->executor_port,
            0x0063u) ||
        core_machine_port_has_write(&session->core_machine->executor_port,
            0x0063u) ||
        core_machine_port_has_read(&session->core_machine->executor_port,
            0x03d6u) ||
        core_machine_port_has_write(&session->core_machine->executor_port,
            0x03d6u) ||
        core_machine_port_has_read(&session->core_machine->executor_port,
            0x03d7u) ||
        core_machine_port_has_write(&session->core_machine->executor_port,
            0x03d7u) ||
        core_machine_port_has_read(&session->core_machine->executor_port,
            0x03f3u) ||
        core_machine_port_has_write(&session->core_machine->executor_port,
            0x03f3u);
    return failed;
}

static C_INT vm_pcat_s4_reset_state_matches(vm_session *session,
    const vm_profile_default_pc_at_descriptor *profile)
{
    core_machine_timeline_observation timeline;
    vm_session_reset_vector vector;
    C_INT nmi_masked = TYPE_TRUE;

    return session == STD_NULL || session->core_machine == STD_NULL ||
        !session->active ||
        vm_session_get_reset_vector(session, &vector) != TYPE_STATUS_OK ||
        vector.cs != 0xf000u || vector.ip != 0xfff0u ||
        core_machine_get_timeline_observation(session->core_machine,
            &timeline) != TYPE_STATUS_OK ||
        timeline.now != 0u || timeline.pending_events != 0u ||
        timeline.next_sequence != 0u ||
        core_machine_get_nmi_mask(session->core_machine, &nmi_masked) !=
            TYPE_STATUS_OK || nmi_masked ||
        vm_pcat_s4_topology_matches(session, profile) != 0;
}

static C_INT vm_pcat_s4_reset_rearms_selected_machine(
    vm_session *session,
    const vm_profile_default_pc_at_descriptor *profile)
{
    static const type_unsigned_8 nop = 0x90u;
    core_machine_run_budget budget = { 1u, 0u };
    core_machine_run_result result;
    core_machine_timeline_observation timeline;
    C_INT nmi_masked = TYPE_FALSE;

    if (session == STD_NULL || session->core_machine == STD_NULL ||
        !test_core_machine_fixture_prepare_real_mode_execution(
            session->core_machine, 0x1000u) ||
        core_machine_memory_write(session->core_machine, 0x1000u, &nop,
            sizeof(nop)) != TYPE_STATUS_OK ||
        core_machine_run(session->core_machine, budget, &result) != TYPE_STATUS_OK ||
        result.reason != CORE_MACHINE_STOP_BUDGET || result.elapsed_ticks == 0u ||
        core_machine_get_timeline_observation(session->core_machine,
            &timeline) != TYPE_STATUS_OK || timeline.now == 0u) {
        return 1;
    }
    core_machine_port_write(&session->core_machine->executor_port,
        0x0070u, 0x80u);
    if (core_machine_get_nmi_mask(session->core_machine, &nmi_masked) !=
            TYPE_STATUS_OK || !nmi_masked) return 1;
    vm_session_reset(session);
    return vm_pcat_s4_reset_state_matches(session, profile);
}

C_INT main(C_VOID)
{
    const vm_profile_default_pc_at_descriptor *profile =
        vm_profile_default_pc_at_descriptor_get();
    vm_session *session = STD_NULL;
    C_INT failed;

    if (profile == STD_NULL ||
        !vm_profile_default_pc_at_descriptor_is_valid(profile) ||
        vm_session_create(STD_NULL, &session) != TYPE_STATUS_OK ||
        session == STD_NULL) {
        vm_session_destroy(session);
        return 1;
    }
    failed = vm_pcat_s4_reset_state_matches(session, profile) != 0 ||
        vm_pcat_s4_reset_rearms_selected_machine(session, profile) != 0;
    vm_session_destroy(session);
    if (failed) return 1;
    STD_PRINTF("M5:T353:S4:PCAT-COMPOSITION:OK\n");
    return 0;
}
