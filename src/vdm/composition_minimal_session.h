#ifndef NTVDM64_VDM_MINIMAL_SESSION_H
#define NTVDM64_VDM_MINIMAL_SESSION_H

#include "type.h"
#include "core/product/runtime/profile_descriptor.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ntvdm64_vdm_minimal_session ntvdm64_vdm_minimal_session;

ntvdm64_status ntvdm64_vdm_minimal_session_create(
    ntvdm64_vdm_minimal_session **out_session);
ntvdm64_status ntvdm64_vdm_minimal_session_reset(
    ntvdm64_vdm_minimal_session *session);
const core_product_runtime_profile_descriptor *ntvdm64_vdm_minimal_session_profile(
    const ntvdm64_vdm_minimal_session *session);
void ntvdm64_vdm_minimal_session_destroy(ntvdm64_vdm_minimal_session *session);

#ifdef __cplusplus
}
#endif

#endif
