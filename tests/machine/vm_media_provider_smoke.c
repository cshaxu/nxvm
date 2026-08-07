#include "type.h"

#include "core/machine/media_interface.h"
#include "vm/machine/fdd.h"
#include "vm/machine/hdd.h"

C_INT main(C_VOID)
{
    core_machine_media_registry registry;
    core_machine_media_info info;
    core_machine_media_result result;
    t_fdd fdd;
    t_hdd hdd;
    uint8_t bytes[512] = {0};
    uint64_t fdd_generation;
    C_INT failed = 0;

    vm_machine_fdd_initialize(&fdd);
    vm_machine_hdd_initialize(&hdd);
    vm_machine_fdd_create_for(&fdd);
    vm_machine_hdd_create(&hdd, 2u);
    core_machine_media_registry_initialize(&registry);
    if (core_machine_media_registry_bind(&registry, 1u, &fdd,
            vm_machine_fdd_media_provider()) != TYPE_STATUS_OK ||
        core_machine_media_registry_bind(&registry, 2u, &hdd,
            vm_machine_hdd_media_provider()) != TYPE_STATUS_OK ||
        core_machine_media_registry_freeze(&registry) != TYPE_STATUS_OK ||
        core_machine_media_query(&registry, 1u, &info, &result) != TYPE_STATUS_OK ||
        result != CORE_MACHINE_MEDIA_RESULT_OK || !info.present ||
        info.geometry.bytes_per_sector != 512u ||
        core_machine_media_query(&registry, 2u, &info, &result) != TYPE_STATUS_OK ||
        result != CORE_MACHINE_MEDIA_RESULT_OK || info.geometry.cylinders != 2u) {
        failed = 1;
    }
    fdd_generation = fdd.connect.media_generation;
    if (!failed && (core_machine_media_write_sectors(&registry, 1u, 0u, 1u,
            bytes, &result) != TYPE_STATUS_OK || result != CORE_MACHINE_MEDIA_RESULT_OK ||
        core_machine_media_format_sectors(&registry, 1u, 0u, 1u, 0xa5u,
            &result) != TYPE_STATUS_OK || result != CORE_MACHINE_MEDIA_RESULT_OK ||
        fdd.connect.media_generation != fdd_generation + 1u ||
        core_machine_media_read_sectors(&registry, 1u, 0u, 1u, bytes,
            &result) != TYPE_STATUS_OK || result != CORE_MACHINE_MEDIA_RESULT_OK ||
        bytes[0] != 0xa5u ||
        core_machine_media_read_sectors(&registry, 2u,
            info.geometry.logical_sector_count, 1u, bytes,
            &result) != TYPE_STATUS_OK || result != CORE_MACHINE_MEDIA_RESULT_INVALID_RANGE)) {
        failed = 1;
    }
    core_machine_media_registry_finalize(&registry);
    vm_machine_fdd_finalize(&fdd);
    vm_machine_hdd_finalize(&hdd);
    if (failed) return 1;
    STD_PRINTF("M5:T272:S2:VM-MEDIA-PROVIDER:OK\n");
    return 0;
}
