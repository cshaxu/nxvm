#include "vdm/composition_minimal_session.h"

#include <stdlib.h>

#include "vdm/machine/dos_minimal.h"
#include "vdm/profile/dos_minimal_profile.h"

struct vdm_composition_minimal_session {
    const core_product_runtime_profile_descriptor *profile;
    core_product_runtime_dos_minimal *dos_minimal;
};

ntvdm64_status vdm_composition_minimal_session_create(
    vdm_composition_minimal_session **out_session)
{
    vdm_composition_minimal_session *session;
    ntvdm64_status status;

    if (out_session == NULL) {
        return NTVDM64_STATUS_INVALID_ARGUMENT;
    }
    *out_session = NULL;
    session = (vdm_composition_minimal_session *)calloc(1u, sizeof(*session));
    if (session == NULL) {
        return NTVDM64_STATUS_NO_MEMORY;
    }
    session->profile = ntvdm64_dos_minimal_profile_descriptor();
    status = core_product_runtime_dos_minimal_create(&session->dos_minimal);
    if (status != NTVDM64_STATUS_OK) {
        free(session);
        return status;
    }
    *out_session = session;
    return NTVDM64_STATUS_OK;
}

ntvdm64_status vdm_composition_minimal_session_reset(
    vdm_composition_minimal_session *session)
{
    if (session == NULL) {
        return NTVDM64_STATUS_INVALID_ARGUMENT;
    }
    return core_product_runtime_dos_minimal_reset(session->dos_minimal);
}

const core_product_runtime_profile_descriptor *vdm_composition_minimal_session_profile(
    const vdm_composition_minimal_session *session)
{
    return session == NULL ? NULL : session->profile;
}

void vdm_composition_minimal_session_destroy(vdm_composition_minimal_session *session)
{
    if (session != NULL) {
        core_product_runtime_dos_minimal_destroy(session->dos_minimal);
        free(session);
    }
}
