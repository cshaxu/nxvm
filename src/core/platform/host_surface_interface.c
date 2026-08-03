#include "core/platform/host_surface_interface.h"

void core_platform_host_surface_context_initialize(
    core_platform_host_surface_context *context,
    core_platform_host_surface_kind kind, void *native_handle)
{
    if (context == NULL) return;
    context->kind = kind;
    context->native_handle = native_handle;
}

void core_platform_host_surface_lease_initialize(
    core_platform_host_surface_lease *lease)
{
    if (lease != NULL) atomic_init(&lease->owner, (uintptr_t)0u);
}

nxvm_core_status core_platform_host_surface_lease_acquire(
    core_platform_host_surface_lease *lease, const void *owner)
{
    uintptr_t expected = (uintptr_t)0u;
    uintptr_t token = (uintptr_t)owner;

    if (lease == NULL || owner == NULL) return NXVM_CORE_STATUS_INVALID_ARGUMENT;
    if (atomic_compare_exchange_strong(&lease->owner, &expected, token)) {
        return NXVM_CORE_STATUS_OK;
    }
    return expected == token ? NXVM_CORE_STATUS_INVALID_STATE :
        NXVM_CORE_STATUS_UNSUPPORTED;
}

nxvm_core_status core_platform_host_surface_lease_release(
    core_platform_host_surface_lease *lease, const void *owner)
{
    uintptr_t expected = (uintptr_t)owner;

    if (lease == NULL || owner == NULL) return NXVM_CORE_STATUS_INVALID_ARGUMENT;
    return atomic_compare_exchange_strong(&lease->owner, &expected,
        (uintptr_t)0u) ? NXVM_CORE_STATUS_OK : NXVM_CORE_STATUS_INVALID_STATE;
}

int core_platform_host_surface_lease_is_owned_by(
    const core_platform_host_surface_lease *lease, const void *owner)
{
    return lease != NULL && owner != NULL &&
        atomic_load(&lease->owner) == (uintptr_t)owner;
}
