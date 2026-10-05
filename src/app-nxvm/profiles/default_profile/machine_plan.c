#include "ibmpc/machine/preparation_interface.h"
#include "ibmpc/board-common/at_assembly_interface.h"
#include "app-nxvm/profiles/default_profile/construction_interface.h"
#include "app-nxvm/profiles/default_profile/external_pc_at_rom.h"
#include "app-nxvm/profiles/default_profile/pc_at_profile_private.h"

#define VM_PROFILE_MACHINE_FDD_MEDIA_ID 1u
#define VM_PROFILE_MACHINE_HDD_MEDIA_ID 2u

typedef struct vm_profile_pc_at_machine_plan {
    vm_machine_construction construction;
    vm_profile_default_pc_at_plan_snapshot profile;
    struct {
        lib_u8 image[VM_PROFILE_EXTERNAL_PC_AT_ROM_BYTES];
        lib_u8 video[VM_PROFILE_EXTERNAL_PC_AT_VIDEO_ROM_MAX_BYTES];
        lib_size video_bytes;
        vm_profile_external_pc_at_rom_context context;
    } firmware;
} vm_profile_pc_at_machine_plan;

static lib_status vm_profile_machine_plan_pc_at_rom(vm_profile_pc_at_machine_plan *plan,
    const vm_machine_config *config, const vm_machine_assets *assets)
{
    if (plan == LIB_NULL || config == LIB_NULL || assets == LIB_NULL ||
        config->bios_count == 0u || config->bios_count > 2u) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    if (config->bios_count == 1u) {
        if (vm_machine_asset_copy(plan->firmware.image,
                VM_PROFILE_EXTERNAL_PC_AT_ROM_BYTES, assets->bios[0u]) != LIB_STATUS_OK) {
            return LIB_STATUS_INVALID_ARGUMENT;
        }
    } else {
        if (vm_profile_rom_interleave(plan->firmware.image, sizeof(plan->firmware.image),
            assets->bios[0u].data, assets->bios[0u].bytes,
            assets->bios[1u].data, assets->bios[1u].bytes) != LIB_STATUS_OK) {
            return LIB_STATUS_INVALID_ARGUMENT;
        }
    }
    if (assets->video.data != LIB_NULL) {
        if (assets->video.bytes == 0u || assets->video.bytes >
            VM_PROFILE_EXTERNAL_PC_AT_VIDEO_ROM_MAX_BYTES) return LIB_STATUS_INVALID_ARGUMENT;
        if (vm_machine_asset_copy(plan->firmware.video, assets->video.bytes,
            assets->video) != LIB_STATUS_OK) return LIB_STATUS_INVALID_ARGUMENT;
        plan->firmware.video_bytes = assets->video.bytes;
    } else if (assets->video.bytes != 0u) return LIB_STATUS_INVALID_ARGUMENT;
    plan->firmware.context = (vm_profile_external_pc_at_rom_context) {
        plan->firmware.image,
        plan->firmware.video_bytes == 0u ? LIB_NULL : plan->firmware.video,
        plan->firmware.video_bytes};
    plan->construction.firmware_provider = vm_profile_external_pc_at_rom_provider();
    plan->construction.firmware_context = &plan->firmware.context;
    return LIB_STATUS_OK;
}

#if !defined(VM_PROFILE_BUILD_5170)
static lib_status vm_profile_machine_plan_default(vm_profile_pc_at_machine_plan *plan,
    const vm_machine_config *config, const vm_machine_assets *assets)
{
    vm_profile_default_at_request request = {0};

    if (vm_machine_floppy_select(config, VM_PROFILE_FLOPPY_35_1440K,
            (1u << VM_PROFILE_FLOPPY_35_1440K) | (1u << VM_PROFILE_FLOPPY_35_720K) |
            (1u << VM_PROFILE_FLOPPY_525_1200K) | (1u << VM_PROFILE_FLOPPY_525_360K),
            &plan->construction.media_kind) != LIB_STATUS_OK) return LIB_STATUS_INVALID_ARGUMENT;
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
    if (plan->construction.media_kind != VM_PROFILE_FLOPPY_35_1440K) {
        request.requested_options |= VM_PROFILE_DEFAULT_AT_SESSION_OPTION_FLOPPY;
        request.floppy_cmos_type = vm_profile_floppy_cmos_type_get(plan->construction.media_kind);
    }
    if (vm_profile_default_at_plan_create(&request, &plan->profile) != LIB_STATUS_OK ||
        vm_profile_machine_plan_pc_at_rom(plan, config, assets) != LIB_STATUS_OK) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    plan->construction.core_config = plan->profile.values.core.configuration;
    plan->construction.timing_rules = plan->profile.values.core.controller_timing_rules;
    plan->construction.topology = plan->profile.topology;
    plan->construction.floppy_kind = plan->construction.media_kind;
    plan->construction.floppy_slot_count = 1u;
    plan->construction.hdc_present = plan->profile.descriptor.hdc_present;
    plan->construction.memory_reconfigurable = LIB_TRUE;
    return LIB_STATUS_OK;
}

