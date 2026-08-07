#include "type.h"



#include "vm/platform/host_surface.h"

C_INT main(C_VOID)
{
    vm_platform_host_surface_context context;
    vm_platform_host_surface_lease lease;
    C_INT first_owner = 0;
    C_INT second_owner = 0;
    C_INT failed = 0;

    vm_platform_host_surface_context_initialize(&context,
        VM_PLATFORM_HOST_SURFACE_TERMINAL, &first_owner);
    vm_platform_host_surface_lease_initialize(&lease);

    failed |= context.kind != VM_PLATFORM_HOST_SURFACE_TERMINAL;
    failed |= context.native_handle != &first_owner;
    failed |= vm_platform_host_surface_lease_acquire(&lease, &first_owner) !=
        TYPE_STATUS_OK;
    failed |= !vm_platform_host_surface_lease_is_owned_by(&lease, &first_owner);
    failed |= vm_platform_host_surface_lease_acquire(&lease, &second_owner) !=
        TYPE_STATUS_UNSUPPORTED;
    failed |= vm_platform_host_surface_lease_release(&lease, &second_owner) !=
        TYPE_STATUS_INVALID_STATE;
    failed |= vm_platform_host_surface_lease_release(&lease, &first_owner) !=
        TYPE_STATUS_OK;
    failed |= vm_platform_host_surface_lease_acquire(&lease, &second_owner) !=
        TYPE_STATUS_OK;
    failed |= vm_platform_host_surface_lease_release(&lease, &second_owner) !=
        TYPE_STATUS_OK;

    if (failed) return 1;
    puts("M5:T89:S1:HOST-SURFACE-LEASE:OK");
    return 0;
}
