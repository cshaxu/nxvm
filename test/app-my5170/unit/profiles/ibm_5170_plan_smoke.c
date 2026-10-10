#include "app-my5170/profiles/profile_interface.h"
#include "core/board-base/machine_board_interface.h"
#include "lib/types/types_interface.h"

static lib_i32 ibm_5170_plan_is_complete(void)
{
    vm_profile_default_pc_at_plan_snapshot profile;
    core_machine_plan *plan = LIB_NULL;
    lib_status status;

    if (vm_profile_ibm_5170_plan_create(&profile) != LIB_STATUS_OK ||
        !vm_profile_default_pc_at_descriptor_is_valid(&profile.descriptor) ||
        profile.values.core.configuration.cpu_profile != CORE_MACHINE_CPU_PROFILE_80286 ||
        profile.values.core.configuration.memory_bytes != 512u * 1024u ||
        profile.values.core.configuration.pic_topology != CORE_MACHINE_PIC_TOPOLOGY_CASCADED ||
        profile.values.core.configuration.dma_controller_count !=
            CORE_MACHINE_DMA_CONTROLLER_COUNT ||
        profile.values.core.configuration.time_axis.kind !=
            CORE_MACHINE_TIME_AXIS_MACRO_PROPORTIONAL ||
        profile.values.core.configuration.time_axis.ticks_per_second != 8000000u ||
        profile.values.port_leaf_count == 0u || profile.values.irq_route_count != 4u ||
        profile.values.drq_route_count != 1u || !profile.topology.display_present ||
        !profile.topology.dma_present || !profile.topology.rtc_cmos_present ||
        profile.values.firmware_policy != VM_PROFILE_CONTRACT_FIRMWARE_POLICY_BUILTIN ||
        profile.values.media_policy != VM_PROFILE_CONTRACT_MEDIA_POLICY_SESSION) return 1;
    status = core_machine_plan_create(&profile.values.core.configuration, &plan);
    if (status == LIB_STATUS_OK) status = core_machine_plan_set_controller_timing_rules(plan,
        &profile.values.core.controller_timing_rules);
    if (status == LIB_STATUS_OK) status = core_machine_plan_set_topology(plan, &profile.topology);
    core_machine_plan_destroy(plan);
    return status != LIB_STATUS_OK;
}

static lib_i32 ibm_5170_memory_options_stay_bounded(void)
{
    vm_profile_default_pc_at_plan_snapshot expanded;
    vm_profile_default_pc_at_plan_snapshot rejected;

    if (vm_profile_ibm_5170_plan_create_memory(1536u * 1024u, &expanded) !=
        LIB_STATUS_OK || expanded.values.core.configuration.memory_bytes !=
            1536u * 1024u || expanded.descriptor.default_memory_bytes !=
            1536u * 1024u || expanded.descriptor.cmos.base_memory_kib != 0x0280u ||
        expanded.descriptor.unpopulated_extended_memory ||
        expanded.values.allowed_session_options != VM_PROFILE_5170_SESSION_OPTION_MEMORY) {
        return 1;
    }
    return vm_profile_ibm_5170_plan_create_memory(1024u * 1024u, &rejected) ==
        LIB_STATUS_OK || vm_profile_ibm_5170_plan_create_memory(4u * 1024u * 1024u,
            &rejected) == LIB_STATUS_OK;
}

lib_i32 main(void)
{
    return ibm_5170_plan_is_complete() || ibm_5170_memory_options_stay_bounded();
}
