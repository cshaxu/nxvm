#ifndef NXVM_COMPOSITION_SESSION_H
#define NXVM_COMPOSITION_SESSION_H

#include "type.h"
#include "core/product/runtime/profile_descriptor.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct nxvm_vm_full_pc_session nxvm_vm_full_pc_session;

typedef struct nxvm_vm_full_pc_session_config {
    const char *fdd_image;
    const char *hdd_image;
    int boot_hdd;
} nxvm_vm_full_pc_session_config;

nxvm_core_status nxvm_vm_full_pc_session_create(
    const nxvm_vm_full_pc_session_config *config,
    nxvm_vm_full_pc_session **out_session);
nxvm_core_status nxvm_vm_full_pc_session_reset(nxvm_vm_full_pc_session *session);
const nxvm_runtime_profile_descriptor *nxvm_vm_full_pc_session_profile(
    const nxvm_vm_full_pc_session *session);
void nxvm_vm_full_pc_session_destroy(nxvm_vm_full_pc_session *session);

#ifdef __cplusplus
}
#endif

#endif
