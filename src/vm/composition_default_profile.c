#include "vm/composition_default_profile.h"

#include <string.h>

#include "vm/composition_full_pc.h"

nxvm_core_status nxvm_product_nxvm_default_profile_create(
    nxvm_product_nxvm_default_profile *default_profile,
    const nxvm_product_nxvm_media_policy *media)
{
    nxvm_full_pc_config config;
    const nxvm_product_nxvm_block_provider *fdd;
    const nxvm_product_nxvm_block_provider *hdd;
    nxvm_core_status status;

    if (default_profile == NULL || media == NULL || !media->frozen ||
        (media->boot_target != NXVM_PRODUCT_NXVM_BOOT_FDD &&
         media->boot_target != NXVM_PRODUCT_NXVM_BOOT_HDD)) {
        return NXVM_CORE_STATUS_INVALID_ARGUMENT;
    }
    memset(default_profile, 0, sizeof(*default_profile));
    fdd = nxvm_product_nxvm_media_provider(media, NXVM_PRODUCT_NXVM_BOOT_FDD);
    hdd = nxvm_product_nxvm_media_provider(media, NXVM_PRODUCT_NXVM_BOOT_HDD);
    config.fdd_image = fdd != NULL && fdd->configured ? fdd->path : NULL;
    config.hdd_image = hdd != NULL && hdd->configured ? hdd->path : NULL;
    config.create_fdd = fdd != NULL && fdd->configured && fdd->created;
    config.create_hdd_cylinders = hdd != NULL && hdd->configured && hdd->created ?
        hdd->cylinders : 0u;
    if (config.create_fdd) config.fdd_image = NULL;
    if (config.create_hdd_cylinders != 0u) config.hdd_image = NULL;
    config.boot_hdd = media->boot_target == NXVM_PRODUCT_NXVM_BOOT_HDD;
    status = nxvm_full_pc_create(&config, &default_profile->full_pc);
    if (status != NXVM_CORE_STATUS_OK) return status;
    default_profile->media = media;
    default_profile->active = 1;
    return NXVM_CORE_STATUS_OK;
}

nxvm_core_status nxvm_product_nxvm_default_profile_get_reset_vector(
    const nxvm_product_nxvm_default_profile *default_profile,
    nxvm_product_nxvm_reset_vector *out_vector)
{
    nxvm_vm_reset_vector vector;
    nxvm_core_status status;

    if (default_profile == NULL || out_vector == NULL || !default_profile->active) {
        return NXVM_CORE_STATUS_INVALID_STATE;
    }
    status = nxvm_full_pc_get_reset_vector(default_profile->full_pc, &vector);
    if (status != NXVM_CORE_STATUS_OK) return status;
    out_vector->cs = vector.cs;
    out_vector->ip = vector.ip;
    return NXVM_CORE_STATUS_OK;
}

nxvm_core_status nxvm_product_nxvm_default_profile_set_window_display(
    nxvm_product_nxvm_default_profile *default_profile, int enabled)
{
    if (default_profile == NULL || !default_profile->active) return NXVM_CORE_STATUS_INVALID_STATE;
    return nxvm_full_pc_set_window_display(default_profile->full_pc, enabled);
}

nxvm_core_status nxvm_product_nxvm_default_profile_set_memory_kb(
    nxvm_product_nxvm_default_profile *default_profile, uint32_t kilobytes)
{
    if (default_profile == NULL || !default_profile->active) return NXVM_CORE_STATUS_INVALID_STATE;
    return nxvm_full_pc_set_memory_kb(default_profile->full_pc, kilobytes);
}

nxvm_core_status nxvm_product_nxvm_default_profile_reset(nxvm_product_nxvm_default_profile *default_profile)
{
    if (default_profile == NULL || !default_profile->active) return NXVM_CORE_STATUS_INVALID_STATE;
    return nxvm_full_pc_reset(default_profile->full_pc);
}

void nxvm_product_nxvm_default_profile_run(nxvm_product_nxvm_default_profile *default_profile)
{
    if (default_profile != NULL && default_profile->active) nxvm_full_pc_run(default_profile->full_pc);
}

void nxvm_product_nxvm_default_profile_resume(nxvm_product_nxvm_default_profile *default_profile)
{
    if (default_profile != NULL && default_profile->active) nxvm_full_pc_resume(default_profile->full_pc);
}

nxvm_core_status nxvm_product_nxvm_default_profile_is_running(
    const nxvm_product_nxvm_default_profile *default_profile, int *out_running)
{
    if (default_profile == NULL || !default_profile->active) return NXVM_CORE_STATUS_INVALID_STATE;
    return nxvm_full_pc_is_running(default_profile->full_pc, out_running);
}

nxvm_core_status nxvm_product_nxvm_default_profile_debug(nxvm_product_nxvm_default_profile *default_profile)
{
    if (default_profile == NULL || !default_profile->active) return NXVM_CORE_STATUS_INVALID_STATE;
    return nxvm_full_pc_debug(default_profile->full_pc);
}

nxvm_core_status nxvm_product_nxvm_default_profile_remove_fdd(
    nxvm_product_nxvm_default_profile *default_profile, const char *path)
{
    if (default_profile == NULL || !default_profile->active) return NXVM_CORE_STATUS_INVALID_STATE;
    return nxvm_full_pc_remove_fdd(default_profile->full_pc, path);
}

nxvm_core_status nxvm_product_nxvm_default_profile_disconnect_hdd(
    nxvm_product_nxvm_default_profile *default_profile, const char *path)
{
    if (default_profile == NULL || !default_profile->active) return NXVM_CORE_STATUS_INVALID_STATE;
    return nxvm_full_pc_disconnect_hdd(default_profile->full_pc, path);
}

nxvm_core_status nxvm_product_nxvm_default_profile_record_start(
    nxvm_product_nxvm_default_profile *default_profile, const char *path)
{
    if (default_profile == NULL || !default_profile->active) return NXVM_CORE_STATUS_INVALID_STATE;
    return nxvm_full_pc_record_start(default_profile->full_pc, path);
}

void nxvm_product_nxvm_default_profile_record_stop(nxvm_product_nxvm_default_profile *default_profile)
{
    if (default_profile != NULL && default_profile->active) nxvm_full_pc_record_stop(default_profile->full_pc);
}

void nxvm_product_nxvm_default_profile_request_stop(nxvm_product_nxvm_default_profile *default_profile)
{
    if (default_profile != NULL && default_profile->active) nxvm_full_pc_request_stop(default_profile->full_pc);
}

void nxvm_product_nxvm_default_profile_destroy(nxvm_product_nxvm_default_profile *default_profile)
{
    if (default_profile != NULL && default_profile->active) {
        nxvm_full_pc_destroy(default_profile->full_pc);
        memset(default_profile, 0, sizeof(*default_profile));
    }
}
