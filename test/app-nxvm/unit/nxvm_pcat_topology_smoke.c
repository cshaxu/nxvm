#include "x86-nxvm/machine/machine_interface.h"
#include "lib/types/types_interface.h"
#include "../../x86-nxvm/board-base/composition/composition_fixture.h"
#include "../../x86-nxvm/board-base/support/composition_fixture.h"
#include "../../x86-nxvm/board-base/support/kbc_state_fixture.h"
#include "lib/types/file.h"

#include "x86-nxvm/machine/lifecycle.h"
#include "x86-nxvm/machine/machine_private.h"
#include "x86-nxvm/machine/machine_interface.h"
#include "app-nxvm/machine_config_interface.h"

#include "../support/rom/session_assets.h"

static lib_i32 pcat_topology_registry_matches_profile(
    const x86_nxvm_board_pc_at_descriptor *profile)
{
    x86_nxvm_machine *session = LIB_NULL;
    const x86_nxvm_board_at_route *pit_route;
    const x86_nxvm_board_at_route *keyboard_route;
    const x86_nxvm_board_at_route *aux_route;
    const x86_nxvm_board_at_route *cmos_route;
    const x86_nxvm_board_at_route *fdc_route;
    test_board_composition_observation board;
    lib_size index;
    lib_i32 failed = 0;

    if (vm_test_default_pc_at_session_create(LIB_NULL, &session) != LIB_STATUS_OK ||
        session == LIB_NULL || session->x86_core_machine == LIB_NULL) {
        x86_nxvm_machine_destroy(session);
        return 1;
    }
    board = test_board_capture_composition(session->board);
    for (index = 0u; index < profile->port_leaf_count; ++index) {
        const x86_nxvm_board_at_port_leaf *leaf =
            &profile->port_leaves[index];

        failed |= test_x86_nxvm_port_accepts_read(session->x86_core_machine, leaf->port) != leaf->read ||
            test_x86_nxvm_port_accepts_write(session->x86_core_machine, leaf->port) != leaf->write;
    }
    failed |= x86_core_machine_reset(session->x86_core_machine) != LIB_STATUS_OK;
    pit_route = x86_nxvm_board_at_route_find(profile->routes, profile->route_count,
        X86_NXVM_BOARD_AT_ROUTE_PIT_IRQ0);
    keyboard_route = x86_nxvm_board_at_route_find(profile->routes, profile->route_count,
        X86_NXVM_BOARD_AT_ROUTE_KBC_KEYBOARD_IRQ1);
    aux_route = x86_nxvm_board_at_route_find(profile->routes, profile->route_count,
        X86_NXVM_BOARD_AT_ROUTE_KBC_AUX_IRQ12);
    cmos_route = x86_nxvm_board_at_route_find(profile->routes, profile->route_count,
        X86_NXVM_BOARD_AT_ROUTE_CMOS_IRQ8);
    fdc_route = x86_nxvm_board_at_route_find(profile->routes, profile->route_count,
        X86_NXVM_BOARD_AT_ROUTE_FDC_IRQ6_DMA2);
    failed |= pit_route == LIB_NULL || keyboard_route == LIB_NULL || aux_route == LIB_NULL ||
        cmos_route == LIB_NULL || fdc_route == LIB_NULL ||
        !test_board_pic_source_matches(session->board, TEST_BOARD_PIT_IRQ0, pit_route->irq) ||
        !test_board_pic_source_matches(session->board, TEST_BOARD_KEYBOARD_IRQ1, keyboard_route->irq) ||
        !test_board_kbc_aux_enabled(session->board) ||
        !test_board_kbc_command_matches(session->board,
            session->x86_core_machine, 0x20u, 0x20u, 0u) ||
        !test_board_pic_source_matches(session->board, TEST_BOARD_KEYBOARD_IRQ12, aux_route->irq) ||
        board.rtc_irq != cmos_route->irq ||
        board.fdc.irq != fdc_route->irq ||
        board.fdc.dma_channel !=
            fdc_route->dma_channel ||
        board.hdc.irq !=
            profile->hdc.irq;
    failed |= !test_x86_nxvm_port_accepts_read(session->x86_core_machine, 0x0061u) ||
        !test_x86_nxvm_port_accepts_write(session->x86_core_machine, 0x0061u) ||
        test_x86_nxvm_port_accepts_read(session->x86_core_machine, 0x0062u) ||
        test_x86_nxvm_port_accepts_write(session->x86_core_machine, 0x0062u) ||
        test_x86_nxvm_port_accepts_read(session->x86_core_machine, 0x0063u) ||
        test_x86_nxvm_port_accepts_write(session->x86_core_machine, 0x0063u) ||
        test_x86_nxvm_port_accepts_read(session->x86_core_machine, 0x03d6u) ||
        test_x86_nxvm_port_accepts_write(session->x86_core_machine, 0x03d6u) ||
        test_x86_nxvm_port_accepts_read(session->x86_core_machine, 0x03d7u) ||
        test_x86_nxvm_port_accepts_write(session->x86_core_machine, 0x03d7u) ||
        test_x86_nxvm_port_accepts_read(session->x86_core_machine, 0x03f3u) ||
        test_x86_nxvm_port_accepts_write(session->x86_core_machine, 0x03f3u);
    x86_nxvm_machine_destroy(session);
    return failed;
}

