#include "type.h"

#include "core/platform/host_surface_interface.h"

C_VOID core_platform_host_surface_context_initialize(
    core_platform_host_surface_context *context,
    core_platform_host_surface_kind kind, C_VOID *native_handle)
{
    if (context == STD_NULL) return;
    context->kind = kind;
    context->native_handle = native_handle;
}

C_VOID core_platform_host_surface_lease_initialize(
    core_platform_host_surface_lease *lease)
{
    if (lease != STD_NULL) STD_ATOMIC_INIT(&lease->owner, (uintptr_t)0u);
}

ntvdm64_status core_platform_host_surface_lease_acquire(
    core_platform_host_surface_lease *lease, const C_VOID *owner)
{
    uintptr_t expected = (uintptr_t)0u;
    uintptr_t token = (uintptr_t)owner;

    if (lease == STD_NULL || owner == STD_NULL) return NTVDM64_STATUS_INVALID_ARGUMENT;
    if (STD_ATOMIC_COMPARE_EXCHANGE_STRONG(&lease->owner, &expected, token)) {
        return NTVDM64_STATUS_OK;
    }
    return expected == token ? NTVDM64_STATUS_INVALID_STATE :
        NTVDM64_STATUS_UNSUPPORTED;
}

ntvdm64_status core_platform_host_surface_lease_release(
    core_platform_host_surface_lease *lease, const C_VOID *owner)
{
    uintptr_t expected = (uintptr_t)owner;

    if (lease == STD_NULL || owner == STD_NULL) return NTVDM64_STATUS_INVALID_ARGUMENT;
    return STD_ATOMIC_COMPARE_EXCHANGE_STRONG(&lease->owner, &expected,
        (uintptr_t)0u) ? NTVDM64_STATUS_OK : NTVDM64_STATUS_INVALID_STATE;
}

C_INT core_platform_host_surface_lease_is_owned_by(
    const core_platform_host_surface_lease *lease, const C_VOID *owner)
{
    return lease != STD_NULL && owner != STD_NULL &&
        STD_ATOMIC_LOAD(&lease->owner) == (uintptr_t)owner;
}
