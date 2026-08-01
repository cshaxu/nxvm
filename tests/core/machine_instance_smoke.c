#include <stdint.h>
#include <stdio.h>

#include "machine/core/machine.h"

typedef struct port_fixture {
    uint16_t last_port;
    uint32_t value;
} port_fixture;

static nxvm_core_status port_read(
    void *owner,
    uint16_t port,
    uint32_t *out_value)
{
    port_fixture *fixture = (port_fixture *)owner;

    fixture->last_port = port;
    *out_value = fixture->value;
    return NXVM_CORE_STATUS_OK;
}

static nxvm_core_status port_write(
    void *owner,
    uint16_t port,
    uint32_t value)
{
    port_fixture *fixture = (port_fixture *)owner;

    fixture->last_port = port;
    fixture->value = value;
    return NXVM_CORE_STATUS_OK;
}

static int expect_status(nxvm_core_status actual, nxvm_core_status expected)
{
    return actual == expected ? 0 : 1;
}

int main(void)
{
    nxvm_core_machine *first = NULL;
    nxvm_core_machine *second = NULL;
    nxvm_core_machine_config config = {
        NXVM_CORE_ABI_VERSION,
        NXVM_CORE_PROFILE_TEST_MINIMAL,
        NXVM_CORE_MINIMUM_MEMORY_BYTES
    };
    nxvm_core_cpu_state cpu;
    nxvm_core_port_ops ops = { port_read, port_write };
    port_fixture first_port = { 0u, 0u };
    port_fixture second_port = { 0u, 0u };
    uint8_t value;
    uint32_t port_value;
    int result = 0;

    result |= expect_status(nxvm_core_machine_create(&config, &first),
                            NXVM_CORE_STATUS_OK);
    result |= expect_status(nxvm_core_machine_create(&config, &second),
                            NXVM_CORE_STATUS_OK);
    result |= expect_status(nxvm_core_machine_reset(first), NXVM_CORE_STATUS_OK);
    result |= expect_status(nxvm_core_machine_reset(second), NXVM_CORE_STATUS_OK);
    result |= expect_status(nxvm_core_machine_get_cpu_state(first, &cpu),
                            NXVM_CORE_STATUS_OK);
    result |= cpu.cs != 0xf000u || cpu.cs_base != 0xffff0000u ||
              cpu.eip != 0x0000fff0u || cpu.eflags != 0x00000002u;

    value = 0x11u;
    result |= expect_status(nxvm_core_machine_memory_write(first, 0u, &value, 1u),
                            NXVM_CORE_STATUS_OK);
    result |= expect_status(nxvm_core_machine_set_a20(first, 1), NXVM_CORE_STATUS_OK);
    value = 0x22u;
    result |= expect_status(nxvm_core_machine_memory_write(first, 0x100000u, &value, 1u),
                            NXVM_CORE_STATUS_OK);
    value = 0u;
    result |= expect_status(nxvm_core_machine_memory_read(first, 0x100000u, &value, 1u),
                            NXVM_CORE_STATUS_OK);
    result |= value != 0x22u;
    result |= expect_status(nxvm_core_machine_set_a20(first, 0), NXVM_CORE_STATUS_OK);
    value = 0u;
    result |= expect_status(nxvm_core_machine_memory_read(first, 0x100000u, &value, 1u),
                            NXVM_CORE_STATUS_OK);
    result |= value != 0x11u;
    value = 0xffu;
    result |= expect_status(nxvm_core_machine_memory_read(second, 0u, &value, 1u),
                            NXVM_CORE_STATUS_OK);
    result |= value != 0u;

    result |= expect_status(nxvm_core_machine_install_port(
                                first, 0x1234u, 0x1234u, &ops, &first_port),
                            NXVM_CORE_STATUS_OK);
    result |= expect_status(nxvm_core_machine_install_port(
                                second, 0x1234u, 0x1234u, &ops, &second_port),
                            NXVM_CORE_STATUS_OK);
    result |= expect_status(nxvm_core_machine_port_write(first, 0x1234u, 0xa5a5u),
                            NXVM_CORE_STATUS_OK);
    result |= expect_status(nxvm_core_machine_port_read(first, 0x1234u, &port_value),
                            NXVM_CORE_STATUS_OK);
    result |= port_value != 0xa5a5u || first_port.last_port != 0x1234u ||
              second_port.value != 0u;

    result |= expect_status(nxvm_core_machine_reset(first), NXVM_CORE_STATUS_OK);
    value = 0xffu;
    result |= expect_status(nxvm_core_machine_memory_read(first, 0u, &value, 1u),
                            NXVM_CORE_STATUS_OK);
    result |= value != 0u;

    nxvm_core_machine_destroy(second);
    nxvm_core_machine_destroy(first);
    if (result != 0) {
        return 1;
    }

    puts("M3:T1:S2:MACHINE-INSTANCE:OK");
    return 0;
}
