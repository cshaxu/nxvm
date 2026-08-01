#ifndef NXVM_COMPOSITION_DEFAULT_PROFILE_H
#define NXVM_COMPOSITION_DEFAULT_PROFILE_H

#include <stdint.h>

#include "core/machine/status.h"
#include "vm/product/media.h"

typedef struct nxvm_product_nxvm_reset_vector {
    uint16_t cs;
    uint16_t ip;
} nxvm_product_nxvm_reset_vector;

typedef struct nxvm_product_nxvm_default_profile {
    const nxvm_product_nxvm_media_policy *media;
    int active;
} nxvm_product_nxvm_default_profile;

nxvm_core_status nxvm_product_nxvm_default_profile_create(
    nxvm_product_nxvm_default_profile *default_profile,
    const nxvm_product_nxvm_media_policy *media);
nxvm_core_status nxvm_product_nxvm_default_profile_get_reset_vector(
    const nxvm_product_nxvm_default_profile *default_profile,
    nxvm_product_nxvm_reset_vector *out_vector);
nxvm_core_status nxvm_product_nxvm_default_profile_set_window_display(
    nxvm_product_nxvm_default_profile *default_profile, int enabled);
nxvm_core_status nxvm_product_nxvm_default_profile_set_memory_kb(
    nxvm_product_nxvm_default_profile *default_profile, uint32_t kilobytes);
nxvm_core_status nxvm_product_nxvm_default_profile_reset(nxvm_product_nxvm_default_profile *default_profile);
void nxvm_product_nxvm_default_profile_run(nxvm_product_nxvm_default_profile *default_profile);
void nxvm_product_nxvm_default_profile_resume(nxvm_product_nxvm_default_profile *default_profile);
nxvm_core_status nxvm_product_nxvm_default_profile_is_running(
    const nxvm_product_nxvm_default_profile *default_profile, int *out_running);
nxvm_core_status nxvm_product_nxvm_default_profile_debug(nxvm_product_nxvm_default_profile *default_profile);
nxvm_core_status nxvm_product_nxvm_default_profile_remove_fdd(
    nxvm_product_nxvm_default_profile *default_profile, const char *path);
nxvm_core_status nxvm_product_nxvm_default_profile_disconnect_hdd(
    nxvm_product_nxvm_default_profile *default_profile, const char *path);
nxvm_core_status nxvm_product_nxvm_default_profile_record_start(
    nxvm_product_nxvm_default_profile *default_profile, const char *path);
void nxvm_product_nxvm_default_profile_record_stop(nxvm_product_nxvm_default_profile *default_profile);
void nxvm_product_nxvm_default_profile_request_stop(nxvm_product_nxvm_default_profile *default_profile);
void nxvm_product_nxvm_default_profile_destroy(nxvm_product_nxvm_default_profile *default_profile);

#endif
