#include "type.h"



#include "vm/platform/host_surface.h"

C_INT main(C_VOID)
{
    vm_platform_host_surface_lease *lease = STD_NULL;
    C_INT first_owner = 0;
    C_INT second_owner = 0;
    C_INT failed = 0;

    if (vm_platform_host_surface_lease_create(&lease) != TYPE_STATUS_OK) return 1;
    failed |= vm_platform_host_surface_lease_acquire(lease, &first_owner) !=
        TYPE_STATUS_OK;
    failed |= !vm_platform_host_surface_lease_is_owned_by(lease, &first_owner);
    failed |= vm_platform_host_surface_lease_acquire(lease, &second_owner) !=
        TYPE_STATUS_UNSUPPORTED;
    failed |= vm_platform_host_surface_lease_release(lease, &second_owner) !=
        TYPE_STATUS_INVALID_STATE;
    failed |= vm_platform_host_surface_lease_release(lease, &first_owner) !=
        TYPE_STATUS_OK;
    failed |= vm_platform_host_surface_lease_acquire(lease, &second_owner) !=
        TYPE_STATUS_OK;
    failed |= vm_platform_host_surface_lease_release(lease, &second_owner) !=
        TYPE_STATUS_OK;

    vm_platform_host_surface_lease_destroy(lease);
    if (failed) return 1;
    puts("M5:T89:S1:HOST-SURFACE-LEASE:OK");
    return 0;
}
