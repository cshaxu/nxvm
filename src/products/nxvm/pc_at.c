#include "products/nxvm/pc_at.h"

#include <string.h>

#include "adapters/nxvm_baseline/full_pc_profile.h"

nxvm_core_status nxvm_product_nxvm_pc_at_create(
    nxvm_product_nxvm_pc_at *pc_at,
    const nxvm_product_nxvm_media_policy *media)
{
    nxvm_baseline_full_pc_config config;
    const nxvm_product_nxvm_block_provider *fdd;
    const nxvm_product_nxvm_block_provider *hdd;
    nxvm_core_status status;

    if (pc_at == NULL || media == NULL || !media->frozen ||
        (media->boot_target != NXVM_PRODUCT_NXVM_BOOT_FDD &&
         media->boot_target != NXVM_PRODUCT_NXVM_BOOT_HDD)) {
        return NXVM_CORE_STATUS_INVALID_ARGUMENT;
    }
    memset(pc_at, 0, sizeof(*pc_at));
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
    status = nxvm_baseline_full_pc_create(&config);
    if (status != NXVM_CORE_STATUS_OK) return status;
    pc_at->media = media;
    pc_at->active = 1;
    return NXVM_CORE_STATUS_OK;
}

nxvm_core_status nxvm_product_nxvm_pc_at_get_reset_vector(
    const nxvm_product_nxvm_pc_at *pc_at,
    nxvm_product_nxvm_reset_vector *out_vector)
{
    nxvm_baseline_reset_vector vector;
    nxvm_core_status status;

    if (pc_at == NULL || out_vector == NULL || !pc_at->active) {
        return NXVM_CORE_STATUS_INVALID_STATE;
    }
    status = nxvm_baseline_full_pc_get_reset_vector(&vector);
    if (status != NXVM_CORE_STATUS_OK) return status;
    out_vector->cs = vector.cs;
    out_vector->ip = vector.ip;
    return NXVM_CORE_STATUS_OK;
}

nxvm_core_status nxvm_product_nxvm_pc_at_set_window_display(
    nxvm_product_nxvm_pc_at *pc_at, int enabled)
{
    if (pc_at == NULL || !pc_at->active) return NXVM_CORE_STATUS_INVALID_STATE;
    return nxvm_baseline_full_pc_set_window_display(enabled);
}

nxvm_core_status nxvm_product_nxvm_pc_at_set_memory_kb(
    nxvm_product_nxvm_pc_at *pc_at, uint32_t kilobytes)
{
    if (pc_at == NULL || !pc_at->active) return NXVM_CORE_STATUS_INVALID_STATE;
    return nxvm_baseline_full_pc_set_memory_kb(kilobytes);
}

nxvm_core_status nxvm_product_nxvm_pc_at_reset(nxvm_product_nxvm_pc_at *pc_at)
{
    if (pc_at == NULL || !pc_at->active) return NXVM_CORE_STATUS_INVALID_STATE;
    return nxvm_baseline_full_pc_reset();
}

void nxvm_product_nxvm_pc_at_run(nxvm_product_nxvm_pc_at *pc_at)
{
    if (pc_at != NULL && pc_at->active) nxvm_baseline_full_pc_run();
}

void nxvm_product_nxvm_pc_at_resume(nxvm_product_nxvm_pc_at *pc_at)
{
    if (pc_at != NULL && pc_at->active) nxvm_baseline_full_pc_resume();
}

nxvm_core_status nxvm_product_nxvm_pc_at_is_running(
    const nxvm_product_nxvm_pc_at *pc_at, int *out_running)
{
    if (pc_at == NULL || !pc_at->active) return NXVM_CORE_STATUS_INVALID_STATE;
    return nxvm_baseline_full_pc_is_running(out_running);
}

nxvm_core_status nxvm_product_nxvm_pc_at_debug(nxvm_product_nxvm_pc_at *pc_at)
{
    if (pc_at == NULL || !pc_at->active) return NXVM_CORE_STATUS_INVALID_STATE;
    return nxvm_baseline_full_pc_debug();
}

nxvm_core_status nxvm_product_nxvm_pc_at_remove_fdd(
    nxvm_product_nxvm_pc_at *pc_at, const char *path)
{
    if (pc_at == NULL || !pc_at->active) return NXVM_CORE_STATUS_INVALID_STATE;
    return nxvm_baseline_full_pc_remove_fdd(path);
}

nxvm_core_status nxvm_product_nxvm_pc_at_disconnect_hdd(
    nxvm_product_nxvm_pc_at *pc_at, const char *path)
{
    if (pc_at == NULL || !pc_at->active) return NXVM_CORE_STATUS_INVALID_STATE;
    return nxvm_baseline_full_pc_disconnect_hdd(path);
}

nxvm_core_status nxvm_product_nxvm_pc_at_record_start(
    nxvm_product_nxvm_pc_at *pc_at, const char *path)
{
    if (pc_at == NULL || !pc_at->active) return NXVM_CORE_STATUS_INVALID_STATE;
    return nxvm_baseline_full_pc_record_start(path);
}

void nxvm_product_nxvm_pc_at_record_stop(nxvm_product_nxvm_pc_at *pc_at)
{
    if (pc_at != NULL && pc_at->active) nxvm_baseline_full_pc_record_stop();
}

void nxvm_product_nxvm_pc_at_request_stop(nxvm_product_nxvm_pc_at *pc_at)
{
    if (pc_at != NULL && pc_at->active) nxvm_baseline_full_pc_request_stop();
}

void nxvm_product_nxvm_pc_at_destroy(nxvm_product_nxvm_pc_at *pc_at)
{
    if (pc_at != NULL && pc_at->active) {
        nxvm_baseline_full_pc_destroy();
        memset(pc_at, 0, sizeof(*pc_at));
    }
}
