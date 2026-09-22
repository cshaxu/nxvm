#include "type.h"

#include "core/devices/machine_interface.h"
#include "core/profiles/default_profile/pc_at_profile_private.h"
#include "core/profiles/model40/model40_private.h"

static C_INT vm_ibm_5170_direct_plan_is_complete(C_VOID)
{
    vm_profile_default_pc_at_plan_snapshot profile;
    core_machine_plan *plan = STD_NULL;
    type_status status;

    if (vm_profile_ibm_5170_plan_create(&profile) != TYPE_STATUS_OK ||
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
    if (status == TYPE_STATUS_OK) status = core_machine_plan_set_controller_timing_rules(plan,
        &profile.values.core.controller_timing_rules);
    if (status == TYPE_STATUS_OK) status = core_machine_plan_set_topology(plan, &profile.topology);
    core_machine_plan_destroy(plan);
    return status != TYPE_STATUS_OK;
}

static C_INT vm_ibm_5170_memory_options_stay_bounded(C_VOID)
{
    vm_profile_default_pc_at_plan_snapshot expanded;
    vm_profile_default_pc_at_plan_snapshot rejected;

    if (vm_profile_ibm_5170_plan_create_memory(1536u * 1024u, &expanded) !=
        TYPE_STATUS_OK || expanded.values.core.configuration.memory_bytes !=
            1536u * 1024u || expanded.descriptor.default_memory_bytes !=
            1536u * 1024u || expanded.descriptor.cmos.base_memory_kib != 0x0280u ||
        expanded.descriptor.unpopulated_extended_memory ||
        expanded.values.allowed_session_options != VM_PROFILE_DEFAULT_AT_SESSION_OPTION_MEMORY) {
        return 1;
    }
    return vm_profile_ibm_5170_plan_create_memory(1024u * 1024u, &rejected) ==
        TYPE_STATUS_OK || vm_profile_ibm_5170_plan_create_memory(4u * 1024u * 1024u,
            &rejected) == TYPE_STATUS_OK;
}

static C_INT vm_model40_direct_plan_is_complete(C_VOID)
{
    vm_profile_contract_values values;

    return vm_profile_model40_values_create(&values) != TYPE_STATUS_OK ||
        values.core.configuration.memory_bytes != 2u * 1024u * 1024u ||
        values.core.configuration.cpu_profile != CORE_MACHINE_CPU_PROFILE_80386 ||
        values.core.configuration.pic_topology != CORE_MACHINE_PIC_TOPOLOGY_CASCADED ||
        values.core.configuration.dma_controller_count != CORE_MACHINE_DMA_CONTROLLER_COUNT ||
        values.firmware_policy != VM_PROFILE_CONTRACT_FIRMWARE_POLICY_BYOB ||
        values.media_policy != VM_PROFILE_CONTRACT_MEDIA_POLICY_SESSION;
}

static C_INT vm_default_at_direct_plan_is_complete(C_VOID)
{
    const vm_profile_default_at_request request = {
        VM_PROFILE_DEFAULT_AT_SESSION_OPTION_CPU_FPU |
        VM_PROFILE_DEFAULT_AT_SESSION_OPTION_MEMORY |
        VM_PROFILE_DEFAULT_AT_SESSION_OPTION_FLOPPY,
        CORE_MACHINE_CPU_PROFILE_80386, CORE_MACHINE_FPU_PROFILE_NONE,
        32u * 1024u * 1024u, 0x40u};
    vm_profile_default_pc_at_plan_snapshot profile;

    return vm_profile_default_at_plan_create(&request, &profile) != TYPE_STATUS_OK ||
        profile.values.core.configuration.cpu_profile != CORE_MACHINE_CPU_PROFILE_80386 ||
        profile.values.core.configuration.memory_bytes != 32u * 1024u * 1024u ||
        profile.values.allowed_session_options !=
            (VM_PROFILE_DEFAULT_AT_SESSION_OPTION_CPU_FPU |
             VM_PROFILE_DEFAULT_AT_SESSION_OPTION_MEMORY |
             VM_PROFILE_DEFAULT_AT_SESSION_OPTION_FLOPPY) ||
        profile.descriptor.cmos.floppy_type != 0x40u;
}

C_INT main(C_VOID)
{
    return vm_ibm_5170_direct_plan_is_complete() ||
        vm_ibm_5170_memory_options_stay_bounded() ||
        vm_model40_direct_plan_is_complete() || vm_default_at_direct_plan_is_complete();
}
