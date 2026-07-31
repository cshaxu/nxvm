#ifndef NXVM_PRODUCT_NXVM_PC_AT_H
#define NXVM_PRODUCT_NXVM_PC_AT_H

#include <stdint.h>

#include "core/status.h"
#include "products/nxvm/media.h"

typedef struct nxvm_product_nxvm_reset_vector {
    uint16_t cs;
    uint16_t ip;
} nxvm_product_nxvm_reset_vector;

typedef struct nxvm_product_nxvm_pc_at {
    const nxvm_product_nxvm_media_policy *media;
    int active;
} nxvm_product_nxvm_pc_at;

nxvm_core_status nxvm_product_nxvm_pc_at_create(
    nxvm_product_nxvm_pc_at *pc_at,
    const nxvm_product_nxvm_media_policy *media);
nxvm_core_status nxvm_product_nxvm_pc_at_get_reset_vector(
    const nxvm_product_nxvm_pc_at *pc_at,
    nxvm_product_nxvm_reset_vector *out_vector);
nxvm_core_status nxvm_product_nxvm_pc_at_set_window_display(
    nxvm_product_nxvm_pc_at *pc_at, int enabled);
nxvm_core_status nxvm_product_nxvm_pc_at_reset(nxvm_product_nxvm_pc_at *pc_at);
void nxvm_product_nxvm_pc_at_run(nxvm_product_nxvm_pc_at *pc_at);
void nxvm_product_nxvm_pc_at_resume(nxvm_product_nxvm_pc_at *pc_at);
nxvm_core_status nxvm_product_nxvm_pc_at_is_running(
    const nxvm_product_nxvm_pc_at *pc_at, int *out_running);
nxvm_core_status nxvm_product_nxvm_pc_at_debug(nxvm_product_nxvm_pc_at *pc_at);
nxvm_core_status nxvm_product_nxvm_pc_at_record_start(
    nxvm_product_nxvm_pc_at *pc_at, const char *path);
void nxvm_product_nxvm_pc_at_record_stop(nxvm_product_nxvm_pc_at *pc_at);
void nxvm_product_nxvm_pc_at_request_stop(nxvm_product_nxvm_pc_at *pc_at);
void nxvm_product_nxvm_pc_at_destroy(nxvm_product_nxvm_pc_at *pc_at);

#endif
