#ifndef NXVM_COMPOSITION_FULL_PC_H
#define NXVM_COMPOSITION_FULL_PC_H

#include <stdint.h>

#include "type.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct vm_composition_full_pc_config {
    const char *fdd_image;
    const char *hdd_image;
    int create_fdd;
    uint16_t create_hdd_cylinders;
    int boot_hdd;
} vm_composition_full_pc_config;

typedef struct vm_composition_reset_vector {
    uint16_t cs;
    uint16_t ip;
} vm_composition_reset_vector;

typedef struct vm_composition_full_pc vm_composition_full_pc;

ntvdm64_status vm_composition_full_pc_create(
    const vm_composition_full_pc_config *config,
    vm_composition_full_pc **out_full_pc);

ntvdm64_status vm_composition_full_pc_get_reset_vector(
    const vm_composition_full_pc *full_pc,
    vm_composition_reset_vector *out_vector);

ntvdm64_status vm_composition_full_pc_set_window_display(vm_composition_full_pc *full_pc, int enabled);
ntvdm64_status vm_composition_full_pc_set_memory_kb(vm_composition_full_pc *full_pc, uint32_t kilobytes);
ntvdm64_status vm_composition_full_pc_reset(vm_composition_full_pc *full_pc);
void vm_composition_full_pc_run(vm_composition_full_pc *full_pc);
void vm_composition_full_pc_resume(vm_composition_full_pc *full_pc);
ntvdm64_status vm_composition_full_pc_is_running(const vm_composition_full_pc *full_pc, int *out_running);
ntvdm64_status vm_composition_full_pc_debug(vm_composition_full_pc *full_pc);
ntvdm64_status vm_composition_full_pc_remove_fdd(vm_composition_full_pc *full_pc, const char *path);
ntvdm64_status vm_composition_full_pc_disconnect_hdd(vm_composition_full_pc *full_pc, const char *path);
ntvdm64_status vm_composition_full_pc_record_start(vm_composition_full_pc *full_pc, const char *path);
void vm_composition_full_pc_record_stop(vm_composition_full_pc *full_pc);
void vm_composition_full_pc_request_stop(vm_composition_full_pc *full_pc);
void vm_composition_full_pc_destroy(vm_composition_full_pc *full_pc);

#ifdef __cplusplus
}
#endif

#endif
