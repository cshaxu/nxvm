#include "runtime/session.h"

#include <stdlib.h>

#include "adapters/nxvm_baseline/full_pc_profile.h"
#include "vdm/machine/dos_minimal.h"

struct nxvm_runtime_session {
    nxvm_runtime_session_config config;
    const nxvm_runtime_profile_descriptor *profile;
    nxvm_runtime_dos_minimal *dos_minimal;
    int full_pc_active;
};

static nxvm_core_status nxvm_runtime_session_create_full_pc(
    nxvm_runtime_session *session)
{
    const nxvm_baseline_full_pc_config config = {
        session->config.fdd_image,
        session->config.hdd_image,
        0,
        0u,
        session->config.boot_hdd
    };
    nxvm_core_status status = nxvm_baseline_full_pc_create(&config);

    if (status == NXVM_CORE_STATUS_OK) {
        session->full_pc_active = 1;
    }
    return status;
}

nxvm_core_status nxvm_runtime_session_create(
    const nxvm_runtime_session_config *config,
    nxvm_runtime_session **out_session)
{
    nxvm_runtime_session *session;
    nxvm_core_status status;

    if (config == NULL || out_session == NULL) {
        return NXVM_CORE_STATUS_INVALID_ARGUMENT;
    }
    *out_session = NULL;
    session = (nxvm_runtime_session *)calloc(1u, sizeof(*session));
    if (session == NULL) {
        return NXVM_CORE_STATUS_NO_MEMORY;
    }
    session->config = *config;
    session->profile = nxvm_runtime_profile_get(config->profile);
    if (session->profile == NULL) {
        free(session);
        return NXVM_CORE_STATUS_UNSUPPORTED;
    }
    if (config->profile == NXVM_RUNTIME_PROFILE_NXVM_FULL_PC) {
        status = nxvm_runtime_session_create_full_pc(session);
    } else {
        status = nxvm_runtime_dos_minimal_create(&session->dos_minimal);
    }
    if (status != NXVM_CORE_STATUS_OK) {
        nxvm_runtime_session_destroy(session);
        return status;
    }
    *out_session = session;
    return NXVM_CORE_STATUS_OK;
}

nxvm_core_status nxvm_runtime_session_reset(nxvm_runtime_session *session)
{
    if (session == NULL) {
        return NXVM_CORE_STATUS_INVALID_ARGUMENT;
    }
    if (session->config.profile == NXVM_RUNTIME_PROFILE_NXVM_FULL_PC) {
        if (session->full_pc_active) {
            nxvm_baseline_full_pc_destroy();
            session->full_pc_active = 0;
        }
        return nxvm_runtime_session_create_full_pc(session);
    }
    return nxvm_runtime_dos_minimal_reset(session->dos_minimal);
}

const nxvm_runtime_profile_descriptor *nxvm_runtime_session_profile(
    const nxvm_runtime_session *session)
{
    return session == NULL ? NULL : session->profile;
}

void nxvm_runtime_session_destroy(nxvm_runtime_session *session)
{
    if (session != NULL) {
        if (session->full_pc_active) {
            nxvm_baseline_full_pc_destroy();
        }
        nxvm_runtime_dos_minimal_destroy(session->dos_minimal);
        free(session);
    }
}
