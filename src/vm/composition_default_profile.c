#include "type.h"

#include "vm/composition_default_profile.h"


#include <string.h>


#include "vm/composition_full_pc.h"

ntvdm64_status vm_composition_default_profile_create(
    vm_composition_default_profile *default_profile,
    const vm_product_media_policy *media)
{
    vm_composition_full_pc_config config;
    const vm_product_block_provider *fdd;
    const vm_product_block_provider *hdd;
    ntvdm64_status status;

    if (default_profile == STD_NULL || media == STD_NULL || !media->frozen ||
        (media->boot_target != VM_PRODUCT_BOOT_FDD &&
         media->boot_target != VM_PRODUCT_BOOT_HDD)) {
        return NTVDM64_STATUS_INVALID_ARGUMENT;
    }
    STD_MEMSET(default_profile, 0, sizeof(*default_profile));
    fdd = vm_product_media_provider(media, VM_PRODUCT_BOOT_FDD);
    hdd = vm_product_media_provider(media, VM_PRODUCT_BOOT_HDD);
    config.fdd_image = fdd != STD_NULL && fdd->configured ? fdd->path : STD_NULL;
    config.hdd_image = hdd != STD_NULL && hdd->configured ? hdd->path : STD_NULL;
    config.create_fdd = fdd != STD_NULL && fdd->configured && fdd->created;
    config.create_hdd_cylinders = hdd != STD_NULL && hdd->configured && hdd->created ?
        hdd->cylinders : 0u;
    if (config.create_fdd) config.fdd_image = STD_NULL;
    if (config.create_hdd_cylinders != 0u) config.hdd_image = STD_NULL;
    config.boot_hdd = media->boot_target == VM_PRODUCT_BOOT_HDD;
    status = vm_composition_full_pc_create(&config, &default_profile->full_pc);
    if (status != NTVDM64_STATUS_OK) return status;
    default_profile->media = media;
    default_profile->active = 1;
    return NTVDM64_STATUS_OK;
}

ntvdm64_status vm_composition_default_profile_get_reset_vector(
    const vm_composition_default_profile *default_profile,
    vm_composition_default_profile_reset_vector *out_vector)
{
    vm_composition_reset_vector vector;
    ntvdm64_status status;

    if (default_profile == STD_NULL || out_vector == STD_NULL || !default_profile->active) {
        return NTVDM64_STATUS_INVALID_STATE;
    }
    status = vm_composition_full_pc_get_reset_vector(default_profile->full_pc, &vector);
    if (status != NTVDM64_STATUS_OK) return status;
    out_vector->cs = vector.cs;
    out_vector->ip = vector.ip;
    return NTVDM64_STATUS_OK;
}

ntvdm64_status vm_composition_default_profile_set_window_display(
    vm_composition_default_profile *default_profile, C_INT enabled)
{
    if (default_profile == STD_NULL || !default_profile->active) return NTVDM64_STATUS_INVALID_STATE;
    return vm_composition_full_pc_set_window_display(default_profile->full_pc, enabled);
}

ntvdm64_status vm_composition_default_profile_set_memory_kb(
    vm_composition_default_profile *default_profile, uint32_t kilobytes)
{
    if (default_profile == STD_NULL || !default_profile->active) return NTVDM64_STATUS_INVALID_STATE;
    return vm_composition_full_pc_set_memory_kb(default_profile->full_pc, kilobytes);
}

ntvdm64_status vm_composition_default_profile_reset(vm_composition_default_profile *default_profile)
{
    if (default_profile == STD_NULL || !default_profile->active) return NTVDM64_STATUS_INVALID_STATE;
    return vm_composition_full_pc_reset(default_profile->full_pc);
}

C_VOID vm_composition_default_profile_run(vm_composition_default_profile *default_profile)
{
    if (default_profile != STD_NULL && default_profile->active) vm_composition_full_pc_run(default_profile->full_pc);
}

C_VOID vm_composition_default_profile_resume(vm_composition_default_profile *default_profile)
{
    if (default_profile != STD_NULL && default_profile->active) vm_composition_full_pc_resume(default_profile->full_pc);
}

ntvdm64_status vm_composition_default_profile_is_running(
    const vm_composition_default_profile *default_profile, C_INT *out_running)
{
    if (default_profile == STD_NULL || !default_profile->active) return NTVDM64_STATUS_INVALID_STATE;
    return vm_composition_full_pc_is_running(default_profile->full_pc, out_running);
}

ntvdm64_status vm_composition_default_profile_debug(vm_composition_default_profile *default_profile)
{
    if (default_profile == STD_NULL || !default_profile->active) return NTVDM64_STATUS_INVALID_STATE;
    return vm_composition_full_pc_debug(default_profile->full_pc);
}

ntvdm64_status vm_composition_default_profile_remove_fdd(
    vm_composition_default_profile *default_profile, const C_CHAR *path)
{
    if (default_profile == STD_NULL || !default_profile->active) return NTVDM64_STATUS_INVALID_STATE;
    return vm_composition_full_pc_remove_fdd(default_profile->full_pc, path);
}

ntvdm64_status vm_composition_default_profile_disconnect_hdd(
    vm_composition_default_profile *default_profile, const C_CHAR *path)
{
    if (default_profile == STD_NULL || !default_profile->active) return NTVDM64_STATUS_INVALID_STATE;
    return vm_composition_full_pc_disconnect_hdd(default_profile->full_pc, path);
}

ntvdm64_status vm_composition_default_profile_record_start(
    vm_composition_default_profile *default_profile, const C_CHAR *path)
{
    if (default_profile == STD_NULL || !default_profile->active) return NTVDM64_STATUS_INVALID_STATE;
    return vm_composition_full_pc_record_start(default_profile->full_pc, path);
}

C_VOID vm_composition_default_profile_record_stop(vm_composition_default_profile *default_profile)
{
    if (default_profile != STD_NULL && default_profile->active) vm_composition_full_pc_record_stop(default_profile->full_pc);
}

C_VOID vm_composition_default_profile_request_stop(vm_composition_default_profile *default_profile)
{
    if (default_profile != STD_NULL && default_profile->active) vm_composition_full_pc_request_stop(default_profile->full_pc);
}

C_VOID vm_composition_default_profile_destroy(vm_composition_default_profile *default_profile)
{
    if (default_profile != STD_NULL && default_profile->active) {
        vm_composition_full_pc_destroy(default_profile->full_pc);
        STD_MEMSET(default_profile, 0, sizeof(*default_profile));
    }
}
