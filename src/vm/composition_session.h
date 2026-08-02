#ifndef NXVM_COMPOSITION_SESSION_H
#define NXVM_COMPOSITION_SESSION_H

#include "type.h"
#include "core/product/runtime/profile_descriptor.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct vm_composition_full_pc_session vm_composition_full_pc_session;

typedef struct vm_composition_full_pc_session_config {
    const char *fdd_image;
    const char *hdd_image;
    int boot_hdd;
} vm_composition_full_pc_session_config;

nxvm_core_status vm_composition_full_pc_session_create(
    const vm_composition_full_pc_session_config *config,
    vm_composition_full_pc_session **out_session);
nxvm_core_status vm_composition_full_pc_session_reset(vm_composition_full_pc_session *session);
const nxvm_runtime_profile_descriptor *vm_composition_full_pc_session_profile(
    const vm_composition_full_pc_session *session);
void vm_composition_full_pc_session_destroy(vm_composition_full_pc_session *session);

#ifdef __cplusplus
}
#endif

#endif
