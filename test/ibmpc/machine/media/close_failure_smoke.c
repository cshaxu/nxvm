#include "lib/types/types_interface.h"

#include "ibmpc/machine/media/fdd.h"
#include "ibmpc/machine/media/hdd.h"

static lib_bool vm_media_close_failure_pending;

/* The injected result occurs only after Storage has consumed the lease. */
lib_status vm_media_close_failure_destroy(lib_storage_medium **medium)
{
    lib_status status = lib_storage_medium_destroy(medium);

    if (status != LIB_STATUS_OK || !vm_media_close_failure_pending) return status;
    vm_media_close_failure_pending = LIB_FALSE;
    return LIB_STATUS_IO_ERROR;
}

int main(void)
{
    static const core_machine_media_geometry fdd_geometry = {
        1u, 512u, 1u, 1u, 1u
    };
    static const lib_u8 fdd_bytes[512] = { 0xa5u };
    static const lib_u8 hdd_bytes[512] = { 0x5au };
    t_fdd fdd;
    t_hdd hdd;

    if (vm_machine_fdd_initialize_with_geometry(&fdd, &fdd_geometry) != LIB_STATUS_OK)
        return 1;
    vm_machine_hdd_initialize(&hdd);
    if (vm_machine_fdd_replace_bytes(&fdd, fdd_bytes, sizeof(fdd_bytes)) != LIB_STATUS_OK ||
        vm_machine_hdd_replace_bytes(&hdd, hdd_bytes, sizeof(hdd_bytes)) != LIB_STATUS_OK)
        return 1;

    vm_media_close_failure_pending = LIB_TRUE;
    if (vm_machine_fdd_remove_for(&fdd) != LIB_STATUS_IO_ERROR ||
        vm_machine_fdd_has_media(&fdd) || fdd.connect.medium != LIB_NULL)
        return 1;
    vm_media_close_failure_pending = LIB_TRUE;
    if (vm_machine_hdd_remove(&hdd) != LIB_STATUS_IO_ERROR ||
        vm_machine_hdd_has_media(&hdd) || hdd.connect.medium != LIB_NULL)
        return 1;
    vm_machine_fdd_finalize(&fdd);
    vm_machine_hdd_finalize(&hdd);
    return 0;
}
