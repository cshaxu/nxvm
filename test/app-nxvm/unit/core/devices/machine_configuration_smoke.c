#include "lib/types/types_interface.h"
#include <stdio.h>

#include "app-nxvm/devices/machine_interface.h"

static lib_status machine_configuration_port_read(void *owner,
    lib_u16 port, lib_u32 *out_value)
{
    (void)owner;
    (void)port;
    *out_value = 0x5au;
    return LIB_STATUS_OK;
}

static lib_i32 machine_configuration_expect(lib_status actual,
    lib_status expected)
{
    return actual == expected ? 0 : 1;
}

lib_i32 main(void)
{
    core_machine *machine = LIB_NULL;
    core_machine_config config = { 0 };
    core_machine_port_provider port_provider = {
        machine_configuration_port_read, LIB_NULL
    };
    core_machine_run_budget budget = { 1u, 0u };
    core_machine_run_result run_result;
    lib_u8 halt = 0xf4u;
    lib_i32 failed = 0;

    failed |= machine_configuration_expect(core_machine_create(&config, &machine),
        LIB_STATUS_OK);
    failed |= machine_configuration_expect(core_machine_reset(machine),
        LIB_STATUS_INVALID_STATE);
    failed |= machine_configuration_expect(core_machine_install_port_provider(
        machine, 0x300u, 0x300u, &port_provider, LIB_NULL), LIB_STATUS_OK);
    failed |= machine_configuration_expect(core_machine_freeze_execution_providers(
        machine), LIB_STATUS_OK);
    failed |= machine_configuration_expect(core_machine_freeze_execution_providers(
        machine), LIB_STATUS_INVALID_STATE);
    failed |= machine_configuration_expect(core_machine_install_port_provider(
        machine, 0x301u, 0x301u, &port_provider, LIB_NULL),
        LIB_STATUS_INVALID_STATE);
    failed |= machine_configuration_expect(core_machine_bind_execution_provider(
        machine, LIB_NULL, LIB_NULL), LIB_STATUS_INVALID_STATE);
    failed |= machine_configuration_expect(core_machine_reset(machine),
        LIB_STATUS_OK);
    failed |= machine_configuration_expect(core_machine_memory_write(machine,
        0xffff0u, &halt, 1u), LIB_STATUS_OK);
    failed |= machine_configuration_expect(core_machine_run(machine, budget,
        &run_result), LIB_STATUS_OK);
    failed |= machine_configuration_expect(core_machine_memory_write(machine,
        0xffff0u, &halt, 1u), LIB_STATUS_OK);

    core_machine_destroy(machine);
    if (failed != 0) return 1;
    printf("M5:T160:S1:LIFECYCLE-CONTRACT:OK\n");
    return 0;
}
