#include "lib/types/types_interface.h"
#include <stdio.h>




#include "app-nxvm/devices/machine_interface.h"
#include "support/core_machine_executor_fixture.h"

typedef struct port_fixture {
    lib_u16 last_port;
    lib_u32 value;
} port_fixture;

static lib_status port_read(
    void *owner,
    lib_u16 port,
    lib_u32 *out_value)
{
    port_fixture *fixture = (port_fixture *)owner;

    fixture->last_port = port;
    *out_value = fixture->value;
    return LIB_STATUS_OK;
}

static lib_status port_write(
    void *owner,
    lib_u16 port,
    lib_u32 value)
{
    port_fixture *fixture = (port_fixture *)owner;

    fixture->last_port = port;
    fixture->value = value;
    return LIB_STATUS_OK;
}

static lib_i32 expect_status(lib_status actual, lib_status expected)
{
    return actual == expected ? 0 : 1;
}

lib_i32 main(void)
{
    core_machine *first = LIB_NULL;
    core_machine *second = LIB_NULL;
    core_machine_cpu_state cpu;
    core_machine_port_provider ops = { port_read, port_write };
    port_fixture first_port = { 0u, 0u };
    port_fixture second_port = { 0u, 0u };
    lib_u8 value;
    lib_u32 port_value;
    lib_i32 result = 0;

    result |= expect_status(test_core_machine_create_executor(
                                CORE_MACHINE_MINIMUM_MEMORY_BYTES, &first),
                            LIB_STATUS_OK);
    result |= expect_status(test_core_machine_create_executor(
                                CORE_MACHINE_MINIMUM_MEMORY_BYTES, &second),
                            LIB_STATUS_OK);
    result |= expect_status(core_machine_install_port_provider(
                                first, 0x1234u, 0x1234u, &ops, &first_port),
                            LIB_STATUS_OK);
    result |= expect_status(core_machine_install_port_provider(
                                second, 0x1234u, 0x1234u, &ops, &second_port),
                            LIB_STATUS_OK);
    result |= expect_status(core_machine_freeze_execution_providers(first),
                            LIB_STATUS_OK);
    result |= expect_status(core_machine_freeze_execution_providers(second),
                            LIB_STATUS_OK);
    result |= expect_status(core_machine_reset(first), LIB_STATUS_OK);
    result |= expect_status(core_machine_reset(second), LIB_STATUS_OK);
    result |= expect_status(core_machine_get_cpu_state(first, &cpu),
                            LIB_STATUS_OK);
    result |= cpu.cs != 0xf000u || cpu.cs_base != 0xffff0000u ||
              cpu.eip != 0x0000fff0u || cpu.eflags != 0x00000002u;

    value = 0x11u;
    result |= expect_status(core_machine_memory_write(first, 0u, &value, 1u),
                            LIB_STATUS_OK);
    result |= expect_status(core_machine_set_a20(first, 1), LIB_STATUS_OK);
    value = 0x22u;
    result |= expect_status(core_machine_memory_write(first, 0x100000u, &value, 1u),
                            LIB_STATUS_OK);
    value = 0u;
    result |= expect_status(core_machine_memory_read(first, 0x100000u, &value, 1u),
                            LIB_STATUS_OK);
    result |= value != 0x22u;
    result |= expect_status(core_machine_set_a20(first, 0), LIB_STATUS_OK);
    value = 0u;
    result |= expect_status(core_machine_memory_read(first, 0x100000u, &value, 1u),
                            LIB_STATUS_OK);
    result |= value != 0x11u;
    value = 0xffu;
    result |= expect_status(core_machine_memory_read(second, 0u, &value, 1u),
                            LIB_STATUS_OK);
    result |= value != 0u;

    result |= expect_status(core_machine_bus_write(first, 0x1234u, 0xa5a5u),
                            LIB_STATUS_OK);
    result |= expect_status(core_machine_bus_read(first, 0x1234u, &port_value),
                            LIB_STATUS_OK);
    result |= port_value != 0xa5a5u || first_port.last_port != 0x1234u ||
              second_port.value != 0u;

    result |= expect_status(core_machine_reset(first), LIB_STATUS_OK);
    value = 0xffu;
    result |= expect_status(core_machine_memory_read(first, 0u, &value, 1u),
                            LIB_STATUS_OK);
    result |= value != 0u;

    core_machine_destroy(second);
    core_machine_destroy(first);
    if (result != 0) {
        return 1;
    }

    puts("M3:T1:S2:MACHINE-INSTANCE:OK");
    return 0;
}
