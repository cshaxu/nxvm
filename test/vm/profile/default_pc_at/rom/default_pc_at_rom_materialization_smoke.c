#include "type.h"

#include "core/machine/machine_interface.h"
#include "vm/composition/session/session_interface.h"
#include "vm/composition/session/session_private.h"

int main(C_VOID)
{
    type_unsigned_8 image[VM_SESSION_PC_AT_ROM_BYTES] = {0};
    const vm_session_config config = {
        .profile_kind = VM_SESSION_PROFILE_DEFAULT_PC_AT,
        .bios_count = 1u
    };
    vm_session_assets assets = {0};
    vm_session *session = STD_NULL;
    core_machine_memory_route route;
    type_unsigned_8 observed[2] = {0};
    type_unsigned_8 overwrite = 0u;
    C_INT failed;

    image[0u] = 0x56u;
    image[1u] = 0x78u;
    image[VM_SESSION_PC_AT_ROM_BYTES - 16u] = 0xf4u;
    assets.bios[0u] = (vm_session_asset_bytes) { image, sizeof(image) };
    failed = vm_session_create_from_assets(&config, &assets, &session) !=
            TYPE_STATUS_OK || session == STD_NULL || !session->pc_at_rom_external;
    if (!failed) {
        failed |= core_machine_memory_query(session->core_machine, 0x000f0000u,
            1u, CORE_MACHINE_MEMORY_ACCESS_READ, &route) != TYPE_STATUS_OK ||
            route != CORE_MACHINE_MEMORY_ROUTE_PROVIDER;
        failed |= core_machine_memory_read(session->core_machine, 0x000f0000u,
            observed, sizeof(observed)) != TYPE_STATUS_OK || observed[0u] != 0x56u ||
            observed[1u] != 0x78u;
        failed |= core_machine_memory_write(session->core_machine, 0x000f0000u,
            &overwrite, sizeof(overwrite)) != TYPE_STATUS_OK;
        failed |= core_machine_memory_read(session->core_machine, 0x000f0000u,
            observed, sizeof(observed)) != TYPE_STATUS_OK || observed[0u] != 0x56u ||
            observed[1u] != 0x78u;
    }
    vm_session_destroy(session);
    if (failed) return 1;
    STD_PRINTF("M5:T515:UNIT:ROM-ASSET-MAPPING:OK\n");
    return 0;
}
