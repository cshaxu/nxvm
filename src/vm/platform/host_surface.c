#include "type.h"

#include "vm/platform/platform_internal.h"

C_VOID vm_platform_host_surface_context_initialize(
    vm_platform_host_surface_context *context,
    vm_platform_host_surface_kind kind, C_VOID *native_handle)
{
    if (context == STD_NULL) return;
    context->kind = kind;
    context->native_handle = native_handle;
}

type_status vm_platform_host_surface_lease_create(
    vm_platform_host_surface_lease **out_lease)
{
    vm_platform_host_surface_lease *lease;

    if (out_lease == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    *out_lease = STD_CALLOC(1u, sizeof(*lease));
    lease = *out_lease;
    if (lease == STD_NULL) return TYPE_STATUS_NO_MEMORY;
    STD_ATOMIC_INIT(&lease->owner, (type_unsigned_pointer)0u);
    return TYPE_STATUS_OK;
}

C_VOID vm_platform_host_surface_lease_destroy(vm_platform_host_surface_lease *lease)
{
    STD_FREE(lease);
}

type_status vm_platform_host_surface_lease_acquire(
    vm_platform_host_surface_lease *lease, const C_VOID *owner)
{
    type_unsigned_pointer expected = (type_unsigned_pointer)0u;
    type_unsigned_pointer token = (type_unsigned_pointer)owner;

    if (lease == STD_NULL || owner == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    if (STD_ATOMIC_COMPARE_EXCHANGE_STRONG(&lease->owner, &expected, token)) {
        return TYPE_STATUS_OK;
    }
    return expected == token ? TYPE_STATUS_INVALID_STATE :
        TYPE_STATUS_UNSUPPORTED;
}

type_status vm_platform_host_surface_lease_release(
    vm_platform_host_surface_lease *lease, const C_VOID *owner)
{
    type_unsigned_pointer expected = (type_unsigned_pointer)owner;

    if (lease == STD_NULL || owner == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    return STD_ATOMIC_COMPARE_EXCHANGE_STRONG(&lease->owner, &expected,
        (type_unsigned_pointer)0u) ? TYPE_STATUS_OK : TYPE_STATUS_INVALID_STATE;
}

C_INT vm_platform_host_surface_lease_is_owned_by(
    const vm_platform_host_surface_lease *lease, const C_VOID *owner)
{
    return lease != STD_NULL && owner != STD_NULL &&
        STD_ATOMIC_LOAD(&lease->owner) == (type_unsigned_pointer)owner;
}
