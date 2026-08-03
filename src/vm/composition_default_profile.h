#ifndef NXVM_COMPOSITION_DEFAULT_PROFILE_H
#define NXVM_COMPOSITION_DEFAULT_PROFILE_H

#include <stdint.h>

#include "type.h"
#include "vm/product/media.h"

typedef struct vm_composition_full_pc vm_composition_full_pc;

typedef struct vm_composition_default_profile_reset_vector {
    uint16_t cs;
    uint16_t ip;
} vm_composition_default_profile_reset_vector;

typedef struct vm_composition_default_profile {
    const vm_product_media_policy *media;
    vm_composition_full_pc *full_pc;
    C_INT active;
} vm_composition_default_profile;

ntvdm64_status vm_composition_default_profile_create(
    vm_composition_default_profile *default_profile,
    const vm_product_media_policy *media);
ntvdm64_status vm_composition_default_profile_get_reset_vector(
    const vm_composition_default_profile *default_profile,
    vm_composition_default_profile_reset_vector *out_vector);
ntvdm64_status vm_composition_default_profile_set_window_display(
    vm_composition_default_profile *default_profile, C_INT enabled);
ntvdm64_status vm_composition_default_profile_set_memory_kb(
    vm_composition_default_profile *default_profile, uint32_t kilobytes);
ntvdm64_status vm_composition_default_profile_reset(vm_composition_default_profile *default_profile);
C_VOID vm_composition_default_profile_run(vm_composition_default_profile *default_profile);
C_VOID vm_composition_default_profile_resume(vm_composition_default_profile *default_profile);
ntvdm64_status vm_composition_default_profile_is_running(
    const vm_composition_default_profile *default_profile, C_INT *out_running);
ntvdm64_status vm_composition_default_profile_debug(vm_composition_default_profile *default_profile);
ntvdm64_status vm_composition_default_profile_remove_fdd(
    vm_composition_default_profile *default_profile, const C_CHAR *path);
ntvdm64_status vm_composition_default_profile_disconnect_hdd(
    vm_composition_default_profile *default_profile, const C_CHAR *path);
ntvdm64_status vm_composition_default_profile_record_start(
    vm_composition_default_profile *default_profile, const C_CHAR *path);
C_VOID vm_composition_default_profile_record_stop(vm_composition_default_profile *default_profile);
C_VOID vm_composition_default_profile_request_stop(vm_composition_default_profile *default_profile);
C_VOID vm_composition_default_profile_destroy(vm_composition_default_profile *default_profile);

#endif