static lib_i32 pcat_topology_routes_are_explicit(
    const x86_nxvm_board_pc_at_descriptor *profile)
{
    static const x86_nxvm_board_at_route expected[] = {
        { X86_NXVM_BOARD_AT_ROUTE_PIT_IRQ0, 0u,
            X86_NXVM_BOARD_AT_NO_DMA_CHANNEL },
        { X86_NXVM_BOARD_AT_ROUTE_KBC_KEYBOARD_IRQ1, 1u,
            X86_NXVM_BOARD_AT_NO_DMA_CHANNEL },
        { X86_NXVM_BOARD_AT_ROUTE_KBC_AUX_IRQ12, 12u,
            X86_NXVM_BOARD_AT_NO_DMA_CHANNEL },
        { X86_NXVM_BOARD_AT_ROUTE_CMOS_IRQ8, 8u,
            X86_NXVM_BOARD_AT_NO_DMA_CHANNEL },
        { X86_NXVM_BOARD_AT_ROUTE_FDC_IRQ6_DMA2, 6u, 2u }
    };
    lib_size index;

    if (profile->route_count != sizeof(expected) / sizeof(expected[0])) return 1;
    for (index = 0u; index < profile->route_count; ++index) {
        const x86_nxvm_board_at_route *route =
            x86_nxvm_board_at_route_find(profile->routes, profile->route_count, expected[index].source);

        if (route == LIB_NULL || lib_memory_compare(route, &expected[index],
                sizeof(*route)) != 0) return 1;
    }
    return 0;
}

static lib_i32 pcat_topology_rejects_before_registration(
    const x86_nxvm_board_pc_at_descriptor *source)
{
    x86_nxvm_board_pc_at_descriptor invalid = *source;
    x86_nxvm_board_at_port_leaf leaves[96];

    if (source->port_leaf_count > sizeof(leaves) / sizeof(leaves[0])) return 1;
    lib_memory_copy(leaves, source->port_leaves,
        source->port_leaf_count * sizeof(leaves[0]));
    leaves[0].read = LIB_FALSE;
    invalid.port_leaves = leaves;
    /* Profile now owns construction validation; Machine accepts only a
     * frozen plan and cannot be fed a second descriptor route. */
    return x86_nxvm_board_pc_at_descriptor_is_valid(&invalid) ? 1 : 0;
}

lib_i32 main(void)
{
    const x86_nxvm_board_pc_at_descriptor *profile =
        vm_profile_default_pc_at_descriptor_get();

    if (profile == LIB_NULL || !x86_nxvm_board_pc_at_descriptor_is_valid(profile) ||
        pcat_topology_registry_matches_profile(profile) != 0 ||
        pcat_topology_routes_are_explicit(profile) != 0 ||
        pcat_topology_rejects_before_registration(profile) != 0) {
        return 1;
    }
    lib_c_printf("PCAT-TOPOLOGY:OK\n");
    return 0;
}
