#include "type.h"

#include "core/machine/machine_interface.h"
#include "vm/composition/session/session_private.h"
#include "../../../support/rom/model40_session_assets.h"
#include "vm/composition/session/session_interface.h"

static C_INT vm_model40_rom_read(core_machine *machine,
    type_unsigned_32 physical, type_unsigned_8 expected)
{
    type_unsigned_8 observed = 0u;

    return core_machine_memory_read(machine, physical, &observed,
        sizeof(observed)) == TYPE_STATUS_OK && observed == expected;
}

C_INT main(C_VOID)
{
    static type_unsigned_8 even[VM_PROFILE_MODEL40_ROM_CHIP_BYTES];
    static type_unsigned_8 odd[VM_PROFILE_MODEL40_ROM_CHIP_BYTES];
    vm_session_config invalid_config = {
        .profile_kind = VM_SESSION_PROFILE_COMPAQ_DESKPRO_386_MODEL_40
    };
    vm_session_assets missing_assets = {0};
    vm_session *session = STD_NULL;
    core_machine_run_result result;
    type_unsigned_8 write = 0u;
    C_INT failed = 0;

    even[0u] = 0x11u;
    odd[0u] = 0x22u;
    even[0x3ff8u] = 0xf4u;

    failed |= vm_session_create_from_assets(&invalid_config, &missing_assets, &session) !=
        TYPE_STATUS_INVALID_ARGUMENT ||
        session != STD_NULL;
    if (!failed) failed |= vm_model40_fixture_create_bytes(even, odd, &session) !=
        TYPE_STATUS_OK || session == STD_NULL ||
        !vm_model40_rom_read(session->core_machine, 0x000f0000u, 0x11u) ||
        !vm_model40_rom_read(session->core_machine, 0x000f0001u, 0x22u) ||
        !vm_model40_rom_read(session->core_machine, 0x000f8000u, 0x11u) ||
        !vm_model40_rom_read(session->core_machine, 0x000ffff0u, 0xf4u) ||
        !vm_model40_rom_read(session->core_machine, 0xffff0000u, 0x11u) ||
        !vm_model40_rom_read(session->core_machine, 0xffff8000u, 0x11u) ||
        !vm_model40_rom_read(session->core_machine, 0xfffffff0u, 0xf4u) ||
        core_machine_memory_write(session->core_machine, 0x000f0000u, &write,
            sizeof(write)) != TYPE_STATUS_OK ||
        core_machine_memory_write(session->core_machine, 0xffff0000u, &write,
            sizeof(write)) != TYPE_STATUS_OK ||
        !vm_model40_rom_read(session->core_machine, 0x000f0000u, 0x11u) ||
        !vm_model40_rom_read(session->core_machine, 0xffff0000u, 0x11u);
    if (!failed) failed |= core_machine_run(session->core_machine,
        (core_machine_run_budget) {1u, 0u}, &result) != TYPE_STATUS_OK ||
        result.executed != 1u ||
        result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT;
    if (!failed) STD_PRINTF("M5:T386:S14:MODEL40-ROM-LAYOUT:OK\n");
    if (!failed) STD_PRINTF("M5:T386:S14:MODEL40-ROM-RESET:OK\n");
    vm_session_destroy(session);
    return failed ? 1 : 0;
}
