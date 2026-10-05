#include "ibmpc/machine/machine_interface.h"
#include "lib/types/types_interface.h"
#include "../../../../x86/core/composition_fixture.h"
#include "../../../../ibmpc/board-common/composition_fixture.h"
#include "../../../../ibmpc/board-common/kbc_state_fixture.h"
#include <stdio.h>

#include "ibmpc/board-common/machine_board_interface.h"
#include "ibmpc/machine/lifecycle.h"
#include "ibmpc/machine/machine_private.h"
#include "ibmpc/machine/machine_interface.h"
#include "app-nxvm/profiles/default_profile/profile_interface.h"

#include "x86/core/debug_interface.h"
#include "support/rom/session_assets.h"

static lib_i32 vm_pcat_s4_topology_matches(
    const vm_machine *session,
    const vm_profile_default_pc_at_descriptor *profile)
{
    const vm_at_route *pit_route;
    const vm_at_route *keyboard_route;
    const vm_at_route *aux_route;
    const vm_at_route *cmos_route;
    const vm_at_route *fdc_route;
    test_board_composition_observation board;
    lib_size index;
    lib_i32 failed = 0;

    if (session == LIB_NULL || session->core_machine == LIB_NULL ||
        profile == LIB_NULL) return 1;
    board = test_board_capture_composition(session->board);
    for (index = 0u; index < profile->port_leaf_count; ++index) {
        const vm_at_port_leaf *leaf =
            &profile->port_leaves[index];

        failed |= test_core_port_has_read(session->core_machine, leaf->port) != leaf->read ||
            test_core_port_has_write(session->core_machine, leaf->port) != leaf->write;
    }
    pit_route = vm_at_route_find(profile->routes, profile->route_count,
        VM_AT_ROUTE_PIT_IRQ0);
    keyboard_route = vm_at_route_find(profile->routes, profile->route_count,
        VM_AT_ROUTE_KBC_KEYBOARD_IRQ1);
    aux_route = vm_at_route_find(profile->routes, profile->route_count,
        VM_AT_ROUTE_KBC_AUX_IRQ12);
    cmos_route = vm_at_route_find(profile->routes, profile->route_count,
        VM_AT_ROUTE_CMOS_IRQ8);
    fdc_route = vm_at_route_find(profile->routes, profile->route_count,
        VM_AT_ROUTE_FDC_IRQ6_DMA2);
    failed |= pit_route == LIB_NULL || keyboard_route == LIB_NULL || aux_route == LIB_NULL ||
        cmos_route == LIB_NULL || fdc_route == LIB_NULL ||
        !test_board_pic_source_matches(session->board, TEST_BOARD_PIT_IRQ0, pit_route->irq) ||
        !test_board_pic_source_matches(session->board, TEST_BOARD_KEYBOARD_IRQ1, keyboard_route->irq) ||
        !test_board_kbc_aux_enabled(session->board) ||
        !test_board_kbc_command_matches(session->board,
            session->core_machine, 0x20u, 0x20u, 0u) ||
        !test_board_pic_source_matches(session->board, TEST_BOARD_KEYBOARD_IRQ12, aux_route->irq) ||
        board.rtc_irq != cmos_route->irq ||
        board.rtc_provenance !=
            CORE_MACHINE_RTC_TIMING_L2_RATIO ||
        board.fdc.irq != fdc_route->irq ||
        board.fdc.dma_channel !=
            fdc_route->dma_channel ||
        board.hdc.irq != profile->hdc.irq;
    failed |= !test_core_port_has_read(session->core_machine, 0x0061u) ||
        !test_core_port_has_write(session->core_machine, 0x0061u) ||
        test_core_port_has_read(session->core_machine, 0x0062u) ||
        test_core_port_has_write(session->core_machine, 0x0062u) ||
        test_core_port_has_read(session->core_machine, 0x0063u) ||
        test_core_port_has_write(session->core_machine, 0x0063u) ||
        test_core_port_has_read(session->core_machine, 0x03d6u) ||
        test_core_port_has_write(session->core_machine, 0x03d6u) ||
        test_core_port_has_read(session->core_machine, 0x03d7u) ||
        test_core_port_has_write(session->core_machine, 0x03d7u) ||
        test_core_port_has_read(session->core_machine, 0x03f3u) ||
        test_core_port_has_write(session->core_machine, 0x03f3u);
    return failed;
}

