#include "x86-nxvm/machine/pc_at_preparation_interface.h"
#include "x86-nxvm/machine/preparation_interface.h"
#include "x86-nxvm/board-base/at_assembly_interface.h"
#include "x86-nxvm/board-base/pc_at_rom_interface.h"

#define X86_NXVM_BOARD_MEDIA_FDD_MEDIA_ID 1u
#define X86_NXVM_BOARD_MEDIA_HDD_MEDIA_ID 2u

typedef struct x86_nxvm_machine_pc_at_machine_plan {
    x86_nxvm_machine_construction construction;
    x86_nxvm_board_pc_at_plan_snapshot profile;
    struct {
        lib_u8 image[X86_NXVM_BOARD_EXTERNAL_PC_AT_ROM_BYTES];
        lib_u8 video[X86_NXVM_BOARD_EXTERNAL_PC_AT_VIDEO_ROM_MAX_BYTES];
        lib_size video_bytes;
        x86_nxvm_board_external_pc_at_rom_context context;
    } firmware;
} x86_nxvm_machine_pc_at_machine_plan;

static lib_status x86_nxvm_machine_plan_pc_at_rom(x86_nxvm_machine_pc_at_machine_plan *plan,
    const x86_nxvm_machine_config *config, const x86_nxvm_machine_assets *assets)
{
    if (plan == LIB_NULL || config == LIB_NULL || assets == LIB_NULL ||
        config->bios_count == 0u || config->bios_count > 2u) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    if (config->bios_count == 1u) {
        if (x86_nxvm_machine_asset_copy(plan->firmware.image,
                X86_NXVM_BOARD_EXTERNAL_PC_AT_ROM_BYTES, assets->bios[0u]) != LIB_STATUS_OK) {
            return LIB_STATUS_INVALID_ARGUMENT;
        }
    } else {
        if (x86_nxvm_board_rom_interleave(plan->firmware.image, sizeof(plan->firmware.image),
            assets->bios[0u].data, assets->bios[0u].bytes,
            assets->bios[1u].data, assets->bios[1u].bytes) != LIB_STATUS_OK) {
            return LIB_STATUS_INVALID_ARGUMENT;
        }
    }
    if (assets->video.data != LIB_NULL) {
        if (assets->video.bytes == 0u || assets->video.bytes >
            X86_NXVM_BOARD_EXTERNAL_PC_AT_VIDEO_ROM_MAX_BYTES) return LIB_STATUS_INVALID_ARGUMENT;
        if (x86_nxvm_machine_asset_copy(plan->firmware.video, assets->video.bytes,
            assets->video) != LIB_STATUS_OK) return LIB_STATUS_INVALID_ARGUMENT;
        plan->firmware.video_bytes = assets->video.bytes;
    } else if (assets->video.bytes != 0u) return LIB_STATUS_INVALID_ARGUMENT;
    plan->firmware.context = (x86_nxvm_board_external_pc_at_rom_context) {
        plan->firmware.image,
        plan->firmware.video_bytes == 0u ? LIB_NULL : plan->firmware.video,
        plan->firmware.video_bytes};
    plan->construction.firmware_provider = x86_nxvm_board_external_pc_at_rom_provider();
    plan->construction.firmware_context = &plan->firmware.context;
    return LIB_STATUS_OK;
}

static lib_status x86_nxvm_machine_plan_materialize_pc_at(
    const x86_nxvm_machine_pc_at_machine_plan *plan, x86_nxvm_board_plan *core_plan)
{
    const x86_nxvm_board_pc_at_descriptor *profile;
    x86_nxvm_board_fdc_drive_bindings drives = {
        {X86_NXVM_BOARD_MEDIA_FDD_MEDIA_ID, X86_NXVM_BOARD_MEDIA_ID_INVALID,
            X86_NXVM_BOARD_MEDIA_ID_INVALID, X86_NXVM_BOARD_MEDIA_ID_INVALID}, 0x01u, 0x01u,
        {0u, 0u, 0u, 0u}, 0u, {0}
    };
    x86_nxvm_board_fdc_config fdc = {0};

    if (plan == LIB_NULL || core_plan == LIB_NULL) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    profile = &plan->profile.descriptor;
    if (!x86_nxvm_board_pc_at_descriptor_is_valid(profile)) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    if (x86_nxvm_board_at_fdc_materialize(profile->port_leaves, profile->port_leaf_count,
            profile->routes, profile->route_count, &fdc) != LIB_STATUS_OK)
        return LIB_STATUS_INVALID_ARGUMENT;
    fdc.ready_mask = profile->fdc_ready_mask;
    fdc.clock_ticks_per_second = plan->construction.core_config.time_axis.ticks_per_second;
    drives.installed_mask = profile->fdc_installed_mask;
    drives.double_sided_mask = profile->fdc_double_sided_mask;
    lib_memory_copy(drives.cylinder_count, profile->fdc_cylinder_count,
        sizeof(drives.cylinder_count));
    drives.track_zero_active_low_mask = profile->fdc_track_zero_active_low_mask;
    for (x86_nxvm_board_floppy_kind kind = X86_NXVM_BOARD_FLOPPY_35_1440K;
            kind <= X86_NXVM_BOARD_FLOPPY_35_720K; ++kind) {
        if (x86_nxvm_board_floppy_cmos_type_get(kind) == profile->cmos.floppy_type) {
            drives.channel = x86_nxvm_board_floppy_channel_get(kind);
            break;
        }
    }
    if (drives.channel.sample == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    fdc.diagnostic_port = profile->fdc_diagnostic_port;
    fdc.diagnostic_read_value = profile->fdc_diagnostic_read_value;
    if (x86_nxvm_board_plan_configure_fdc(core_plan, &drives, &fdc) != LIB_STATUS_OK) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    if (!profile->hdc_present) return LIB_STATUS_OK;
    return x86_nxvm_board_plan_configure_hdc(core_plan, X86_NXVM_BOARD_MEDIA_HDD_MEDIA_ID,
        X86_NXVM_BOARD_MEDIA_ID_INVALID, &profile->hdc);
}

static lib_status x86_nxvm_machine_pc_at_configure(void *context, x86_nxvm_board_plan *plan)
{
    return x86_nxvm_machine_plan_materialize_pc_at(context, plan);
}

static void x86_nxvm_machine_pc_at_release(void *context)
{
    lib_release(context);
}

lib_status x86_nxvm_machine_pc_at_construction_create(const x86_nxvm_machine_config *config,
    const x86_nxvm_machine_assets *assets, x86_nxvm_machine_pc_at_profile_prepare prepare,
    x86_nxvm_machine_construction *out_construction)
{
    x86_nxvm_machine_pc_at_machine_plan *plan;
    lib_status status = x86_nxvm_machine_construction_begin(config, assets, out_construction);

    if (status != LIB_STATUS_OK) return status;
    if (prepare == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    plan = (x86_nxvm_machine_pc_at_machine_plan *)lib_allocate_zero(1u, sizeof(*plan));
    if (plan == LIB_NULL) return LIB_STATUS_NO_MEMORY;
    plan->construction.profile = (x86_nxvm_machine_profile_binding) {
        .context = plan,
        .configure = x86_nxvm_machine_pc_at_configure,
        .release = x86_nxvm_machine_pc_at_release};
    status = prepare(config, &plan->profile, &plan->construction);
    if (status == LIB_STATUS_OK)
        status = x86_nxvm_machine_plan_pc_at_rom(plan, config, assets);
    return x86_nxvm_machine_construction_finish(&plan->construction, config, assets, status, out_construction);
}
