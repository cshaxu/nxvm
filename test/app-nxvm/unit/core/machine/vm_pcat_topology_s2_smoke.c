#include "lib/types/types_interface.h"
#include "type.h"

#include "app-nxvm/devices/machine.h"
#include "app-nxvm/devices/port.h"
#include "app-nxvm/machine/lifecycle.h"
#include "app-nxvm/machine/machine_private.h"
#include "app-nxvm/machine/machine_interface.h"
#include "app-nxvm/profiles/default_profile/pc_at_profile_private.h"

#include "support/rom/session_assets.h"

static C_INT pcat_topology_registry_matches_profile(
    const vm_profile_default_pc_at_descriptor *profile)
{
    vm_machine *session = LIB_NULL;
    const vm_profile_default_pc_at_route *pit_route;
    const vm_profile_default_pc_at_route *keyboard_route;
    const vm_profile_default_pc_at_route *aux_route;
    const vm_profile_default_pc_at_route *cmos_route;
    const vm_profile_default_pc_at_route *fdc_route;
    lib_size index;
    C_INT failed = 0;

    if (vm_test_default_pc_at_session_create(LIB_NULL, &session) != TYPE_STATUS_OK ||
        session == LIB_NULL || session->core_machine == LIB_NULL) {
        vm_machine_destroy(session);
        return 1;
    }
    for (index = 0u; index < profile->port_leaf_count; ++index) {
        const vm_profile_default_pc_at_port_leaf *leaf =
            &profile->port_leaves[index];

        failed |= core_machine_port_has_read(&session->core_machine->executor_port,
            leaf->port) != leaf->read ||
            core_machine_port_has_write(&session->core_machine->executor_port,
                leaf->port) != leaf->write;
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
    failed |= pit_route == LIB_NULL || keyboard_route == LIB_NULL || aux_route == LIB_NULL ||
        cmos_route == LIB_NULL || fdc_route == LIB_NULL ||
        session->core_machine->shared_pit_irq0_source.irq != pit_route->irq ||
        session->core_machine->shared_kbc.connect.irq1_source.irq !=
            keyboard_route->irq ||
        !session->core_machine->shared_kbc.connect.aux_present ||
        session->core_machine->shared_kbc.connect.irq12_source.irq != aux_route->irq ||
        session->core_machine->rtc_cmos_config.irq != cmos_route->irq ||
        session->core_machine->fdc_topology.config.irq != fdc_route->irq ||
        session->core_machine->fdc_topology.config.dma_channel !=
            fdc_route->dma_channel ||
        session->core_machine->hdc_topology.config.irq !=
            profile->hdc.irq;
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
    vm_machine_destroy(session);
    return failed;
}

static C_INT pcat_topology_routes_are_explicit(
    const vm_profile_default_pc_at_descriptor *profile)
{
    static const vm_profile_default_pc_at_route expected[] = {
        { VM_PROFILE_DEFAULT_PC_AT_ROUTE_PIT_IRQ0, 0u,
            VM_PROFILE_DEFAULT_PC_AT_NO_DMA_CHANNEL },
        { VM_PROFILE_DEFAULT_PC_AT_ROUTE_KBC_KEYBOARD_IRQ1, 1u,
            VM_PROFILE_DEFAULT_PC_AT_NO_DMA_CHANNEL },
        { VM_PROFILE_DEFAULT_PC_AT_ROUTE_KBC_AUX_IRQ12, 12u,
            VM_PROFILE_DEFAULT_PC_AT_NO_DMA_CHANNEL },
        { VM_PROFILE_DEFAULT_PC_AT_ROUTE_CMOS_IRQ8, 8u,
            VM_PROFILE_DEFAULT_PC_AT_NO_DMA_CHANNEL },
        { VM_PROFILE_DEFAULT_PC_AT_ROUTE_FDC_IRQ6_DMA2, 6u, 2u }
    };
    lib_size index;

    if (profile->route_count != sizeof(expected) / sizeof(expected[0])) return 1;
    for (index = 0u; index < profile->route_count; ++index) {
        const vm_profile_default_pc_at_route *route =
            vm_profile_default_pc_at_route_find(profile, expected[index].source);

        if (route == LIB_NULL || lib_memory_compare(route, &expected[index],
                sizeof(*route)) != 0) return 1;
    }
    return 0;
}

static C_INT pcat_topology_rejects_before_registration(
    const vm_profile_default_pc_at_descriptor *source)
{
    vm_profile_default_pc_at_descriptor invalid = *source;
    vm_profile_default_pc_at_port_leaf leaves[96];

    if (source->port_leaf_count > sizeof(leaves) / sizeof(leaves[0])) return 1;
    lib_memory_copy(leaves, source->port_leaves,
        source->port_leaf_count * sizeof(leaves[0]));
    leaves[0].read = LIB_FALSE;
    invalid.port_leaves = leaves;
    /* Profile now owns construction validation; Machine accepts only a
     * frozen plan and cannot be fed a second descriptor route. */
    return vm_profile_default_pc_at_descriptor_is_valid(&invalid) ? 1 : 0;
}

C_INT main(C_VOID)
{
    const vm_profile_default_pc_at_descriptor *profile =
        vm_profile_default_pc_at_descriptor_get();

    if (profile == LIB_NULL || !vm_profile_default_pc_at_descriptor_is_valid(profile) ||
        pcat_topology_registry_matches_profile(profile) != 0 ||
        pcat_topology_routes_are_explicit(profile) != 0 ||
        pcat_topology_rejects_before_registration(profile) != 0) {
        return 1;
    }
    STD_PRINTF("M5:T353:S2:PCAT-TOPOLOGY:OK\n");
    return 0;
}