#endif
#if !defined(VM_PROFILE_BUILD_DEFAULT)
static lib_status vm_profile_machine_plan_5170(vm_profile_pc_at_machine_plan *plan,
    const vm_machine_config *config, const vm_machine_assets *assets)
{
    if (vm_machine_floppy_select(config, VM_PROFILE_FLOPPY_525_1200K,
            (1u << VM_PROFILE_FLOPPY_525_1200K) | (1u << VM_PROFILE_FLOPPY_525_360K),
            &plan->construction.media_kind) != LIB_STATUS_OK ||
        vm_profile_ibm_5170_plan_create_memory(config->memory_bytes,
            &plan->profile) != LIB_STATUS_OK ||
        vm_profile_machine_plan_pc_at_rom(plan, config, assets) != LIB_STATUS_OK) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    plan->construction.core_config = plan->profile.values.core.configuration;
    plan->construction.timing_rules = plan->profile.values.core.controller_timing_rules;
    plan->construction.topology = plan->profile.topology;
    plan->construction.floppy_kind = VM_PROFILE_FLOPPY_525_1200K;
    plan->construction.floppy_slot_count = 1u;
    plan->construction.hdc_present = plan->profile.descriptor.hdc_present;
    plan->construction.memory_reconfigurable = LIB_TRUE;
    return LIB_STATUS_OK;
}

#endif
static lib_status vm_profile_machine_plan_materialize_pc_at(
    const vm_profile_pc_at_machine_plan *plan, core_machine_plan *core_plan)
{
    const vm_profile_default_pc_at_descriptor *profile;
    core_machine_fdc_drive_bindings drives = {
        {VM_PROFILE_MACHINE_FDD_MEDIA_ID, CORE_MACHINE_MEDIA_ID_INVALID,
            CORE_MACHINE_MEDIA_ID_INVALID, CORE_MACHINE_MEDIA_ID_INVALID}, 0x01u, 0x01u,
        {0u, 0u, 0u, 0u}, 0u, {0}
    };
    core_machine_fdc_config fdc = {0};

    if (plan == LIB_NULL || core_plan == LIB_NULL) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    profile = &plan->profile.descriptor;
    if (!vm_profile_default_pc_at_descriptor_is_valid(profile)) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    if (vm_at_fdc_materialize(profile->port_leaves, profile->port_leaf_count,
            profile->routes, profile->route_count, &fdc) != LIB_STATUS_OK)
        return LIB_STATUS_INVALID_ARGUMENT;
    fdc.ready_mask = profile->fdc_ready_mask;
    fdc.clock_ticks_per_second = plan->construction.core_config.time_axis.ticks_per_second;
    drives.installed_mask = profile->fdc_installed_mask;
    drives.double_sided_mask = profile->fdc_double_sided_mask;
    lib_memory_copy(drives.cylinder_count, profile->fdc_cylinder_count,
        sizeof(drives.cylinder_count));
    drives.track_zero_active_low_mask = profile->fdc_track_zero_active_low_mask;
    for (vm_profile_floppy_kind kind = VM_PROFILE_FLOPPY_35_1440K;
            kind <= VM_PROFILE_FLOPPY_35_720K; ++kind) {
        if (vm_profile_floppy_cmos_type_get(kind) == profile->cmos.floppy_type) {
            drives.channel = vm_profile_floppy_channel_get(kind);
            break;
        }
    }
    if (drives.channel.sample == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    fdc.diagnostic_port = profile->fdc_diagnostic_port;
    fdc.diagnostic_read_value = profile->fdc_diagnostic_read_value;
    if (core_machine_plan_configure_fdc(core_plan, &drives, &fdc) != LIB_STATUS_OK) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    if (!profile->hdc_present) return LIB_STATUS_OK;
    return core_machine_plan_configure_hdc(core_plan, VM_PROFILE_MACHINE_HDD_MEDIA_ID,
        CORE_MACHINE_MEDIA_ID_INVALID, &profile->hdc);
}

static lib_status vm_profile_pc_at_configure(void *context, core_machine_plan *plan)
{
    return vm_profile_machine_plan_materialize_pc_at(context, plan);
}

static void vm_profile_pc_at_release(void *context)
{
    lib_release(context);
}

#if !defined(VM_PROFILE_BUILD_5170)
lib_status vm_profile_machine_plan_create_default(const vm_machine_config *config,
    const vm_machine_assets *assets, vm_machine_construction *out_construction)
{
    vm_profile_pc_at_machine_plan *plan;
    lib_status status = vm_machine_construction_begin(config, assets, out_construction);

    if (status != LIB_STATUS_OK) return status;
    plan = (vm_profile_pc_at_machine_plan *)lib_allocate_zero(1u, sizeof(*plan));
    if (plan == LIB_NULL) return LIB_STATUS_NO_MEMORY;
    plan->construction.profile = (vm_machine_profile_binding) {
        plan, vm_profile_pc_at_configure, LIB_NULL, vm_profile_pc_at_release };
    status = vm_profile_machine_plan_default(plan, config, assets);
    return vm_machine_construction_finish(&plan->construction, config, assets, status, out_construction);
}

#endif
#if !defined(VM_PROFILE_BUILD_DEFAULT)
lib_status vm_profile_machine_plan_create_5170(const vm_machine_config *config,
    const vm_machine_assets *assets, vm_machine_construction *out_construction)
{
    vm_profile_pc_at_machine_plan *plan;
    lib_status status = vm_machine_construction_begin(config, assets, out_construction);

    if (status != LIB_STATUS_OK) return status;
    plan = (vm_profile_pc_at_machine_plan *)lib_allocate_zero(1u, sizeof(*plan));
    if (plan == LIB_NULL) return LIB_STATUS_NO_MEMORY;
    plan->construction.profile = (vm_machine_profile_binding) {
        plan, vm_profile_pc_at_configure, LIB_NULL, vm_profile_pc_at_release };
    status = vm_profile_machine_plan_5170(plan, config, assets);
    return vm_machine_construction_finish(&plan->construction, config, assets, status, out_construction);
}
#endif
