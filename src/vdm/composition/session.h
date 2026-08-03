#ifndef NTVDM64_VDM_MINIMAL_SESSION_H
#define NTVDM64_VDM_MINIMAL_SESSION_H

#include "type.h"
#include "core/product/runtime/profile_descriptor.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct vdm_session vdm_session;

ntvdm64_status vdm_session_create(
    vdm_session **out_session);
ntvdm64_status vdm_session_reset(
    vdm_session *session);
const core_product_runtime_profile_descriptor *vdm_session_profile(
    const vdm_session *session);
C_VOID vdm_session_destroy(vdm_session *session);

#ifdef __cplusplus
}
#endif

#endif
