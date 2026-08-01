#ifndef NXVM_RUNTIME_SESSION_H
#define NXVM_RUNTIME_SESSION_H

#include "machine/core/status.h"
#include "runtime/profile.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct nxvm_runtime_session nxvm_runtime_session;

typedef struct nxvm_runtime_session_config {
    nxvm_runtime_profile_id profile;
    const char *fdd_image;
    const char *hdd_image;
    int boot_hdd;
} nxvm_runtime_session_config;

nxvm_core_status nxvm_runtime_session_create(
    const nxvm_runtime_session_config *config,
    nxvm_runtime_session **out_session);
nxvm_core_status nxvm_runtime_session_reset(nxvm_runtime_session *session);
const nxvm_runtime_profile_descriptor *nxvm_runtime_session_profile(
    const nxvm_runtime_session *session);
void nxvm_runtime_session_destroy(nxvm_runtime_session *session);

#ifdef __cplusplus
}
#endif

#endif
