#ifndef NXVM_COMPOSITION_FULL_PC_H
#define NXVM_COMPOSITION_FULL_PC_H

#include <stdint.h>

#include "type.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct nxvm_full_pc_config {
    const char *fdd_image;
    const char *hdd_image;
    int create_fdd;
    uint16_t create_hdd_cylinders;
    int boot_hdd;
} nxvm_full_pc_config;

typedef struct nxvm_vm_reset_vector {
    uint16_t cs;
    uint16_t ip;
} nxvm_vm_reset_vector;

typedef struct nxvm_full_pc nxvm_full_pc;

ntvdm64_status nxvm_full_pc_create(
    const nxvm_full_pc_config *config,
    nxvm_full_pc **out_full_pc);

ntvdm64_status nxvm_full_pc_get_reset_vector(
    const nxvm_full_pc *full_pc,
    nxvm_vm_reset_vector *out_vector);

ntvdm64_status nxvm_full_pc_set_window_display(nxvm_full_pc *full_pc, int enabled);
ntvdm64_status nxvm_full_pc_set_memory_kb(nxvm_full_pc *full_pc, uint32_t kilobytes);
ntvdm64_status nxvm_full_pc_reset(nxvm_full_pc *full_pc);
void nxvm_full_pc_run(nxvm_full_pc *full_pc);
void nxvm_full_pc_resume(nxvm_full_pc *full_pc);
ntvdm64_status nxvm_full_pc_is_running(const nxvm_full_pc *full_pc, int *out_running);
ntvdm64_status nxvm_full_pc_debug(nxvm_full_pc *full_pc);
ntvdm64_status nxvm_full_pc_remove_fdd(nxvm_full_pc *full_pc, const char *path);
ntvdm64_status nxvm_full_pc_disconnect_hdd(nxvm_full_pc *full_pc, const char *path);
ntvdm64_status nxvm_full_pc_record_start(nxvm_full_pc *full_pc, const char *path);
void nxvm_full_pc_record_stop(nxvm_full_pc *full_pc);
void nxvm_full_pc_request_stop(nxvm_full_pc *full_pc);
void nxvm_full_pc_destroy(nxvm_full_pc *full_pc);

#ifdef __cplusplus
}
#endif

#endif
