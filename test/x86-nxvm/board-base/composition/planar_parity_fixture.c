#include "planar_parity_fixture.h"

lib_i32 test_planar_parity_memory_fault(x86_core_machine *machine,
    x86_nxvm_board_state *board)
{
    lib_u8 written = 0x5au;
    lib_u8 read = 0u;
    lib_i32 failed = 0;

    failed |= x86_core_machine_memory_write(machine, 0x1234u, &written,
            sizeof(written)) != LIB_STATUS_OK ||
        x86_core_machine_reconfigure_memory(machine, 512u * 1024u) != LIB_STATUS_INVALID_STATE;
    if (!failed) failed |= x86_nxvm_board_report_planar_parity_fault(board) != LIB_STATUS_OK;
    if (!failed) failed |= x86_core_machine_memory_read(machine, 0x1234u, &read,
            sizeof(read)) != LIB_STATUS_OK || read != written;
    return failed;
}

static void ignored_parity_fault(void *owner, lib_u32 physical)
{
    (void)owner;
    (void)physical;
}

lib_i32 test_planar_parity_memory_binding(x86_core_machine *machine,
    const void *expected)
{
    const x86_core_machine_memory_parity_config probe = {1u, ignored_parity_fault};
    lib_u8 owner = 0u;

    if (expected != LIB_NULL &&
        (x86_core_machine_install_memory_device_routes(machine, LIB_NULL, 0u,
            LIB_NULL, &probe, &owner) != LIB_STATUS_INVALID_ARGUMENT ||
         x86_core_machine_remove_memory_device_routes(machine, expected) != LIB_STATUS_OK))
        return 1;
    /* A fresh lease proves absence, or proves the exact prior owner revoked
     * its exclusive storage. No private allocator identity is borrowed. */
    return x86_core_machine_install_memory_device_routes(machine, LIB_NULL, 0u,
            LIB_NULL, &probe, &owner) != LIB_STATUS_OK ||
        x86_core_machine_remove_memory_device_routes(machine, &owner) != LIB_STATUS_OK;
}

lib_i32 test_planar_parity_unbound_reconfigure(void)
{
    x86_nxvm_board_config config = {0};
    x86_core_machine *machine = LIB_NULL;
    lib_i32 failed = 0;

    config.memory_bytes = 2u * 1024u * 1024u;
    if (x86_nxvm_board_create(&config, &machine, LIB_NULL) != LIB_STATUS_OK) failed = 1;
    else if (x86_core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK) failed = 2;
    else if (x86_core_machine_reset(machine) != LIB_STATUS_OK) failed = 3;
    else if (x86_core_machine_reconfigure_memory(machine, 512u * 1024u) != LIB_STATUS_INVALID_STATE) failed = 4;
    x86_core_machine_destroy(machine);
    return failed;
}
