#ifndef NXVM_BASELINE_FULL_PC_PROFILE_H
#define NXVM_BASELINE_FULL_PC_PROFILE_H

#include <stdint.h>

#include "core/status.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct nxvm_baseline_full_pc_config {
    const char *fdd_image;
    const char *hdd_image;
    int boot_hdd;
} nxvm_baseline_full_pc_config;

typedef struct nxvm_baseline_reset_vector {
    uint16_t cs;
    uint16_t ip;
} nxvm_baseline_reset_vector;

nxvm_core_status nxvm_baseline_full_pc_create(
    const nxvm_baseline_full_pc_config *config);

nxvm_core_status nxvm_baseline_full_pc_get_reset_vector(
    nxvm_baseline_reset_vector *out_vector);

void nxvm_baseline_full_pc_run(void);
void nxvm_baseline_full_pc_request_stop(void);
void nxvm_baseline_full_pc_destroy(void);

#ifdef __cplusplus
}
#endif

#endif
