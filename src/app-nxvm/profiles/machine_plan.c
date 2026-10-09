#include "app-nxvm/profiles/construction_interface.h"
#include "app-nxvm/profiles/profile_interface.h"
#include "core/machine/pc_at_preparation_interface.h"
#include "core/machine/preparation_interface.h"

static lib_status vm_profile_prepare_default(const vm_machine_config *config,
    vm_profile_default_pc_at_plan_snapshot *profile,
    vm_machine_construction *construction)
{
    vm_profile_default_at_request request = {0};

    if (vm_machine_floppy_select(config, VM_PROFILE_FLOPPY_35_1440K,
            (1u << VM_PROFILE_FLOPPY_35_1440K) | (1u << VM_PROFILE_FLOPPY_35_720K) |
            (1u << VM_PROFILE_FLOPPY_525_1200K) | (1u << VM_PROFILE_FLOPPY_525_360K),
            &construction->media_kind) != LIB_STATUS_OK) return LIB_STATUS_INVALID_ARGUMENT;
    if (config->cpu_profile != CORE_MACHINE_CPU_PROFILE_DEFAULT ||
        config->fpu_profile != X86_FPU_PROFILE_NONE) {
        request.requested_options |= VM_PROFILE_DEFAULT_AT_SESSION_OPTION_CPU_FPU;
        request.cpu_profile = config->cpu_profile;
        request.fpu_profile = config->fpu_profile;
    }
    if (config->memory_bytes != 0u) {
        request.requested_options |= VM_PROFILE_DEFAULT_AT_SESSION_OPTION_MEMORY;
        request.memory_bytes = config->memory_bytes;
    }
    if (construction->media_kind != VM_PROFILE_FLOPPY_35_1440K) {
        request.requested_options |= VM_PROFILE_DEFAULT_AT_SESSION_OPTION_FLOPPY;
        request.floppy_cmos_type = vm_profile_floppy_cmos_type_get(construction->media_kind);
    }
    if (vm_profile_default_at_plan_create(&request, profile) != LIB_STATUS_OK) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    construction->core_config = profile->values.core.configuration;
    construction->timing_rules = profile->values.core.controller_timing_rules;
    construction->topology = profile->topology;
    construction->floppy_kind = construction->media_kind;
    construction->floppy_slot_count = 1u;
    construction->hdc_present = profile->descriptor.hdc_present;
    construction->memory_reconfigurable = LIB_TRUE;
    return LIB_STATUS_OK;
}

lib_status vm_profile_machine_plan_create_default(const vm_machine_config *config,
    const vm_machine_assets *assets, vm_machine_construction *out_construction)
{
    return vm_pc_at_construction_create(config, assets, vm_profile_prepare_default,
        out_construction);
}
