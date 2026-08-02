#include "vm/composition_session.h"

#include <stdlib.h>

#include "vm/composition_full_pc.h"
#include "vm/profile/full_pc_profile.h"

struct vm_composition_full_pc_session {
    vm_composition_full_pc_session_config config;
    const nxvm_runtime_profile_descriptor *profile;
    int active;
};

static nxvm_core_status vm_composition_full_pc_session_start(
    vm_composition_full_pc_session *session)
{
    const nxvm_full_pc_config config = {
        session->config.fdd_image,
        session->config.hdd_image,
        0,
        0u,
        session->config.boot_hdd
    };
    nxvm_core_status status = nxvm_full_pc_create(&config);

    if (status == NXVM_CORE_STATUS_OK) {
        session->active = 1;
    }
    return status;
}

nxvm_core_status vm_composition_full_pc_session_create(
    const vm_composition_full_pc_session_config *config,
    vm_composition_full_pc_session **out_session)
{
    vm_composition_full_pc_session *session;
    nxvm_core_status status;

    if (config == NULL || out_session == NULL) {
        return NXVM_CORE_STATUS_INVALID_ARGUMENT;
    }
    *out_session = NULL;
    session = (vm_composition_full_pc_session *)calloc(1u, sizeof(*session));
    if (session == NULL) {
        return NXVM_CORE_STATUS_NO_MEMORY;
    }
    session->config = *config;
    session->profile = nxvm_vm_full_pc_profile_descriptor();
    status = vm_composition_full_pc_session_start(session);
    if (status != NXVM_CORE_STATUS_OK) {
        vm_composition_full_pc_session_destroy(session);
        return status;
    }
    *out_session = session;
    return NXVM_CORE_STATUS_OK;
}

nxvm_core_status vm_composition_full_pc_session_reset(vm_composition_full_pc_session *session)
{
    if (session == NULL) {
        return NXVM_CORE_STATUS_INVALID_ARGUMENT;
    }
    if (session->active) {
        nxvm_full_pc_destroy();
        session->active = 0;
    }
    return vm_composition_full_pc_session_start(session);
}

const nxvm_runtime_profile_descriptor *vm_composition_full_pc_session_profile(
    const vm_composition_full_pc_session *session)
{
    return session == NULL ? NULL : session->profile;
}

void vm_composition_full_pc_session_destroy(vm_composition_full_pc_session *session)
{
    if (session != NULL) {
        if (session->active) {
            nxvm_full_pc_destroy();
        }
        free(session);
    }
}
