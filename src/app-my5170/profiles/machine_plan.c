#include "app-my5170/profiles/construction_interface.h"
#include "app-my5170/profiles/profile_interface.h"
#include "core/machine/pc_at_preparation_interface.h"
#include "core/machine/preparation_interface.h"

static lib_status vm_profile_prepare_5170(const vm_machine_config *config,
    vm_profile_default_pc_at_plan_snapshot *profile,
    vm_machine_construction *construction)
{
    if (vm_machine_floppy_select(config, VM_PROFILE_FLOPPY_525_1200K,
            (1u << VM_PROFILE_FLOPPY_525_1200K) | (1u << VM_PROFILE_FLOPPY_525_360K),
            &construction->media_kind) != LIB_STATUS_OK ||
        vm_profile_ibm_5170_plan_create_memory(config->memory_bytes,
            profile) != LIB_STATUS_OK) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    construction->core_config = profile->values.core.configuration;
    construction->timing_rules = profile->values.core.controller_timing_rules;
    construction->topology = profile->topology;
    construction->floppy_kind = VM_PROFILE_FLOPPY_525_1200K;
    construction->floppy_slot_count = 1u;
    construction->hdc_present = profile->descriptor.hdc_present;
    construction->memory_reconfigurable = LIB_TRUE;
    return LIB_STATUS_OK;
}

lib_status vm_profile_machine_plan_create_5170(const vm_machine_config *config,
    const vm_machine_assets *assets, vm_machine_construction *out_construction)
{
    return vm_pc_at_construction_create(config, assets, vm_profile_prepare_5170,
        out_construction);
}
