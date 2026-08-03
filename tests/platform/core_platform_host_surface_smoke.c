#include "type.h"

#include <stdio.h>


#include "core/platform/host_surface_interface.h"

C_INT main(C_VOID)
{
    core_platform_host_surface_context context;
    core_platform_host_surface_lease lease;
    C_INT first_owner = 0;
    C_INT second_owner = 0;
    C_INT failed = 0;

    core_platform_host_surface_context_initialize(&context,
        CORE_PLATFORM_HOST_SURFACE_TERMINAL, &first_owner);
    core_platform_host_surface_lease_initialize(&lease);

    failed |= context.kind != CORE_PLATFORM_HOST_SURFACE_TERMINAL;
    failed |= context.native_handle != &first_owner;
    failed |= core_platform_host_surface_lease_acquire(&lease, &first_owner) !=
        NTVDM64_STATUS_OK;
    failed |= !core_platform_host_surface_lease_is_owned_by(&lease, &first_owner);
    failed |= core_platform_host_surface_lease_acquire(&lease, &second_owner) !=
        NTVDM64_STATUS_UNSUPPORTED;
    failed |= core_platform_host_surface_lease_release(&lease, &second_owner) !=
        NTVDM64_STATUS_INVALID_STATE;
    failed |= core_platform_host_surface_lease_release(&lease, &first_owner) !=
        NTVDM64_STATUS_OK;
    failed |= core_platform_host_surface_lease_acquire(&lease, &second_owner) !=
        NTVDM64_STATUS_OK;
    failed |= core_platform_host_surface_lease_release(&lease, &second_owner) !=
        NTVDM64_STATUS_OK;

    if (failed) return 1;
    puts("M5:T89:S1:HOST-SURFACE-LEASE:OK");
    return 0;
}
