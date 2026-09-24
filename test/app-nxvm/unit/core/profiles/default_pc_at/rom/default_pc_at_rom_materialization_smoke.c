#include "lib/types/types_interface.h"
#include <stdio.h>

#include "app-nxvm/devices/machine_interface.h"
#include "app-nxvm/machine/machine_interface.h"
#include "app-nxvm/machine/machine_private.h"

int main(void)
{
    lib_u8 image[VM_PROFILE_EXTERNAL_PC_AT_ROM_BYTES] = {0};
    const vm_machine_config config = {
        .profile_kind = VM_MACHINE_PROFILE_DEFAULT_PC_AT,
        .bios_count = 1u
    };
    vm_machine_assets assets = {0};
    vm_machine *session = LIB_NULL;
    core_machine_memory_route route;
    lib_u8 observed[2] = {0};
    lib_u8 overwrite = 0u;
    lib_i32 failed;

    image[0u] = 0x56u;
    image[1u] = 0x78u;
    image[VM_PROFILE_EXTERNAL_PC_AT_ROM_BYTES - 16u] = 0xf4u;
    assets.bios[0u] = (vm_machine_asset_bytes) { image, sizeof(image) };
    failed = vm_machine_create_from_assets(&config, &assets, &session) !=
            LIB_STATUS_OK || session == LIB_NULL || !vm_profile_machine_plan_external_firmware(
                session->profile_plan);
    if (!failed) {
        failed |= core_machine_memory_query(session->core_machine, 0x000f0000u,
            1u, CORE_MACHINE_MEMORY_ACCESS_READ, &route) != LIB_STATUS_OK ||
            route != CORE_MACHINE_MEMORY_ROUTE_PROVIDER;
        failed |= core_machine_memory_read(session->core_machine, 0x000f0000u,
            observed, sizeof(observed)) != LIB_STATUS_OK || observed[0u] != 0x56u ||
            observed[1u] != 0x78u;
        failed |= core_machine_memory_write(session->core_machine, 0x000f0000u,
            &overwrite, sizeof(overwrite)) != LIB_STATUS_OK;
        failed |= core_machine_memory_read(session->core_machine, 0x000f0000u,
            observed, sizeof(observed)) != LIB_STATUS_OK || observed[0u] != 0x56u ||
            observed[1u] != 0x78u;
    }
    vm_machine_destroy(session);
    if (failed) return 1;
    printf("M5:T515:UNIT:ROM-ASSET-MAPPING:OK\n");
    return 0;
}
