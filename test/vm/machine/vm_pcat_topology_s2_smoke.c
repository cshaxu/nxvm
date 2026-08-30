#include "type.h"

#include "core/machine/machine.h"
#include "core/machine/port.h"
#include "vm/composition/session/lifecycle.h"
#include "vm/composition/session/session_private.h"
#include "vm/composition/session/session_interface.h"
#include "vm/profile/default_profile/pc_at_profile.h"

static C_INT pcat_topology_registry_matches_profile(
    const vm_profile_default_pc_at_descriptor *profile)
{
    vm_session *session = STD_NULL;
    const vm_profile_default_pc_at_route *pit_route;
    const vm_profile_default_pc_at_route *keyboard_route;
    const vm_profile_default_pc_at_route *aux_route;
    const vm_profile_default_pc_at_route *cmos_route;
    const vm_profile_default_pc_at_route *fdc_route;
    STD_SIZE_T index;
    C_INT failed = 0;

    if (vm_session_create(STD_NULL, &session) != TYPE_STATUS_OK ||
        session == STD_NULL || session->core_machine == STD_NULL) {
        vm_session_destroy(session);
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
    failed |= pit_route == STD_NULL || keyboard_route == STD_NULL || aux_route == STD_NULL ||
        cmos_route == STD_NULL || fdc_route == STD_NULL ||
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
    vm_session_destroy(session);
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
    STD_SIZE_T index;

    if (profile->route_count != sizeof(expected) / sizeof(expected[0])) return 1;
    for (index = 0u; index < profile->route_count; ++index) {
        const vm_profile_default_pc_at_route *route =
            vm_profile_default_pc_at_route_find(profile, expected[index].source);

        if (route == STD_NULL || STD_MEMCMP(route, &expected[index],
                sizeof(*route)) != 0) return 1;
    }
    return 0;
}

static C_INT pcat_topology_rejects_before_registration(
    const vm_profile_default_pc_at_descriptor *source)
{
    vm_profile_default_pc_at_descriptor invalid = *source;
    vm_profile_default_pc_at_port_leaf leaves[96];
    vm_session session = {0};

    if (source->port_leaf_count > sizeof(leaves) / sizeof(leaves[0])) return 1;
    STD_MEMCPY(leaves, source->port_leaves,
        source->port_leaf_count * sizeof(leaves[0]));
    leaves[0].read = TYPE_FALSE;
    invalid.port_leaves = leaves;
    session.profile = &invalid;
    session.core_machine_config.memory_bytes = invalid.default_memory_bytes;
    session.core_machine_config.cpu_profile = invalid.cpu_profile;
    session.core_machine_config.fpu_profile = invalid.fpu_profile;
    session.core_machine_config.ticks_per_instruction = invalid.ticks_per_instruction;
    session.core_machine_config.instruction_timing = invalid.instruction_timing;
    session.core_machine_config.clock_plan = invalid.clock_plan;
    session.core_machine_config.kbc_typematic_initial_ticks =
        invalid.kbc_typematic_initial_ticks;
    session.core_machine_config.kbc_typematic_repeat_ticks =
        invalid.kbc_typematic_repeat_ticks;
    session.core_machine_config.kbc_command_response_ticks =
        invalid.kbc_command_response_ticks;
    if (vm_session_initialize(&session) != TYPE_STATUS_INVALID_ARGUMENT ||
        session.active || session.core_machine != STD_NULL) {
        vm_session_finalize(&session);
        return 1;
    }
    vm_session_finalize(&session);
    return 0;
}

C_INT main(C_VOID)
{
    const vm_profile_default_pc_at_descriptor *profile =
        vm_profile_default_pc_at_descriptor_get();

    if (profile == STD_NULL || !vm_profile_default_pc_at_descriptor_is_valid(profile) ||
        pcat_topology_registry_matches_profile(profile) != 0 ||
        pcat_topology_routes_are_explicit(profile) != 0 ||
        pcat_topology_rejects_before_registration(profile) != 0) {
        return 1;
    }
    STD_PRINTF("M5:T353:S2:PCAT-TOPOLOGY:OK\n");
    return 0;
}
