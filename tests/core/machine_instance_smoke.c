#include "type.h"




#include "core/machine/machine_interface.h"
#include "../support/core_machine_executor_fixture.h"

typedef struct port_fixture {
    type_unsigned_16 last_port;
    type_unsigned_32 value;
} port_fixture;

static type_status port_read(
    C_VOID *owner,
    type_unsigned_16 port,
    type_unsigned_32 *out_value)
{
    port_fixture *fixture = (port_fixture *)owner;

    fixture->last_port = port;
    *out_value = fixture->value;
    return TYPE_STATUS_OK;
}

static type_status port_write(
    C_VOID *owner,
    type_unsigned_16 port,
    type_unsigned_32 value)
{
    port_fixture *fixture = (port_fixture *)owner;

    fixture->last_port = port;
    fixture->value = value;
    return TYPE_STATUS_OK;
}

static C_INT expect_status(type_status actual, type_status expected)
{
    return actual == expected ? 0 : 1;
}

C_INT main(C_VOID)
{
    core_machine *first = STD_NULL;
    core_machine *second = STD_NULL;
    core_machine_cpu_state cpu;
    core_machine_port_provider ops = { port_read, port_write };
    port_fixture first_port = { 0u, 0u };
    port_fixture second_port = { 0u, 0u };
    type_unsigned_8 value;
    type_unsigned_32 port_value;
    C_INT result = 0;

    result |= expect_status(test_core_machine_create_executor(
                                CORE_MACHINE_MINIMUM_MEMORY_BYTES, &first),
                            TYPE_STATUS_OK);
    result |= expect_status(test_core_machine_create_executor(
                                CORE_MACHINE_MINIMUM_MEMORY_BYTES, &second),
                            TYPE_STATUS_OK);
    result |= expect_status(core_machine_install_port_provider(
                                first, 0x1234u, 0x1234u, &ops, &first_port),
                            TYPE_STATUS_OK);
    result |= expect_status(core_machine_install_port_provider(
                                second, 0x1234u, 0x1234u, &ops, &second_port),
                            TYPE_STATUS_OK);
    result |= expect_status(core_machine_freeze_execution_providers(first),
                            TYPE_STATUS_OK);
    result |= expect_status(core_machine_freeze_execution_providers(second),
                            TYPE_STATUS_OK);
    result |= expect_status(core_machine_reset(first), TYPE_STATUS_OK);
    result |= expect_status(core_machine_reset(second), TYPE_STATUS_OK);
    result |= expect_status(core_machine_get_cpu_state(first, &cpu),
                            TYPE_STATUS_OK);
    result |= cpu.cs != 0xf000u || cpu.cs_base != 0xffff0000u ||
              cpu.eip != 0x0000fff0u || cpu.eflags != 0x00000002u;

    value = 0x11u;
    result |= expect_status(core_machine_memory_write(first, 0u, &value, 1u),
                            TYPE_STATUS_OK);
    result |= expect_status(core_machine_set_a20(first, 1), TYPE_STATUS_OK);
    value = 0x22u;
    result |= expect_status(core_machine_memory_write(first, 0x100000u, &value, 1u),
                            TYPE_STATUS_OK);
    value = 0u;
    result |= expect_status(core_machine_memory_read(first, 0x100000u, &value, 1u),
                            TYPE_STATUS_OK);
    result |= value != 0x22u;
    result |= expect_status(core_machine_set_a20(first, 0), TYPE_STATUS_OK);
    value = 0u;
    result |= expect_status(core_machine_memory_read(first, 0x100000u, &value, 1u),
                            TYPE_STATUS_OK);
    result |= value != 0x11u;
    value = 0xffu;
    result |= expect_status(core_machine_memory_read(second, 0u, &value, 1u),
                            TYPE_STATUS_OK);
    result |= value != 0u;

    result |= expect_status(core_machine_bus_write(first, 0x1234u, 0xa5a5u),
                            TYPE_STATUS_OK);
    result |= expect_status(core_machine_bus_read(first, 0x1234u, &port_value),
                            TYPE_STATUS_OK);
    result |= port_value != 0xa5a5u || first_port.last_port != 0x1234u ||
              second_port.value != 0u;

    result |= expect_status(core_machine_reset(first), TYPE_STATUS_OK);
    value = 0xffu;
    result |= expect_status(core_machine_memory_read(first, 0u, &value, 1u),
                            TYPE_STATUS_OK);
    result |= value != 0u;

    core_machine_destroy(second);
    core_machine_destroy(first);
    if (result != 0) {
        return 1;
    }

    puts("M3:T1:S2:MACHINE-INSTANCE:OK");
    return 0;
}
