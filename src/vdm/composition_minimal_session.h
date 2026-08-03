#ifndef NTVDM64_VDM_MINIMAL_SESSION_H
#define NTVDM64_VDM_MINIMAL_SESSION_H

#include "type.h"
#include "core/product/runtime/profile_descriptor.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct vdm_composition_minimal_session vdm_composition_minimal_session;

ntvdm64_status vdm_composition_minimal_session_create(
    vdm_composition_minimal_session **out_session);
ntvdm64_status vdm_composition_minimal_session_reset(
    vdm_composition_minimal_session *session);
const core_product_runtime_profile_descriptor *vdm_composition_minimal_session_profile(
    const vdm_composition_minimal_session *session);
void vdm_composition_minimal_session_destroy(vdm_composition_minimal_session *session);

#ifdef __cplusplus
}
#endif

#endif
