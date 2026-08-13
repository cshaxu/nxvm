#include "type.h"

#include "core/machine/machine_interface.h"

static type_status machine_configuration_port_read(C_VOID *owner,
    type_unsigned_16 port, type_unsigned_32 *out_value)
{
    (C_VOID)owner;
    (C_VOID)port;
    *out_value = 0x5au;
    return TYPE_STATUS_OK;
}

static C_INT machine_configuration_expect(type_status actual,
    type_status expected)
{
    return actual == expected ? 0 : 1;
}

C_INT main(C_VOID)
{
    core_machine *machine = STD_NULL;
    core_machine_config config = { 0 };
    core_machine_port_provider port_provider = {
        machine_configuration_port_read, STD_NULL
    };
    core_machine_run_budget budget = { 1u, 0u };
    core_machine_run_result run_result;
    C_UCHAR halt = 0xf4u;
    C_INT failed = 0;

    failed |= machine_configuration_expect(core_machine_create(&config, &machine),
        TYPE_STATUS_OK);
    failed |= machine_configuration_expect(core_machine_reset(machine),
        TYPE_STATUS_INVALID_STATE);
    failed |= machine_configuration_expect(core_machine_install_port_provider(
        machine, 0x300u, 0x300u, &port_provider, STD_NULL), TYPE_STATUS_OK);
    failed |= machine_configuration_expect(core_machine_freeze_execution_providers(
        machine), TYPE_STATUS_OK);
    failed |= machine_configuration_expect(core_machine_freeze_execution_providers(
        machine), TYPE_STATUS_INVALID_STATE);
    failed |= machine_configuration_expect(core_machine_install_port_provider(
        machine, 0x301u, 0x301u, &port_provider, STD_NULL),
        TYPE_STATUS_INVALID_STATE);
    failed |= machine_configuration_expect(core_machine_bind_execution_provider(
        machine, STD_NULL, STD_NULL), TYPE_STATUS_INVALID_STATE);
    failed |= machine_configuration_expect(core_machine_reset(machine),
        TYPE_STATUS_OK);
    failed |= machine_configuration_expect(core_machine_memory_write(machine,
        0xffff0u, &halt, 1u), TYPE_STATUS_OK);
    failed |= machine_configuration_expect(core_machine_run(machine, budget,
        &run_result), TYPE_STATUS_OK);
    failed |= machine_configuration_expect(core_machine_memory_write(machine,
        0xffff0u, &halt, 1u), TYPE_STATUS_OK);

    core_machine_destroy(machine);
    if (failed != 0) return 1;
    STD_PRINTF("M5:T160:S1:LIFECYCLE-CONTRACT:OK\n");
    return 0;
}
