#include "vdm/product/minimal_session.h"

#include <stdlib.h>

#include "vdm/machine/dos_minimal.h"
#include "vdm/profile/dos_minimal_profile.h"

struct ntvdm64_vdm_minimal_session {
    const nxvm_runtime_profile_descriptor *profile;
    nxvm_runtime_dos_minimal *dos_minimal;
};

nxvm_core_status ntvdm64_vdm_minimal_session_create(
    ntvdm64_vdm_minimal_session **out_session)
{
    ntvdm64_vdm_minimal_session *session;
    nxvm_core_status status;

    if (out_session == NULL) {
        return NXVM_CORE_STATUS_INVALID_ARGUMENT;
    }
    *out_session = NULL;
    session = (ntvdm64_vdm_minimal_session *)calloc(1u, sizeof(*session));
    if (session == NULL) {
        return NXVM_CORE_STATUS_NO_MEMORY;
    }
    session->profile = ntvdm64_dos_minimal_profile_descriptor();
    status = nxvm_runtime_dos_minimal_create(&session->dos_minimal);
    if (status != NXVM_CORE_STATUS_OK) {
        free(session);
        return status;
    }
    *out_session = session;
    return NXVM_CORE_STATUS_OK;
}

nxvm_core_status ntvdm64_vdm_minimal_session_reset(
    ntvdm64_vdm_minimal_session *session)
{
    if (session == NULL) {
        return NXVM_CORE_STATUS_INVALID_ARGUMENT;
    }
    return nxvm_runtime_dos_minimal_reset(session->dos_minimal);
}

const nxvm_runtime_profile_descriptor *ntvdm64_vdm_minimal_session_profile(
    const ntvdm64_vdm_minimal_session *session)
{
    return session == NULL ? NULL : session->profile;
}

void ntvdm64_vdm_minimal_session_destroy(ntvdm64_vdm_minimal_session *session)
{
    if (session != NULL) {
        nxvm_runtime_dos_minimal_destroy(session->dos_minimal);
        free(session);
    }
}