static lib_i32 vm_pcat_s4_reset_state_matches(vm_machine *session,
    const vm_profile_default_pc_at_descriptor *profile)
{
    core_machine_timeline_observation timeline;
    vm_machine_reset_vector vector;
    lib_i32 nmi_masked = LIB_TRUE;

    return session == LIB_NULL || session->core_machine == LIB_NULL ||
        !session->active ||
        vm_machine_get_reset_vector(session, &vector) != LIB_STATUS_OK ||
        vector.cs != 0xf000u || vector.ip != 0xfff0u ||
        core_machine_get_timeline_observation(session->core_machine,
            &timeline) != LIB_STATUS_OK ||
        timeline.now != 0u || timeline.pending_events != 0u ||
        timeline.next_sequence != 0u ||
        core_machine_get_nmi_mask(session->core_machine, &nmi_masked) !=
            LIB_STATUS_OK || nmi_masked ||
        vm_pcat_s4_topology_matches(session, profile) != 0;
}

static lib_i32 vm_pcat_s4_reset_rearms_selected_machine(
    vm_machine *session,
    const vm_profile_default_pc_at_descriptor *profile)
{
    static const lib_u8 nop = 0x90u;
    const core_machine_debug_register_patch entry = {
        .mask = CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_CS) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_DS) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_ES) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_SS) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EIP),
        .values = { [CORE_MACHINE_DEBUG_EIP] = 0x1000u }
    };
    core_machine_run_budget budget = { 1u, 0u };
    core_machine_run_result result;
    core_machine_timeline_observation timeline;
    lib_i32 nmi_masked = LIB_FALSE;

    if (session == LIB_NULL || session->core_machine == LIB_NULL ||
        core_machine_debug_patch_registers(session->core_machine, &entry) !=
            LIB_STATUS_OK ||
        core_machine_memory_write(session->core_machine, 0x1000u, &nop,
            sizeof(nop)) != LIB_STATUS_OK ||
        core_machine_run(session->core_machine, budget, &result) != LIB_STATUS_OK ||
        result.reason != CORE_MACHINE_STOP_BUDGET || result.elapsed_ticks == 0u ||
        core_machine_get_timeline_observation(session->core_machine,
            &timeline) != LIB_STATUS_OK || timeline.now == 0u) {
        return 1;
    }
    test_core_write_port_after_run(session->core_machine, 0x0070u, 0x80u);
    if (core_machine_get_nmi_mask(session->core_machine, &nmi_masked) !=
            LIB_STATUS_OK || !nmi_masked) return 1;
    vm_machine_reset(session);
    return vm_pcat_s4_reset_state_matches(session, profile);
}

lib_i32 main(void)
{
    const vm_profile_default_pc_at_descriptor *profile =
        vm_profile_default_pc_at_descriptor_get();
    vm_machine *session = LIB_NULL;
    lib_i32 failed;

    if (profile == LIB_NULL ||
        !vm_profile_default_pc_at_descriptor_is_valid(profile) ||
        vm_test_default_pc_at_session_create(LIB_NULL, &session) != LIB_STATUS_OK ||
        session == LIB_NULL) {
        vm_machine_destroy(session);
        return 1;
    }
    failed = vm_pcat_s4_reset_state_matches(session, profile) != 0 ||
        vm_pcat_s4_reset_rearms_selected_machine(session, profile) != 0;
    vm_machine_destroy(session);
    if (failed) return 1;
    printf("M5:T353:S4:PCAT-COMPOSITION:OK\n");
    return 0;
}
