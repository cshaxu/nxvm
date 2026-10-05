#include "../../ibmpc/board-xt/controller_fixture.h"
#include "x86/core/machine.h"

static void xt_existing_port(t_port *port, lib_u16 address, void *owner)
{
    (void)address;
    (void)owner;
    port->data.ioByte = 0x5au;
}

lib_bool test_xt_ppi_core_construction(core_machine_xt_ppi_keyboard *keyboard,
    const core_machine_xt_ppi_keyboard_config *config, lib_size fail_at)
{
    lib_bool failed = LIB_FALSE;
    core_machine_port_test_allocation allocation = {fail_at, 0u};
    core_machine machine = {0};
    t_port *port = &machine.executor_port;
    machine.lifecycle = CORE_MACHINE_INITIALIZED;
    core_machine_port_initialize(port);
    failed |= core_machine_port_add_read(port, 0x80u, xt_existing_port, port) != LIB_STATUS_OK;
    core_machine_port_set_test_allocation(port, &allocation);
    failed |= test_xt_ppi_initialize(keyboard, config, &machine) != LIB_STATUS_NO_MEMORY;
    failed |= !test_xt_ppi_unpublished(keyboard);
    for (lib_u16 address = 0x60u; address <= 0x63u; ++address) {
        failed |= core_machine_port_has_read(port, address) || core_machine_port_has_write(port, address);
    }
    failed |= core_machine_port_read(port, 0x80u) != 0x5au;
    core_machine_port_set_test_allocation(port, LIB_NULL);
    failed |= test_xt_ppi_initialize(keyboard, config, &machine) != LIB_STATUS_OK;
    core_machine_port_write(port, 0x63u, 0x80u);
    core_machine_port_write(port, 0x60u, 0xa5u);
    failed |= core_machine_port_read(port, 0x60u) != 0xa5u;
    test_xt_ppi_finalize(keyboard);
    test_xt_ppi_finalize(keyboard);
    core_machine_port_finalize(port);
    return failed;
}
