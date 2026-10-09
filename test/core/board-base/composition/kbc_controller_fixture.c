#include "../../board-at/controller_fixture.h"
#include "core/x86/machine.h"

static void kbc_existing_port(t_port *port, lib_u16 address, void *owner)
{
    (void)address;
    (void)owner;
    port->data.ioByte = 0x5au;
}

lib_bool test_kbc_core_construction(t_kbc *kbc)
{
    lib_size fail_at;
    lib_bool failed = LIB_FALSE;
    for (fail_at = 1u; fail_at <= 4u; ++fail_at) {
        core_machine_port_test_allocation allocation = { fail_at, 0u };
        core_machine machine = {.lifecycle = CORE_MACHINE_INITIALIZED};
        t_port *port = &machine.executor_port;
        core_machine_port_initialize(port);
        failed |= core_machine_port_add_read(port, 0x80u, kbc_existing_port,
            port) != LIB_STATUS_OK;
        core_machine_port_set_test_allocation(port, &allocation);
        failed |= test_kbc_initialize(kbc, &machine) != LIB_STATUS_NO_MEMORY;
        failed |= !test_kbc_unpublished(kbc);
        failed |= core_machine_port_has_read(port, 0x60u) ||
            core_machine_port_has_read(port, 0x64u) ||
            core_machine_port_has_write(port, 0x60u) ||
            core_machine_port_has_write(port, 0x64u) ||
            core_machine_port_read(port, 0x80u) != 0x5au;
        core_machine_port_set_test_allocation(port, LIB_NULL);
        failed |= test_kbc_initialize(kbc, &machine) != LIB_STATUS_OK;
        core_machine_port_write(port, 0x64u, 0xaau);
        failed |= core_machine_port_read(port, 0x60u) != 0x55u;
        test_kbc_finalize(kbc);
        test_kbc_finalize(kbc);
        core_machine_port_finalize(port);
    }
    {
        core_machine machine = {.lifecycle = CORE_MACHINE_INITIALIZED};
        t_port *port = &machine.executor_port;
        core_machine_port_initialize(port);
        failed |= core_machine_port_add_write(port, 0x64u, kbc_existing_port,
            port) != LIB_STATUS_OK;
        failed |= test_kbc_initialize(kbc, &machine) != LIB_STATUS_INVALID_STATE;
        failed |= !test_kbc_unpublished(kbc) ||
            core_machine_port_has_read(port, 0x60u) ||
            core_machine_port_has_read(port, 0x64u) ||
            core_machine_port_has_write(port, 0x60u);
        core_machine_port_write(port, 0x64u, 0u);
        failed |= port->data.ioByte != 0x5au;
        failed |= core_machine_port_add_write(port, 0x64u, kbc_existing_port,
            kbc) != LIB_STATUS_INVALID_STATE;
        failed |= test_kbc_initialize(kbc, &machine) != LIB_STATUS_INVALID_STATE;
        failed |= core_machine_port_registration_status(port) != LIB_STATUS_INVALID_STATE;
        test_kbc_finalize(kbc);
        core_machine_port_finalize(port);
    }
    return failed;
}


lib_status test_kbc_core_reset_entry(core_machine *machine,
    const core_machine_debug_register_patch *entry)
{
    return core_machine_cpu_debug_patch_registers(machine->executor_cpu_execution,
        entry);
}
